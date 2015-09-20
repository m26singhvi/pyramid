#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jobs.h"
#include "groups.h"
#include "common.h"
#include "server_helper.h"
#include "central_server.h"
#include "logging.h"


bool initJob(int groupId, int jobId, int taskType, char *inputFile)
{

   if ((groupId <= 0)|| (jobId < 0))
    return false;
   
  Task *task = (Task *)malloc(sizeof(Task)); //needs to be delted when job is done;
  memset(task->basePath, 0, 256);
  memcpy(task->basePath, inputFile, strlen(inputFile));
  task->taskType = taskType;
 
  JobNode *jobNode = addJob(jobId, task);
  if (jobNode == NULL)
  {
    // make this debug; printf("\nJob Id  %d could not be added", jobId);
    return false;
  }
  client_group_head *cgh = server_get_client_gid_head(groupId);
  client_group *cg = cgh->h;
  while(cg)
  {
	// make this debug: printf("cg=%p, cgh->tc=%d, cg->ci=%p, cg->p=%p, cg->n=%p\n", cg, cgh->tc, cg->ci, cg->p, cg->n);
	if (cg->ci->busy == FALSE) {
 	    addClientToJob(jobNode, cg->ci);
	    server_update_job_node(cg->ci, jobNode);
	    cg->ci->busy = TRUE;
	}
	cg = cg->n;
  }
  logging_informational("No. of clients processing the job: %d", jobNode->numClients);
  if (jobNode->numClients == 0)
  {
    //delete the job here , remove from dll
    printf("\nCannot execute the task now");
    return false;
  }
  db_server_divide(task->basePath, jobId, jobNode->numClients);
  
// call the api to divide the task here
  int file_status =  db_server_divide(task->basePath, jobId, jobNode->numClients);
  if(file_status)
      return false;

  // send tlv to all the clients here 
  bool sent = assignJob(jobNode, task);
  if (sent)
   return true;
  else
   return false;  
}

bool addClientToJob(JobNode *jobNode, Client *client)
{
   ClientNode *node = (ClientNode *)malloc(sizeof(ClientNode));
//   printf("Node = %p\n", node); 
   node->client = client;
   node->next = NULL;
   node->prev = jobNode->job.tail;
   jobNode->job.tail->next = node;
   jobNode->job.tail= node;
   jobNode->numClients++;
   return true;
}

bool freeClient(int jobId, Client *client)
{
  if (jobId < 0)
   return false;

  JobNode *jobNode = getJobNode(jobId);
  
  if (jobNode == NULL)
   return false;

  ClientNode *current = jobNode->job.head->next;
  while ((current)&&(current->client != client))
    current = current->next;

  if (current == NULL)
   printf("\nClient is not in the list");
  else if (current->next == NULL)
   {
    current->prev->next = NULL;
    client->busy = FALSE;  //client is no longer busy
    jobNode->job.tail = current->prev;
    jobNode->numClients--;
    free (current);
   }
  else
   {
    current->next->prev = current->prev;
    client->busy = FALSE;  //client is no longer busy
    jobNode->numClients--;
    free (current);
   }

  return true;
}
bool assignJob(JobNode *jobNode, Task *task)
{
  ClientNode *current = jobNode->job.head->next;
  char buffer [MAX_SSH_CMD_SIZE] = {0};
  const char * jd = cntrl_srv_get_job_directory();
  const char * ip = cntrl_srv_get_central_repo_ip();
  uint i = 0;

  while(current)
  {
 //   printf("\n============================================");  
   //add the index based on spliting here onto basePath
   snprintf(buffer, MAX_SSH_CMD_SIZE, "%s:%sjob_%d/prob/p%d%d", ip, jd, jobNode->job.id, i/10, i%10);
   // make this debug: printf("assignJob: %s\n", buffer);
   sh_send_encoded_data(current->client->cfd, buffer, task->taskType);
   current = current->next;
   i++;
  }
  logging_notifications("Sent to all clients");
  return true;
}  

bool initializeJobDll()
{
  pHead = (JobNode *)malloc(sizeof(JobNode));
  pHead->prev = NULL;
  pHead->next = pTail;
  pHead->job.id = -1;
  pHead->job.head = NULL;
  pHead->job.tail = NULL;
  pHead->numClients = 0;
  pTail = pHead; 
  return true;
}

JobNode* addJob(int jobId, Task *task)
{
  logging_informational("%s : Job Id = %d", __func__, jobId);

  JobNode *new = (JobNode *)malloc(sizeof(JobNode));
  new->next = NULL;
  new->prev = pTail;
  new->job.id = jobId;
  new->job.head = (ClientNode *)malloc(sizeof(ClientNode));
  new->job.head->prev = NULL;
  new->job.head->client = (Client *)0xdeadbeef;
  new->job.head->next = new->job.tail;
  new->job.tail = new->job.head;
  new->job.task = task;
  new->numClients = 0;
  pTail->next = new;
  pTail = new; 
  return new; 
}

bool removeJob( int jobId)
{
  logging_informational("%sRemoving : Job Id = %d",__func__ , jobId);
  JobNode *jobNode = getJobNode(jobId);
  
  if (jobNode == NULL)
  {
   printf("\nNo such job present\n");
   return false;
  }
  else if((jobNode->job.head->next == NULL)&&(jobNode->job.tail->next == NULL))
  {  
    if(jobNode->next == NULL)
    {
     jobNode->prev->next = jobNode->next;
     pTail = jobNode->prev;
    }
    else
    {
     jobNode->prev->next = jobNode->next;
     jobNode->next->prev = jobNode->prev;
    }
     free(jobNode->job.head);
     free(jobNode->job.tail);
     free(jobNode->job.task);
     free(jobNode); 
  }
  else
  {
   printf("\nCan't delete the job,first free clients from job");
   return false;
  }

  return true;
}

JobNode *getJobNode(int jobId)
{
 // printf("Job Id = %d \n", jobId);
  JobNode *jobNode = pHead->next;
  while((jobNode)&&(jobNode->job.id != jobId))
   jobNode = jobNode->next;
  
  return jobNode;
}


enum boolean updateJobResult(int cfd, char *value)
{
    client_info_head *cih = server_get_client_info_head(cfd);
    Client  *client = server_search_client_fd(cih, cfd);
    JobNode *jn = NULL;
    long long result = atoll(value);

    if (client && client->jn) {
	jn = client->jn;
	if (jn->job.result < result) {
	    jn->job.result =  result;
	}
    } else {
	return FALSE;
    }

    logging_notifications("Job Id: %d. Result received from client [%d] "			\
			  "for problem [MAX]: %lld",
			  client->jn->job.id, client->cfd, result);

    if (freeClient(jn->job.id, client) == FALSE) {
	return FALSE;
    }

    if (jn->numClients == 0)
	logging_informational("Job Id: %d. The Result for [MAX] = %lld\n",
			      client->jn->job.id, client->jn->job.result);

    return TRUE;
}
