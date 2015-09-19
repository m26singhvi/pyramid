#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "groups.h"
#include "common.h"
#include "jobs.h"
#include "server_helper.h"


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
    printf("Job Id  %d could not be added", jobId);
    return false;
  }
  unsigned int numClient = 0;
  client_group_head *cgh = server_get_client_gid_head(groupId);
  client_group *cg = cgh->h;
  while(cg)
  {
//	printf("cg=%p, cgh->tc=%d, cg->ci=%p, cg->p=%p, cg->n=%p\n", cg, cgh->tc, cg->ci, cg->p, cg->n);
	if (cg->ci->busy == FALSE) {
 	    addClientToJob(jobNode, cg->ci);
	    cg->ci->busy = TRUE;
	}
	cg = cg->n;
  }
 // printf("Number of Clients : %d\n", numClient);
  if (jobNode->numClients == 0)
  {
    //delete the job here , remove from dll
    printf("Cannot execute the task now");
    return false;
  }
  db_server_divide(task->basePath, jobId, jobNode->numClients);
  
// call the api to divide the task here
  int file_status =  db_server_divide(task->basePath, jobId, numClient);
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

  printf("%s\n",__func__ ); 
  JobNode *jobNode = getJobNode(jobId);
  
  if (jobNode == NULL)
   return false;

  ClientNode *current = jobNode->job.head->next;
  while ((current)&&(current->client != client))
    current = current->next;

  if (current == NULL)
   printf("Client is not in the list");
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
  while(current)
  {
    printf("\n============================================");  
   //add the index based on spliting here onto basePath
   sh_send_encoded_data(current->client->cfd, task->basePath, task->taskType);
   current = current->next;
  }
  printf("\nSent to all clients");
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
  printf(" %s : Job Id = %d\n ", __func__, jobId);

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
  printf("%s  Job Id = %d \n",__func__ , jobId);
  JobNode *jobNode = getJobNode(jobId);
  
  if (jobNode == NULL)
  {
   printf("No such job present\n");
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
   printf("Can't delete the job,first free clients from job");
   return false;
  }

  return true;
}

JobNode *getJobNode(int jobId)
{
  JobNode *jobNode = pHead->next;
  while((jobNode)&&(jobNode->job.id != jobId))
   jobNode = jobNode->next;
  
  return jobNode;
}



