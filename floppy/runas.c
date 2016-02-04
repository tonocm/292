#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>

/* getUsrInfo - gets user information
   parameters:
   char* line: buffered line from reading the file
   long lineSize: size of buffered line (char* line)
   char** usr: pointer to a char* (string) that will store the user found in given line
   long** uid: pointer to the pointer that stores the user id associated with the found user.
   long** gid: pointer to the pointer that stores the group id associated with the found user.

   return:
   none

   indirectly modified variables:
   usr: stores the user found in line
   uid: stores the uid found in line
   gid: stores the gid found in line
*/
void getUsrInfo(char* line, long lineSize, char** usr, long** uid, long** gid){
  int colons = 0;
  int indexPreviousColon = 0;
  char* uidStr = (char*)malloc(lineSize*sizeof(char));
  char* gidStr = (char*)malloc(lineSize*sizeof(char));

  for(int i=0; i < lineSize; i++) {
    if(line[i] == ':') { // delimiter
      colons++;
      if(colons == 1){ // username
        strncpy(*usr, line+indexPreviousColon, i-indexPreviousColon);
      }
      else if(colons == 3) {// user ID
        strncpy(uidStr, line+indexPreviousColon, i-indexPreviousColon);
        **uid = atol(uidStr);
      }
      else if(colons == 4) { // group ID
        strncpy(gidStr, line+indexPreviousColon, i-indexPreviousColon);
        **gid = atol(gidStr);
        return;
      }
      indexPreviousColon = i+1;
    }
  }
  strncpy(*usr, "", 1);
}

/* validateUser - validates user information based on username search
   parameters:
   char* user: char* containing the username we're searching for
   long* uid: pointer to the long integer that stores the user id that we're looking for
   long* gid: pointer to the long integer that stores the group id that we're looking for

   return:
   true (1): user provided exists
   false (0): user provided does not exist

   indirectly modified variables:
   uid: stores found uid
   gid: stores found gid
*/
int validateUser(char* user, long* uid, long* gid){
  FILE* fp = fopen("/etc/passwd", "r");
  char * line = NULL;
  size_t len = 0;
  long lineSize;
  char* usr;

  if (fp == NULL){
    printf("Error: '/etc/passwd' not found\n");
    exit(EXIT_FAILURE);
  }

  while ((lineSize = getline(&line, &len, fp)) != -1) {
    usr = (char*)calloc(lineSize,sizeof(char));
    getUsrInfo(line, lineSize, &usr, &uid, &gid);
    if(strcmp(usr, user) == 0) { // Strings are equal
      fclose(fp);
      return 1;
    }
    free(usr);
  }
  fclose(fp);
  return 0;
}

/* getPwdInfo - gets password and user iformation from /etc/runas
   parameters:
   char* line: buffered line from the file we're reading
   long lineSize: size of the buffered line
   char** u1: pointer to string that contains the user that can run programs as u2
   char** u2: pointer to string that contains the user that u1 can run programs as
   char** pwd: pointer to string that contains the password for u1

   return:
   none

   indirectly modified variables:
   u1: stores found uid
   u2: stores found gid
   pwd: stores found password
*/
void getPwdInfo(char* line, long lineSize, char** u1, char** u2, char** pwd){
  int colons = 0;
  int indexPreviousColon = 0;
  int j;
  for(int i=0; i < lineSize; i++) {
    if(line[i] == ':') { // delimiter
      colons++;
      if(colons == 1){ // u1
        strncpy(*u1, line+indexPreviousColon, i-indexPreviousColon);
      }
      else if(colons == 2) {// u2
        strncpy(*u2, line+indexPreviousColon, i-indexPreviousColon);
        j=i;
        while(1){
          if(line[j] == '\0' || line[j] == '\n' || line[j] == '\r')
            break;
          else
          j++;
        }
        strncpy(*pwd, line+i+1, j-i-1);
        return;
      }
      indexPreviousColon = i+1;
    }
  }
  strncpy(*u1, "", 1);
  strncpy(*u2, "", 1);
  strncpy(*pwd, "", 1);
}

/* validatePassword - compares password extracted from getPwdInfo and user input
   parameters:
   char* user1: string containing original user
   char* user2: string containing runas user

   return:
   true: user input password matches /etc/runas
   false: user input password does not match /etc/runas
*/
int validatePassword(char* user1, char* user2){

  FILE* fp = fopen("/etc/runas", "r");
  char * line = NULL;
  size_t len = 0;
  long lineSize;
  char* u1;
  char* u2;
  char* pwd;

  if (fp == NULL)
    exit(EXIT_FAILURE);

  char* pass = getpass("Password: ");
  while ((lineSize = getline(&line, &len, fp)) != -1) {

    u1 = (char*)calloc(lineSize,sizeof(char));
    u2 = (char*)calloc(lineSize,sizeof(char));
    pwd = (char*)calloc(lineSize,sizeof(char));

    getPwdInfo(line, lineSize, &u1, &u2, &pwd);
    
    if(strcmp(user1, u1) == 0 && strcmp(user2, u2) == 0 && strcmp(pass, pwd) == 0) {  // All strings are equal
      fclose(fp);
      return 1;
    }
    free(u1);
    free(u2);
    free(pwd);
  }
  fclose(fp);
  return 0;
}

