#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

void db_server_divide(char *path, unsigned int job_id, unsigned int n)
{
   if (n == 0) 
   {
     printf("Number of parts = 0 ");
     return;
   }
   char buf[20];
   char split[100] = "split -dl ";
   unsigned int count = 0;
   FILE *fp = fopen(path, "r+");
   while(fgets(buf, 20, fp))
   {
      count++;  
   }
   printf("Number of Parts : %d\n", n);
   count = count / n;
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
   }
   fclose(fp);
}

int test()
{
   db_server_divide("/home/praveen/syspgm/test_split", 1, 11);
   return 0;
}
