#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define max_size 20
void main()
{
  FILE *fptr;
  char username[max_size];
  char line[20];
  if((fptr=fopen("/etc/passwd","r"))==NULL)
    {
      printf("cannot open file");
    }
  else
    {
      fptr=fopen("/etc/passwd","r");
      fputs("enter the username",stdout);
      fflush(stdout);
      fgets(username,sizeof username,stdin);
      while((fgets(line,sizeof(line),fptr))!=NULL)
        {
          if(strcmp(line,username) == 0)
            {
              printf("%s valid user",username);
              break;
            }
          else
            {
              printf("%s not valid user",username);
            }
        }
      fclose(fptr);
    }
}
