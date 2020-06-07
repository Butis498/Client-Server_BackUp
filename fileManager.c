/*
Helper functions for file management
*/

//dump of headers
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>

//deletes a file by its name and directory
void deleteFile(const char *fileName, const char *directory)
{
    int size = strlen(fileName)+strlen(directory);
    char *file = (char*) malloc(size * sizeof(char));
    sprintf(file,"%s/%s",directory,fileName);
    printf("%s\n",file);

    if (remove(file) == 0)
        printf("Deleted successfully\n");
    else
        printf("Unable to delete the file\n");
    
    free(file);
}

//create or modify a file by its name and directory, and by placing the fileContet string as its content
void CreateOrModifyFile(const char * fileName, const char * fileContet, const char *directory){

    //open the file
    FILE * file;
    int size = strlen(fileName)+strlen(directory);
    char *fileFullPath = (char*) malloc(size * sizeof(char));
    sprintf(fileFullPath,"%s/%s",directory,fileName);
    file = fopen(fileFullPath, "w");

    //write the fileContent inside the file
    fprintf(file,"%s",fileContet);
    
    free(fileFullPath);
    fclose(file);
}

//returns file contents as a single string
char *readFile(const char *fileName) {

    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;
    
    if (file == NULL)
        return NULL; //could not open file

    //set file position in the sream to the end of the file
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    //returns the file position in the sream to the beggining of the file
    fseek(file, 0, SEEK_SET);
    code = (char *)malloc(f_size);

    while ((c = fgetc(file)) != EOF) {//copy the file characters one at a time
        code[n++] = (char)c;
    }
    code[n] = '\0';

    return code;
}


