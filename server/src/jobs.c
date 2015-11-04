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
#include "pthread.h"
#include "dynamic_lib_interface.h"

struct reassignQ {
    ClientNode *head;
    ClientNode *tail;
} reassignQ;

void
enqueue_pending_job (JobNode *jn, ClientNode *cn)
{
    if (jn == NULL || cn == NULL)
	return;

    cn->next = reassignQ.head;
    cn->prev = NULL;

    if (reassignQ.head) {
	reassignQ.head->prev = cn;
    } else {
	reassignQ.tail = cn;
    }

    reassignQ.head = cn;
}

ClientNode*
dequeue_pending_job (JobNode *jn)
{
    ClientNode *cur = reassignQ.tail;

    while (cur && (cur->jid != jn->job.id)) {
	cur = cur->prev;
    }

    if (cur) {
	if (cur->prev)
	    cur->prev->next = cur->next;
	else
	    reassignQ.head = cur->next;
	if (cur->next)
	    cur->next->prev = cur->prev;
	else
	    reassignQ.tail = cur->prev;
    }

    return cur;
}

enum boolean
assign_pending_job (JobNode *jn, ClientNode *cn, Client *reassign2client)
{
    
    char buffer [MAX_SSH_CMD_SIZE] = {0};
    const char * jd = cntrl_srv_get_job_directory();
    const char * ip = cntrl_srv_get_central_repo_ip();

    if (jn == NULL || cn == NULL || reassign2client == NULL)
	return FALSE;

    ClientNode *reassign2cn = getClientNode(jn, reassign2client);

    if (reassign2cn == NULL)
	return FALSE;

    int index = reassign2cn->index = cn->index;
    free(cn);

    int cfd = reassign2cn->client->cfd;

    logging_informational("Pending job reassigned to client %d", cfd);

    
    snprintf(buffer, MAX_SSH_CMD_SIZE, "%s:%sjob_%d/prob/input_p%d%d%d", ip, jd, jn->job.id, index/100, index/10, index%10);
    sh_send_encoded_data(cfd, buffer, jn->job.task->taskType);

    return TRUE;
}

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
    logging_errors("Job Id %d could not be added : ", jobNode->numClients);
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
  logging_informational("No. of clients reserved for processing the job: %d", jobNode->numClients);
  if (jobNode->numClients == 0)
  {
    //delete the job here , remove from dll
    printf("\nCannot execute the task now");
    return false;
  }
//  db_server_divide(task->basePath, jobId, jobNode->numClients);
  
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

ClientNode* addClientToJob(JobNode *jobNode, Client *client)
{
   ClientNode *node = (ClientNode *)malloc(sizeof(ClientNode));
   node->client = client;
   node->jid = jobNode->job.id;
   node->next = NULL;
   node->prev = jobNode->job.tail;
   jobNode->job.tail->next = node;
   jobNode->job.tail= node;
   jobNode->numClients++;
   return node;
}

bool freeClient(int jobId, Client *client)
{
  if (jobId < 0)
  {
    printf("Job Id = %d \n", jobId);
   return false;
  } 

  JobNode *jobNode = getJobNode(jobId);
  
  if (jobNode == NULL)
  {
    printf("JobNode is null \n");
    return false;
  }

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
    current->index = i;
   //add the index based on spliting here onto basePath
   snprintf(buffer, MAX_SSH_CMD_SIZE, "%s:%sjob_%d/prob/input_p%d%d%d", ip, jd, jobNode->job.id, i/100, i/10, i%10);
   // make this debug: printf("assignJob: %s\n", buffer);
   sh_send_encoded_data(current->client->cfd, buffer, task->taskType);
   current = current->next;
   i++;
  }
  logging_notifications("Sent to all clients");
  return true;
}  

