#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define SUCCESS 0
#define FAILURE 1


int db_server_divide(char *path, unsigned int job_id, unsigned int n)
{
   if (n == 0) 
   {
     printf("Number of parts = 0 ");
     return FAILURE;
   }
   char buf[20];
   char split[100] = "split -dl ";
   unsigned int count = 0;
   FILE *fp = fopen(path, "r+");
   if(fp == NULL){
     printf("\nInvalid file path");
     return FAILURE;
   }
   while(fgets(buf, 20, fp))
   {
      count++;  
   }
   printf("Number of Parts : %d\n", n);
   if((count%n) == 0){
       count = count / n;
   } else {
     count = count/n +1;
   }
   printf("%u",count);
   sprintf(buf,"%d", count);
   strcat(split, buf);
   strcat(split, " ");
   strcat(split, path);
   strcat(split, " ");
   sprintf(buf,"%d", job_id);
   strcat(split, buf);
   strcat(split, "_"); 
   printf("FINAL = %s",split);
   if (system(split) == 0)
   {
    printf("Split Successful \n");
   } else{
     return FAILURE;
   }
   fclose(fp);
   return SUCCESS;
}

int test()
{
   db_server_divide("/home/praveen/syspgm/test_split", 1, 11);
   return 0;
}
