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
#include <dirent.h>
#include <netdb.h>
#include <sys/socket.h>

void deleteFile(const char *fileName, const char *directory)
{
    int size = strlen(fileName) + strlen(directory) + 3;
    char *file = (char *)malloc(size * sizeof(char));
    //filename = (char*)malloc(strlen(temp)+1);

    sprintf(file, "%s/%s", directory, fileName);
    printf("fileName: \"%s\"\n", fileName);
    //sprintf(file, "%s/%s", directory, "0.txt");
    printf("%s\n", file);
/*
    FILE* fp;
    fp = open(file, "r");
    if(fp == NULL) printf("Database error\n");
    else {
        fp.close()
        if(remove(file)) printf("Unable to delete the file\n");
        else printf("Delete successful\n");
    }
*/
    //chdir(directory);

    char buffer[1024];
    printf("Current directory: %s\n", getcwd(buffer, sizeof(buffer)));

    int status = remove(file);

    if (status == 0)
        printf("Deleted successfully\n");
    else
        //perror("Unable to delete the file, error code is: %d\n", status);
        perror("Unable to delete the file, error code is: \n");

    free(file);
}

/*
void deleteFile(const char *fileName, const char *directory)
{
    int size = strlen(fileName) + strlen(directory);
    char *file = (char *)malloc(size * sizeof(char));
    sprintf(file, "./UpdatedServerFolder/0.txt");
    //sprintf(file, "1.txt");
    printf("%s\n", file);
    if (remove(file) == 0)
        printf("Deleted successfully\n");
    else
        printf("Unable to delete the file\n");

    free(file);
}
*/

void CreateOrModifyFile(const char *fileName, const char *fileContet, const char *directory)
{

    FILE *file;
    int size = strlen(fileName) + strlen(directory);
    char *fileFullPath = (char *)malloc(size * sizeof(char));
    sprintf(fileFullPath, "%s/%s", directory, fileName);

    file = fopen(fileFullPath, "w");

    fprintf(file, "%s", fileContet);

    free(fileFullPath);
    fclose(file);
}

int deleteDirectory(const char *directory)
{
    DIR *d = opendir(directory);
    size_t path_len = strlen(directory);
    int r = -1;

    if (d)
    {
        struct dirent *p;

        r = 0;
        while (!r && (p = readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf)
            {
                struct stat statbuf;

                snprintf(buf, len, "%s/%s", directory, p->d_name);
                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = deleteDirectory(buf);
                    else
                        r2 = unlink(buf);
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }

    if (!r)
        r = rmdir(directory);

    return r;
}

void createDirectory(const char *directory, const char *dirName)
{

    struct stat st = {0};
    int size = strlen(dirName) + strlen(directory);
    char *dirFullPath = (char *)malloc(size * sizeof(char));

    sprintf(dirFullPath, "%s/%s", directory, dirName);
    int check;

    check = mkdir(dirFullPath, 0777);

    // check if directory is created or not
    if (!check)
        syslog(LOG_NOTICE, "Directory created\n");
    else
    {
        syslog(LOG_NOTICE, "Unable to create directory\n");
        exit(1);
    }

    
}

void renameFile(const char *directory, const char* oldName ,const char * newName){

    int size = strlen(oldName) + strlen(directory);
    char *fileFullPath = (char *)malloc(size * sizeof(char));
    sprintf(fileFullPath, "%s/%s", directory, oldName);

    size = strlen(newName) + strlen(directory);
    char *fileFullPathNew = (char *)malloc(size * sizeof(char));
    sprintf(fileFullPathNew, "%s/%s", directory, newName);


    if (rename(fileFullPath, fileFullPathNew) == 0)
    {
        syslog(LOG_NOTICE, "File renamed successfully.\n");
    }
    else
    {
        syslog(LOG_NOTICE, "Unable to rename files. Please check files exist and you have permissions to modify files.\n");
    }

}

void renameDirectory(const char *directory, const char* oldName ,const char * newName){
    
    int size = strlen(oldName) + strlen(directory);
    char *fileFullPath = (char *)malloc(size * sizeof(char));
    sprintf(fileFullPath, "%s/%s", directory, oldName);

    size = strlen(newName) + strlen(directory);
    char *fileFullPathNew = (char *)malloc(size * sizeof(char));
    sprintf(fileFullPathNew, "%s/%s", directory, newName);


    if (rename(fileFullPath, fileFullPathNew) == 0)
    {
        syslog(LOG_NOTICE, "Directory renamed successfully.\n");
    }
    else
    {
        syslog(LOG_NOTICE, "Unable to rename directory. Please check directory exist and you have permissions to modify files.\n");
    }
}

void moveFile(){

}

void moveDirectory(){

    
}
