// wyls.c
// Rhiannon Kilduff
// COSC 3750 SP 2025
// Homework 5
// February 28, 2025 
// This prorgam is a simple version of the ls utility written in C. 
// It is designed to list the contents of a directory in a simlar formate to ls-l


#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <pwd.h> 
#include <grp.h> 
#include <time.h>
#include <stdio.h>
#include <string.h> 
#include <stdint.h>

struct dirent *dPtr;
struct stat statBuffer;
struct passwd *pwd; 
struct group *grp; 
struct tm *tm; 
char dataString[256];

// Function to print the file permissions 
void printPermissions(mode_t mode){
  char permissions[11];
  // Determine file type 
  if (S_ISDIR(mode)) permissions[0] ='d';
  else if (S_ISLNK(mode)) permissions [0] = 'l';
  else permissions[0] = '-';

   // Owner permissions
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Group permissions
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Other permissions
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';

    printf("%s ", permissions);
}
// print owner and group owners 
void printOwner(uid_t uid, gid_t gid, int numeric) {
  // print user and group ID
    if(numeric){
      printf("%d %d ", uid, gid);
    }else {
      pwd = getpwuid(uid);
      grp = getgrgid(gid);

      if(pwd != NULL && grp != NULL){
        printf("%s %s " , pwd->pw_name, grp->gr_name);  
      }else{
        printf("%d %d ", uid, gid);
      }
    }
}
// Print the size
void printSize(off_t size, int humanReadable){
  if(humanReadable){
    const char *units[] = {"", "K ", "M ", "G "};
    int unitIndex = 0; 
    double humanSize = size;


    while (humanSize >= 1020 && unitIndex < 3){
        humanSize /= 1024; 
        unitIndex++; 
    }

    // Print with correct unit
    if(unitIndex == 0){
      printf("%.0f ", humanSize);
    }else {
      if (humanSize == (int)humanSize){
        printf("%.0f%s ", humanSize, units[unitIndex]);

      }else{
        printf("%.1f%s ", humanSize, units[unitIndex]);

      }
    }
     
  }else {
    printf("%ld " , (long)size);
  }
}

// Print the date and time
void printTime(time_t mTime){
  tm = localtime(&mTime);
  char timeStr[20];
  // check if file has been modified in the last 180 days 
  if(difftime(time(NULL), mTime) > 180 *24 * 60 *60 ){
    strftime(timeStr, sizeof(timeStr), "%b %d %y", tm);
   }else{
    strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", tm);
   }

   printf("%s ", timeStr);
}



int main(int argc, char **argv) { 
  int humanReadable = 0; 
  int numeric = 0; 
  int dirIndex = -1; 
// Parse command line arguments 
  for(int i =1; i < argc; i++) {
    if (argv[i][0] == '-'){
      // Process flags 
      for(int j = 1; j < strlen(argv[i]); j++){
    
        if(argv[i][j] == 'h'){
          humanReadable = 1; 
      
        }
         else if (argv[i][j] == 'n'){
          numeric = 1; 
          
        } else{
          char errorFlag = argv[i][j];

          printf("Usage: -%c is not a vaild flag\n", errorFlag);
        }
      }
    } else {
      // Set the directory index to the first non-flag argument 
      dirIndex = i; 
      break;
    }

  }


  DIR *dir;
  char path[256];
  if(dirIndex == -1){
    // If no directory given use cwd
    getcwd(path, sizeof(path));
    dir = opendir(path); 

  }else {
    // Use given directory
    strcpy(path, argv[dirIndex]);
    dir = opendir(path); 
  } 

  // Error if directory cannot be opened
  if(dir == NULL){
    perror("opendir");
    return 1;
  }

// Check each directory entry 
  int end = 0;
  while(!end) {
    dPtr = readdir(dir);
    if (dPtr == NULL){
      end = 1; 
    }else {
      // If "." or ".." encounter move on
      if (strcmp(dPtr->d_name, ".") == 0 || strcmp(dPtr->d_name, "..") == 0) {
        continue;
      }
      // Create the full path for the entry
      char fullPath[1024];
      snprintf(fullPath, sizeof(fullPath), "%s/%s", path, dPtr->d_name);  

      // Print the collected information 
      stat(fullPath, &statBuffer);
      printPermissions(statBuffer.st_mode);
      printOwner(statBuffer.st_uid, statBuffer.st_gid, numeric);
      printSize(statBuffer.st_size, humanReadable);
      printTime(statBuffer.st_mtime);
      printf("%s\n", dPtr->d_name);
      
    }
  }
  closedir(dir);
  return 0;
}