/* findUsername - finds username based on real uid
   parameters:
   char* line: buffer from file being read
   long lineSize: size of line
   char** usr: pointer to the string storing the username in /etc/passwd
   long* uid: pointer to the user id found in etc/passwd

   return:
   none

   indirectly modified variables:
   usr: stores username if uid matches uid in /etc/passwd
*/
void findUsername(char* line, long lineSize, char** usr, long* uid){
  int colons = 0;
  int indexPreviousColon = 0;
  char* uidStr = (char*)malloc(lineSize*sizeof(char));

  for(int i=0; i < lineSize; i++) {
    if(line[i] == ':') { // delimiter
      colons++;
      if(colons == 1){ // username
        strncpy(*usr, line+indexPreviousColon, i-indexPreviousColon);
      }
      else if(colons == 3) {// user ID
        strncpy(uidStr, line+indexPreviousColon, i-indexPreviousColon);
        *uid = atol(uidStr);
        free(uidStr);
        return;
      }
      indexPreviousColon = i+1;
    }
  }
  strncpy(*usr, "", 1);
}

/* getUser - finds username based on real uid
   parameters:
   long origUID: long integer containing real uid of user that ran program

   return:
   char* usr: username that matches origUID
*/
char* getUser(long origUID){
  FILE* fp = fopen("/etc/passwd", "r");
  char * line = NULL;
  size_t len = 0;
  long lineSize;
  long uid;
  char* usr;

  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((lineSize = getline(&line, &len, fp)) != -1) {

    usr = (char*)calloc(lineSize,sizeof(char));
    findUsername(line, lineSize, &usr, &uid);
    if(origUID == uid) {  // Strings are equal
      fclose(fp);
      return usr;
    }
    free(usr);
  }

  printf("Error: Active user not found in /etc/passwd.\n");
  fclose(fp);
  exit(EXIT_FAILURE);
}

/* main - authenticates and validates runas credentials, then executes
   parameters:
   int argc: number of arguments
   char* argv[]: argument array of strings
*/
int main(int argc, char* argv[]){
  long origUID = getuid(); //geteuid for effective user
  long origGID = getgid(); //getegid for effective group
  char* origUser;
  char* runas = argv[1]; // user to be run as
  char* program = argv[2];
  long uid = -1;
  long gid = -1;
  pid_t  pid, wpid;
  int exit_status, set_uid_status, set_gid_status;
  int exec_result, chmod_result;
  FILE* fp;
  struct stat s;

  origUser = getUser(origUID);

  if(validateUser(runas, &uid, &gid)){
    if(validatePassword(origUser, runas)) {
      pid = fork();

      if(!pid){ //child
        set_gid_status = setgid(gid); //never use setuid at first because you cannot regain root privileges.
        set_uid_status = setuid(uid); //never use setuid at first because you cannot regain root privileges.

        if(!set_uid_status){
          exec_result = execvp(program, &argv[2]);

          if(exec_result == -1){
            printf("Error: execvp returned -1.\n");
            exit(EXIT_FAILURE);
          }
        }
      }
      else{ //parent
        wpid = wait(&exit_status); //wait for fork

        if(wpid == -1){
          printf("Error: wait() returned with -1.\n");
          exit(EXIT_FAILURE);
        }
        else{
          if (access("/var/tmp/", F_OK) != 0) {
            if (ENOENT == errno) {
              mkdir("/var/tmp/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // read/write/search permissions for owner and group, and read/search permissions for others.
            }
          }
          chmod_result = chmod("/var/tmp/runaslog", S_ISVTX | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); //change file permissions to read/write root, all read.
          fp = fopen("/var/tmp/runaslog", "a");

          int test = WEXITSTATUS(exit_status);
          printf("exit status: %d", exit_status);
          printf("wexitstatus: %d", test);
          fprintf(fp, "%d %s\n", exit_status, program);
          fclose(fp);
        }
      }
    }
    else{
      printf("Error: Invalid password.\n");
      exit(EXIT_FAILURE);
    }
  }
  else{
    printf("Error: User '%s' not found.\n", runas);
    exit(EXIT_FAILURE);
  }
  return 0;
}
