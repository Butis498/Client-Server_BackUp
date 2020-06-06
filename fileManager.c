#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include<string.h>

#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>

void deleteFile(const char *fileName, const char *directory)
{
    char *file = malloc( (char*)(strlen(fileName)+strlen(directory)));
    sprintf(file,"%s/%s",directory,fileName);
    printf("%s\n",file);
    if (remove(file) == 0)
        printf("Deleted successfully\n");
    else
        printf("Unable to delete the file\n");
}

void CreateOrModifyFile(const char * fileName, const char * fileContet, const char *directory){

    FILE * file;
    char *fileFullPath = malloc( (char*)(strlen(fileName)+strlen(directory)));
    sprintf(file,"%s/%s",directory,fileName);

    file = fopen(fileFullPath, "w");


    fprintf(file,fileContet);


    fclose(file);

}

char *readFile(const char *fileName)
{

    FILE *file = fopen(fileName, "r");
    char *code;
    size_t n = 0;
    int c;

    if (file == NULL)
        return NULL; //could not open file
    fseek(file, 0, SEEK_END);
    long f_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    code = (char *)malloc(f_size);

    while ((c = fgetc(file)) != EOF)
    {
        code[n++] = (char)c;
    }

    code[n] = '\0';

    return code;
}

