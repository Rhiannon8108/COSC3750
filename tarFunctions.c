#include <limits.h> 
#include "tar_header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int createMissingDirectories(const char *path) {

  // Declare temp buffer to store path
  char temp[PATH_MAX];

  // Copy input path to temp biffer
  strncpy(temp, path, PATH_MAX);

// Get parent directorie of given path
  char *dir = dirname(temp);

  // Create directorie with read, write, execute permissions 
  if (mkdir(dir, 0755) == -1) {

      if (errno == ENOENT) {

          createMissingDirectories(dir); 
          // Recursively attempt to create parent directories
          mkdir(dir, 0755);
      } else if (errno != EEXIST) {
          perror("Error making dir");
          return 1;
      }
      return 0;
  }
}

int extractArchive(FILE *archive) {

  // holds header information for file in archive
  struct tar_header header;
  char buffer[512];
   // Loop through headers in the archive
  while (fread(&header, sizeof(header), 1, archive) == 1) {
      // if header name is empty the end as been reached
      if (header.name[0] == '\0') {
          break;
      }

      // Convert from octal string to size_t value
      size_t fileSize = strtol(header.size, NULL, 8);

      // Handle directories
      if (header.typeflag == '5') { // 5 means dir
          printf("Extracting directory: %s\n", header.name);

          // Attempt to create directory 
          if (mkdir(header.name, 0755) == -1 && errno != EEXIST) {
              perror(header.name);
              return 1; 
          }
          continue; // next header
      }

      // Handle symbolic links
      if (header.typeflag == '2') { // 2 means sym link

        // Attempt to create link
          printf("Extracting symbolic link: %s -> %s\n", 
            header.name, header.linkname);
          if (symlink(header.linkname, header.name) == -1) {
              perror(header.name);
              exit(EXIT_FAILURE);
          }
          continue; // next header 
      }

      // Handle regular files
      printf("Extracting file: %s\n", header.name);

      // open for writing
      FILE *file = fopen(header.name, "wb");
      if (!file) {
          if (errno == ENOENT) {
              createMissingDirectories(header.name);
              file = fopen(header.name, "wb");
              if (!file) {
                  perror(header.name);
                  return 1;
              }
          } else {
              perror(header.name);
              return 1; 
          }
      }

      // Write file contents

      // Track bytes left to read 
      size_t bytesRemaining = fileSize;
      while (bytesRemaining > 0) {
       // Read upto 512 bytes at a time 
          size_t bytesToRead = (bytesRemaining > 512) ? 512 : bytesRemaining;
          // read from archive 
          fread(buffer, 1, bytesToRead, archive);

          // write to file
          fwrite(buffer, 1, bytesToRead, file);
          // decrement bytes remaining 
          bytesRemaining -= bytesToRead;
      }

      fclose(file);

      // Skip padding
      if (fileSize % 512 != 0) {
        // move pointer past padding
          fseek(archive, 512 - (fileSize % 512), SEEK_CUR);
      }
  }
  return 0; 
}

unsigned int calculateChecksum(struct tar_header *header) {
    // initialize checksum value
    unsigned int sum = 0;
    
    // treat header as array of bytes 
    unsigned char *p = (unsigned char *)header;
    
    // Add up bytes od the header excluding the checksum field
    for (int i = 0; i < 148; i++)
        sum += p[i]; // bytes before checksum 
    for (int i = 148; i < 156; i++)
        sum += ' '; // checksum field treated as spaces 
    for (int i = 156; i < 512; i++)
        sum += p[i]; // bytes after checksum 
    
    return sum;
}


