// wycat.c
// Rhiannon Kilduff
// COSC 3750 SP 2025
// Homework 4
// February 18, 2025 
// This prorgam is a simple version of the cat utility written in C. 
// It is designed to read from both standard input and given files.
// Then output file contents and standard input to standard output 

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096 

// int process_file(File *file)
// parameters: file is a readable file pointer 
// returns: 0
int process_file(FILE *file) {
  char buffer[BUFFER_SIZE];
  size_t bytesRead;
  size_t bytesWritten;
  int bufferEmpty = 0;
  while (!bufferEmpty){
    bytesRead = fread(buffer, 1, BUFFER_SIZE, file);

    if(bytesRead < BUFFER_SIZE ){
        bufferEmpty = 1;
    } 
      bytesWritten = fwrite(buffer, 1, bytesRead, stdout);

       if(bytesRead != bytesWritten){
        return 1; 

    }

  }
  fflush(stdout);
  return 0;
}


// function reads from stdin 
// returns the return code from process_file(stdin)
int process_stdin() {
  return process_file(stdin);
}

int main(int argc, char *argv[]) {
  int returnValue;

  // if no argument is given just write to stdin
  if(argc ==1){
    int returnValue = process_stdin(stdin);
    if(returnValue == 1){
      perror("Error writing to stdout.");
    }
  } else {
    // process arguments 
    for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] == '\0'){
        // process stdin if no argument given
        returnValue = process_stdin(stdin);
        if(returnValue == 1){
          perror("Error writing to stdout");
        }

      } else {
        // open the next file at argv index and right it stdout as long as there is no error. 
        // if an error occurs throw an error and continue to next argument 
        // call process_file function then close the file 
        FILE *file = fopen(argv[i], "r");
        if (file == NULL){
          perror(argv[i]);
          continue;
        }
         returnValue = process_file(file);
          if (returnValue == 1){
            perror("Error writing to stdout");
          }
          //process_file(file);
          fclose(file);
        }
      }
   
     return 0; }
  }



// README Ishita Patel, Lily Brongo, Alex Bryan, Dainel Collins 