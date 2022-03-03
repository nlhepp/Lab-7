#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int
free_heap(char **args, int *argnum, char* cpy){
    for(int j = 0; j < *argnum; j ++){
      free(args[j]);
    }
    free(args);
    free(cpy);

  return 0;
}

char **
tokenize_str(char *tokens, int  *argnum){
  // use string copy in finding number of args
  char *inputstr = strdup(tokens);
  char *token = strtok(inputstr, " ");

  // Count number of space deliminated arguments
  while( token != NULL){
    *argnum = *argnum+1;
    token = strtok(NULL, " ");
  }
  free(inputstr);

  // add an element for the NULL char
  *argnum = *argnum+1;
  char **args = malloc(sizeof(char*) * *argnum);
  fflush(stdout);
  // populate the args array with the arguments
  token = strtok(tokens, " ");
  int index = 0;
  while (token != NULL){
    args[index] = malloc(sizeof(token));
    args[index] = strdup(token);
    token = strtok(NULL, " ");
    index ++;
  }

  // add the final null char to args array and return
  args[index] = malloc(sizeof(char));
  args[index] = '\0';
  return args;
}

int
exec_cmd(char* cmd, char **args, int argn,  char *outputfile){
  // char*outfile = outputfile;
  // free(outputfile);  
  printf("cmd = %s\n", cmd);
  printf("argn = %d\n", argn);
  printf("outputfile = %s\n", outputfile);
  fflush(stdout);
  int forkval = fork();
  if (forkval == 0){
    // execute the command within the child process  

    if (strcmp("", outputfile) != 0){
      // redirect output to output.txt file
	    printf("Redirecting output\n");
	    fflush(stdout);
      int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(fd, fileno(stdout));
      close(fd);
      
    }
    // execute the command
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
  //int argnum = 2; // name of the proc and null
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
    return 0;
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

    // check if the escape string was entered
    if (strcmp(inputstr, escstr) == 0 ) {
      free(cpin);
      write(1, "escape str found\n", 18);
      return 0;
    }
    
    // variables to be used for executing the function
    int argnumint = 0;
    int *argnum = &argnumint;
    char ** args = NULL;
    char *outputfile = "";


    // check for redirection
    char *ptr = NULL;
    int rdrchr = '>';
    ptr = strchr( inputstr, rdrchr);

    // there is a redirection
    if (ptr != NULL){ 
      int redirects = 0;
      // check for muliple redirection symbols the old fashioned way
      for (int k = 0; k < strlen(inputstr); k++){
        if (inputstr[k] == '>'){
          redirects ++;
          if (redirects > 1){
            write(1, "Redirection misformatted.\n", 26);
            free(cpin);
            break;
          }
        }
      }
      if (redirects > 1){
        continue;
      }

      char *cmd_tokens = strtok(inputstr, ">");
      if (cmd_tokens == NULL){
        free(cpin);
        continue;
      }
      printf("In rdr, tokens is %s\n", cmd_tokens);
      printf("In rdr, tokens is %ld\n", strlen(cmd_tokens));
      fflush(stdout);
      
      char *rdrfile = strtok(NULL, ">");
      if (rdrfile == NULL){
        write(1, "Redirection misformatted.\n", 26);
        free(cpin);
        continue;
      }
      printf("In rdr, file is %s\n", rdrfile);
      printf("In rdr, file is %ld\n", strlen(rdrfile));
      fflush(stdout);

      int numfiles = 0;
      int *onames = &numfiles;
      char **outfiles = NULL;
      outfiles = tokenize_str(rdrfile, onames);
      
      for (int n = 0; n < *onames; n++){
        printf("outfiles %s\n", outfiles[n]);
      }
      
      // incorrect file output name
      if (*onames != 2 || outfiles[1] != NULL){
        printf("Incorrect formatting: mulitple output files\n");
        fflush(stdout);
        free_heap(outfiles, onames, cpin);
        continue;
      }
      
      outputfile = rdrfile;

      printf("Redirection\n");
      fflush(stdout);
      args = tokenize_str(cmd_tokens, argnum);

    } else{
      char *tokens = strdup(inputstr);

      args = tokenize_str(tokens, argnum);
      printf("Size of args = %d\n", *argnum);
      fflush(stdout);
      free(tokens);
    }


    // **FOR TESTING AND DEBUGGING**  print out args array
    for(int i = 0; i < *argnum; i ++){
      printf("index %d is %s\n", i, args[i]);
      fflush(stdout);
    }
    int exec_val = exec_cmd(args[0], args, *argnum, outputfile);

    free_heap(args, argnum, cpin);
 
/*     for(int j = 0; j < *argnum; j ++){
      free(args[j]);
    }
    free(args);
    free(cpin); */

    if (exec_val == -1){
      printf("-1 returned\n");
      fflush(stdout);
      return 0;
    }
  }
  
  return 0;
}
