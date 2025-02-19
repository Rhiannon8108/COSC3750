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

// function reads in file to buffer and writes file contents to stdout 
void process_file(FILE *file) {
  char buffer[BUFFER_SIZE];
  size_t bytesRead;

  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file))){
    fwrite(buffer, 1, bytesRead, stdout);
  }
// if there is an error the perror() function produces a message on stderr. 
  if(ferror(file)){
    perror("Error");
  }

}


// function reads from stdin 
void process_stdin() {
  process_file(stdin);

}

int main(int argc, char *argv[]) {

  // if no argument is given just write to stdin
  if(argc ==1){
    process_stdin();
  } else {
    // process arguments 
    for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] == '\0'){
        //process stdin if no argument given
        process_stdin();

      } else {
        // open the next file at argv index and right it stdout as long as there is no error. 
        // if an error occurs throw an error and continue to next argument 
        // call process_file function then close the file 
        FILE *file = fopen(argv[i], "r");
        if (file == NULL){
          perror(argv[i]);
          continue;

        } else {
          process_file(file);
          fclose(file);
        }
      }
    }
  }

  return 0;
}
