#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int
exit_mysh(char **args, int argnum, char* cpy){
  free(cpy);
  for(int j = 0; j < argnum; j ++){
    free(args[j]);
  }
  return 0;
}


int
exec_cmd(char* cmd, char **args, int argn,  char *outputfile){
  // char*outfile = outputfile;
  // free(outputfile);
  int forkval = fork();
  printf("cmd = %s\n", cmd);
  fflush(stdout);
  // execute the command within the child process  
  
    if (forkval == 0){
    // redirect output to output.txt file
      if (argn == 4){
        if (strcmp(args[1], ">") == 0){
	  printf("Here in redirect\n");
	  fflush(stdout);
          int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
          dup2(fd, fileno(stdout));
          close(fd);
        }
      }
    //char *arglist[] = {"testprog", NULL};
    execv(cmd, args);
    // an error occurred in execution
    printf("Error occured executing command \n");
    fflush(stdout);
    return -1;
  } else{
    int status;

    // wait until child process finishes
    waitpid(forkval, &status, 0);
    printf("Child has finished with status %d\n", status);
    return 0;
  }

}


int
main(int argc, char** argv) {
  /*
   * These are just a few strings that are useful to have saved
   * I am not positive how large the input might be and how to
   * handle it yet. I saw something about 512 chars in the description
   * */
  int argnum = 2; // name of the proc and null
  char inputstr[513]; 
  char *prompt = "mysh> ";
  char *escstr = "exit";
  FILE *batchInput; // batch file reference if needed

  // if more than 2 args just exit
  if (argc > 2) {
    write(2, "Usage: mysh [batch-file]\n", 25);
    exit(1);
  }

// BATCH MODE
  if (argc == 2){
    // try to open file.  If it fails return
    if ((batchInput = fopen(argv[1], "r")) == NULL) {
      printf("Error: Cannot open file %s.\n", argv[1]); // TODO Change to write?
      exit(1);
    }
    while (1) {

      // if end of file, exit
      if (fgets(inputstr, 513, stdin) == NULL) {
        write(1, "\n", 1);
        exit(0);
      }

      // Dealing with input over 512
      // if last char of inputstr is not a newline then continue to next input
      // flush extra buffer inside before continuing
      if (inputstr[strlen(inputstr) - 1] != '\n') {
        // TODO Specification says we should still echo here
        while (!strchr(inputstr, '\n')) {
          fgets(inputstr, 513, stdin);
        }
      continue;
      }

    // duplicating the input string so we have the copy while being able to parse and modify the original
    char *cpin = strdup(inputstr);

    // there may be a better way to remove the final newline char, but this works for now
    inputstr[strlen(inputstr)-1] = '\0';
    char *tokens = strdup(inputstr);

    // check if the escape string was entered
    if (strcmp(inputstr, escstr) == 0 ) {
      free(cpin);
      write(1, "escape str found\n", 18);
      return 0;
    }
    free(cpin);
    free(tokens);
    }

    return -1;
  }

// INTERACTIVE
  // loop for the shell until the exit command or Ctrl-D is found
  while (1) {
    // print prompt to user
    write(1, prompt, strlen(prompt));
    
    // get input from user & detect Ctrl D/EOF
    if (fgets(inputstr, 513, stdin) == NULL) {
      write(1, "\n", 1);
      exit(0);
    }

    // Dealing with input over 512
    // if last char of inputstr is not a newline then continue to next input
    // flush extra buffer inside before continuing
    if (inputstr[strlen(inputstr) - 1] != '\n') {
      while (!strchr(inputstr, '\n')) {
        fgets(inputstr, 513, stdin);
      }
      continue;
    }
    
    // duplicating the input string so we have the copy while being able to parse and modify the original
    char *cpin = strdup(inputstr);
    
    // there may be a better way to remove the final newline char, but this works for now
    inputstr[strlen(inputstr)-1] = '\0';
    char *tokens = strdup(inputstr);

    // check if the escape string was entered
    if (strcmp(inputstr, escstr) == 0 ) {
      free(cpin);
      write(1, "escape str found\n", 18);
      return 0;
    }

    // find how many args have been input and print result
    argnum = 0;
    char *token = strtok(inputstr, " ");
    
    // What is the point of this?
    while( token != NULL){
      argnum ++;
      //printf("arg %d = %s\n", argnum-1, token);
      //fflush(stdout);
      token = strtok(NULL, " ");

    }
    // add an element for the NULL char
    argnum ++;

    char **args = malloc(sizeof(char*) * argnum);
    token = strtok(tokens, " ");
    
    int index = 0;
    while (token != NULL){
      args[index] = malloc(sizeof(token));
      args[index] = strdup(token);
      token = strtok(NULL, " ");
      index ++;
    }
    free(tokens);

    args[index] = malloc(sizeof(char));
    args[index] = '\0';
    
    // create args array and populate it
    for(int i = 0; i < argnum; i ++){
      printf("index %d is %s\n", i, args[i]);
      fflush(stdout);
    }
    char *outputfile = malloc(sizeof(char)*2);
    outputfile = "x";
    /*char *outputfile = malloc(sizeof(char));
    outputfile = "";
    if (argnum > 3){
      if (strcmp(args[argnum-3], ">") == 0){
        printf("found redirection\n");
        fflush(stdout);
        outputfile = malloc(sizeof(args[argnum-2]));
        outputfile = args[argnum-2];
      }
    }
    */
    int exec_val = exec_cmd(args[0], args, argnum, outputfile);
    for(int j = 0; j < argnum; j ++){
      free(args[j]);
    }
    
    free(args);
    free(cpin);
    if (exec_val == -1){
      printf("-1 returned\n");
      fflush(stdout);
      return 0;
    }
  }
  
  return 0;
}
