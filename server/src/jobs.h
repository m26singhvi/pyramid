#ifndef __JOBS_H__
#define __JOBS_H__

#include <arpa/inet.h>
#include <stdbool.h>
#include "tlv.h"
#include "common.h"
#include "groups.h"

typedef struct jobNode job_node;
typedef struct client_info Client;

typedef struct Task
{
  char basePath[256];
  Attribute taskType; // ALGO_SORT, ALGO_MAX 
}Task;

typedef struct ClientNode
{
  Client *client;
  struct ClientNode *prev;
  struct ClientNode *next;
  int index;
}ClientNode;

typedef struct Job
{
  int id;
  /* need to merge the below 2 results */
  long long result;
  char result_path[256];
  Task *task;
  ClientNode *head;
  ClientNode *tail;
}Job;

typedef struct jobNode
{
  pthread_mutex_t lock;  
  Job job;
  int numClients;
  struct jobNode *prev;
  struct jobNode *next;
}JobNode;

JobNode *pHead;
JobNode *pTail;

bool initializeJobDll();
JobNode* addJob(int jobId, Task *task);
bool removeJob(int jobId);
JobNode* getJobNode(int jobId);

bool initJob(int groupId, int jobId, int taskType, char *inputFile);
bool assignJob(JobNode *jobNode, Task *task);
ClientNode* addClientToJob(JobNode* jobNode, Client *client);
bool freeClient(int jobId, Client *client);
extern enum boolean updateJobResult(int cfd, char *value); 
bool reassign_job(int cfd);
ClientNode* getClientNode(JobNode *jobNode, Client *client);
#endif /* __JOBS_H__ */