void createHeader(struct tar_header *header,
                   const char *filename,
                   const struct stat *statbuf) {
  // Set header fields to 0;
    memset(header, 0, sizeof(struct tar_header));
    
    // Handle filename (must not exceed 100 chars)
    if (strlen(filename) > 100) {

      // find the last '/' in the filename
      //split into prefix and basename
        const char *basename = strrchr(filename, '/');

        if (!basename) {
          // copy entire filename to header 
            strncpy(header->name, filename, 100);

        } else {
          // Move past '/'
            basename++;
            // calculate prefix
            size_t prefix_len = basename - filename - 1;
            if (prefix_len > 155) prefix_len = 155;
            // copy prefix into 'prefix field' 
            strncpy(header->prefix, filename, prefix_len);
            // copy name into 'name' field 
            strncpy(header->name, basename, 100);
        }
    } else {
      // if the filename is less than 100 characters 
        strncpy(header->name, filename, 100); // copy to name field
    }
    // Populate the permissions User ID and Group ID 
    // And Modification Time Fields 
    snprintf(header->mode, 8, "%07o", statbuf->st_mode & 07777);
    snprintf(header->uid, 8, "%07o", statbuf->st_uid);
    snprintf(header->gid, 8, "%07o", statbuf->st_gid);
    snprintf(header->mtime, 12, "%011lo", (unsigned long)statbuf->st_mtime);

    // Handel Diretories 
    if (S_ISDIR(statbuf->st_mode)) {
        header->typeflag = '5';
        snprintf(header->size, 12, "%011o", 0);
        // Make sure dir name ends with '/'
        if (header->name[strlen(header->name)-1] != '/') {
            strncat(header->name, "/", 
              sizeof(header->name) - strlen(header->name) - 1);
        }
    } 
    // Handel sym-Links
    else if (S_ISLNK(statbuf->st_mode)) {
        header->typeflag = '2';
        snprintf(header->size, 12, "%011o", 0);
        ssize_t len = readlink(filename, header->linkname, 
                      sizeof(header->linkname));
        if (len == -1) {
            perror(filename);
            return;
        }
        header->linkname[len] = '\0';
    } 
    // Handel regular file
    else if (S_ISREG(statbuf->st_mode)) {
        header->typeflag = '0';
        snprintf(header->size, 12, "%011lo", 
                (unsigned long)statbuf->st_size);
    }
    // Identify tar format 
    strncpy(header->magic, "ustar", 6);
    
    header->version[0] = '0';
    header->version[1] = '0';
    
    // Compute Checksum for header 
      // Intialize field with ' '
    memset(header->chksum, ' ', 8); 
    unsigned int checksum = calculate_checksum(header);
      // Write checksum in octal format
    snprintf(header->chksum, 8, "%06o", checksum);
      // Ensure final character is white space
    header->chksum[7] = ' ';
}

int writeFileContents(FILE *archive, const char *filename,
                         size_t filesize) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        return 1;
    }
    // Buffer to hold blocks of file data
    char buffer[512];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytes_read, archive);
    }
    // Read the contetns of file 
    // Write them to the archive 
    if (filesize % 512 != 0) {
        size_t padding = 512 - (filesize % 512);
        memset(buffer, 0, padding);
        fwrite(buffer, 1, padding, archive);
    }

    fclose(file);
}

// Handel nested directories 
int processDirectory(FILE *archive, const char *dirname) {
    DIR *dir = opendir(dirname);
    if (!dir) {
        perror(dirname);
        return 1;
    }
    // Poniter to hold directories 
    struct dirent *entry;

    int empty = 0;
    // Iterate through each directory entry 
    while (!empty) {

      entry = readdir(dir);
      if(entry == NULL){
        empty = 1;
        break;
    }
      if (strcmp(entry->d_name, ".") == 0 || 
          strcmp(entry->d_name, "..") == 0)
          continue;

      char fullpath[PATH_MAX];

      // Construct full path of the current directory 
      snprintf(fullpath, sizeof(fullpath), "%s/%s", dirname, entry->d_name);

      // strcut to hold file metadata 
      struct stat statbuf;

      // Retreive metadata 
      if (stat(fullpath, &statbuf) == -1) {
          perror(fullpath);
          continue;
      }

      // Create tar header for this entry
      struct tar_header header;
      create_header(&header, fullpath, &statbuf);
      fwrite(&header, sizeof(header), 1, archive);

    int errCheck = 0;

      // Handel Regular Files 
      if (S_ISREG(statbuf.st_mode)) {
          write_file_contents(archive, fullpath, statbuf.st_size);
      }
      else if (S_ISDIR(statbuf.st_mode)) {
          // Recursively process subdirectory
          errCheck = process_directory(archive, fullpath);
          if (errCheck ==1 ){

            printf("Error processing directory \n");
            return 1;

          }

      } else if S_ISLNK(statbuf.st_mode){
        continue;

      } else {
        perror("File type unknown");

        return 1; 
      }
  }
    closedir(dir);
    return 0; 
}

int createArchive(FILE *archive, char **files) {

  // Iterate through the files array  
    for (int i = 0; files[i] != NULL; i++) {
        struct stat statbuf; // Holds metadata 

        // Get metadata for current file/directory 
        if (lstat(files[i], &statbuf) == -1) {
            perror(files[i]);
            continue;
        }

        // Create header for current file/directory 
        struct tar_header header;
        create_header(&header, files[i], &statbuf);
        fwrite(&header, sizeof(header), 1, archive);


        int errCheck = 0;

        if (S_ISREG(statbuf.st_mode)) {

            errCheck = write_file_contents(archive, 
                      files[i], statbuf.st_size);
            if (errCheck == 1) {
              printf("Error writing file contents");
              return 1; 
            }
        }
        else if (S_ISDIR(statbuf.st_mode)) {

            errCheck = process_directory(archive, files[i]);

            if (errCheck == 1){
              printf("Error processing directory"){
                return 1;
              }
            }
        } 
        else if (S_ISLNK(statbuf.st_mode)){
          continue;
        }else {
          perror("File type unrecognized");
          exit(1); 
        }

        
    }

    // End of archive markers
    char empty_block[512] = {0};
    fwrite(empty_block, sizeof(empty_block), 1, archive);
    fwrite(empty_block, sizeof(empty_block), 1, archive);
}