bool reassign_job(int cfd)
{
  logging_notifications("Reassigning Job");
  client_info_head *cih = server_get_client_info_head(cfd);
  Client  *client = server_search_client_fd(cih, cfd);
  char buffer [MAX_SSH_CMD_SIZE] = {0};
  const char * jd = cntrl_srv_get_job_directory();
  const char * ip = cntrl_srv_get_central_repo_ip();
  JobNode *jobNode = client->jn;
  int groupId = client->cg->gid;
  int index = 0;
  ClientNode *oldcn = getClientNode(jobNode, client);
  ClientNode *cn = NULL; 

 
  client_group_head *cgh = server_get_client_gid_head(groupId);
  client_group *cg = cgh->h;
  while(cg)
  {
	if (cg->ci->busy == FALSE) {
	    server_update_job_node(cg->ci, jobNode);
 	    cn = addClientToJob(jobNode, cg->ci);
	    cg->ci->busy = TRUE;
            break;
	}
	    cg = cg->n;
  }

  Client *newClient = NULL;
  if (cg == NULL)
  {
   logging_informational("No free clients to reassign the job, adding to pending job queue");  
   // Reassigning to the same client
   oldcn->next->prev = oldcn->prev;
   oldcn->prev->next = oldcn->next;
   oldcn->prev = oldcn->next = NULL;
   oldcn->client = NULL;
   enqueue_pending_job(jobNode, oldcn);
   return true;
  }  
  else
  {
    newClient = cg->ci;
    cn->index = oldcn->index;
    index = oldcn->index;
    //newClient = client->index;
    logging_informational("Reassigning new job");
  }
   snprintf(buffer, MAX_SSH_CMD_SIZE, "%s:%sjob_%d/prob/input_p%d%d%d", ip, jd, jobNode->job.id, index/100, index/10, index%10);
   sh_send_encoded_data(newClient->cfd, buffer, jobNode->job.task->taskType);
  // Freeing the old client now
  freeClient(jobNode->job.id, client);
  return true;  
}

ClientNode* getClientNode(JobNode *jobNode, Client *client)
{
  
  ClientNode *current = jobNode->job.head->next;
  while ((current)&&(current->client != client))
    current = current->next;

  if (current == NULL)
   printf("\nClient is not in the list");

  return current;
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
  pthread_mutex_init(&pHead->lock, NULL);
  pTail = pHead;
  return true;
}

JobNode* addJob(int jobId, Task *task)
{
  logging_informational("%s : Job Id = %d", __func__, jobId);
  pthread_mutex_lock(&pTail->lock);

  JobNode *new = (JobNode *)malloc(sizeof(JobNode));
  new->next = NULL;
  new->prev = pTail;
  new->job.id = jobId;
  pthread_mutex_init(&new->lock, NULL);
  new->job.head = (ClientNode *)malloc(sizeof(ClientNode));
  new->job.head->prev = NULL;
  new->job.head->client = (Client *)0xdeadbeef;
  new->job.head->next = new->job.tail;
  new->job.tail = new->job.head;
  new->job.task = task;
  new->numClients = 0;
  pTail->next = new;
  pTail = new; 
  pthread_mutex_unlock(&pTail->lock);
  return new; 
}
// below function is not called anywhere, it is not thread safe
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
/*
JobNode *getJobNode(int jobId)
{
  JobNode *jobNode = NULL;
  JobNode *tempJobNode = NULL;
  printf("%s\n", __func__);
  if (pthread_mutex_lock(&pHead->lock)== 0)
  {  
   jobNode = pHead->next;
     printf("%s 0\n", __func__);
   while(jobNode)
   {  
     printf("%s 1\n", __func__);
     if (pthread_mutex_lock(&jobNode->lock) == 0) {
     printf("%s 2\n", __func__);
        if (jobNode->job.id != jobId) {
     printf("%s 3\n", __func__);
    tempJobNode = jobNode->next;
     printf("%s 4\n", __func__);
    jobNode = tempJobNode;
        }
    pthread_mutex_unlock(&jobNode->lock);
     printf("%s 5\n", __func__);
    }
   }
   pthread_mutex_unlock(&pHead->lock);
  }
  else
    printf("I can't do it \n");

  return jobNode;
}
*/

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
    pthread_mutex_lock(&jn->lock);
	if (jn->job.result < result) {
	    jn->job.result =  result;
	}
    pthread_mutex_unlock(&jn->lock);
    } else {
	return FALSE;
    }

    logging_notifications("Job Id: %d. Result received from client [%d] "			\
			  "for problem [MAX]: %lld",
			  client->jn->job.id, client->cfd, result);
    ClientNode *cn = dequeue_pending_job(jn);
    if (cn) {
	assign_pending_job(jn, cn, client);
	return TRUE;
    }
    printf("Freeing Clients\n");
    if (freeClient(jn->job.id, client) == FALSE) {
	return FALSE;
    }
    printf("Freed Clients: Clients Remaining = %d \n", jn->numClients);

    //pthread_mutex_lock(&jn->lock);
    if (jn->numClients == 0)
	logging_informational("Job Id: %d. The Result for [MAX] = %lld\n",
			      client->jn->job.id, client->jn->job.result);
   // pthread_mutex_unlock(&jn->lock);

    return TRUE;
}
