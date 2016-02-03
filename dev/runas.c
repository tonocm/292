#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE 100

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

int validateUser(char* user, long* uid, long* gid){
  FILE* fp = fopen("/etc/passwd", "r");
  char * line = NULL;
  size_t len = 0;
  long lineSize;
  char* usr;

  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((lineSize = getline(&line, &len, fp)) != -1) {

    usr = (char*)calloc(lineSize,sizeof(char));
    getUsrInfo(line, lineSize, &usr, &uid, &gid);
    if(strcmp(usr, user) == 0) {  // Strings are equal
      return 1;
//      printf("we've found a user!\n");
//      printf("uname: %s uid: %ld gid: %ld\n", usr, *uid, *gid);
    }
    free(usr);
  }

  return 0;
}

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

int validatePassword(char* user1, char* user2){

  FILE* fp = fopen("./perms", "r");
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
//      printf("%s %s %s\n", u1, u2, pwd);
      return 1;
    }
    free(u1);
    free(u2);
    free(pwd);
  }
  return 0;
}

void findUsername(char* line, long lineSize, char** usr, long *uid){
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
      return usr;
    }
    free(usr);
  }

  printf("Error: Active user not found in /etc/passwd.\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){
  long origUID = getuid(); //geteuid for effective user
  long origGID = getgid(); //getegid for effective group
  char* origUser;
  char* runas = argv[1]; // user to be run as
  char* program = argv[2];

  char* args = argv[3]; //char *const argv[]
  long uid = -1;
  long gid = -1;
  pid_t  pid, wpid;
  int exit_status, set_uid_status;
  int exec_result;
  origUser = getUser(origUID);

//  if(validateUser(runas, &uid, &gid)){
//    if(validatePassword(origUser, runas)) {
  if(1){
    if(1) {
//      printf("Authentication successful.\n");
//      printf("effective user id: %d\n", geteuid());
//      printf("user id: %d\n", getuid());

      //magic happens here
      pid = fork();

      if(!pid){ //child
        set_uid_status = setuid(uid); //never use setuid at first because you cannot regain root privileges.
        // Actually, for security purposes, I believe i should use setuid so that no program passed can just gain root again.

        if(!set_uid_status){
          printf("New uid: %d.\n", getuid());
//          char* test[] = {"ls", "-a", NULL};
//          exec_result = execvp("echo", test);

//          exec_result = execvp(program, args);
          exec_result = execvp(program, &program);

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
          printf("Child's exit status was %d\n", exit_status);
          //now do the log.
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
