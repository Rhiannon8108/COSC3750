// wyls.c
// Rhiannon Kilduff
// COSC 3750 SP 2025
// Homework 5
// February 28, 2025 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "tar_header.h"


int main(int argc, char *argv[]) {
    if (argc < 3){
      fprintf(stderr, "Error: Not enough arguments provided. \n");
      fprintf(stderr, "Usage: wytar -c or -x  -f archive_name. \n ");

      return EXIT_FAILURE;
    }
    int cFlag = 0; 
    int xFlag = 0;
    int fFlag = 0; 
    int filePos = 0; 
    int posFlag = 0;
    char *archiveName = NULL;

    // Parse command-line arguments 
    for (int i = 1; i < argc && i < 4; i++){

      if(strcmp(argv[i], "-f") == 0) {

        archiveName = argv[i+1];
        fFlag = 1;
        i++;


      } else if (argv[i][0] == '-'){

        for (int j = 1;  j < strlen(argv[i]); j++){

          if(argv[i][j] == 'c'){
            cFlag = 1; 
            posFlag = i;
        } else if (argv[i][j] == 'x'){
            xFlag = 1; 
            posFlag = i;

        } else if (argv[i][j] == 'f'){

          if (argv[i][j+1] != '\0'){
            perror("Error: f must be the last flag \n");
            return 1; 
          }

            archiveName = argv[i+1];
            fFlag =1; 
          }else {
            perror(" Error: Invalid flag provided \n");
            return 1; 
        }

        if (cFlag && xFlag){
          perror("Error: you cannont extract and create");
          return 1; 
        }

        if(fFlag && (cFlag ^ xFlag)){
          i = 5; 

        }

      }
        
      }else {
        perror("Error: Invalid argument provided \n");
        return 1;

    }
  }

   if(!(fFlag && (cFlag ^ xFlag))){
     perror("No create or extract action given");
     return 1; 
    }



    if (cFlag) {
        if (argv[posFlag][2] == 'f') {
            filePos = posFlag + 2;
        } else if (argv[posFlag][1] == '3') {
            filePos = posFlag + 1;
        } else {
            filePos = posFlag + 3;
        }

    }


    FILE *archive = fopen(archiveName, "wb");
    if(!archive){
      perror("Failed to open Archive");{
      return 1;
      }
      int errCheck = 0; 
      errCheck = createArchive(archive, &argv[filePos]);
      if (errCheck == 1){
        
        printf("Error creating archive \n");
        fclose(archive);      }
    }

      return 0; 
}