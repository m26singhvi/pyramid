#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void db_server_divide(char *path, unsigned int job_id, unsigned int n)
{
   char buf[20];
   char split[100] = "split -dl ";
   unsigned int count = 0;
   FILE *fp = fopen(path, "r+");
   while(fgets(buf, 20, fp))
   {
      count++;  
   }
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
   system(split);
   fclose(fp);
}

int test()
{
   db_server_divide("/home/praveen/syspgm/test_split", 1, 11);
   return 0;
}
