#include "client.c"

#include <dirent.h>
#include <pthread.h>

#define EVENT_SIZE (sizeof(struct inotify_event)) //Size of the inotify_event structure that describes a watched filesystem event
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

//structure to store a monitored directory
typedef struct monitoredDir
{
    char *path;
    pthread_t *thread;
    int alive; //set to 0, to kill the tread, and stop monitoring
} monitoredDir;

//Node for a linked list of monitored directories
typedef struct Node
{
    monitoredDir data;
    struct Node *next;
} Node;

pthread_mutex_t lock;
pthread_mutex_t list_lock;
pthread_mutex_t serverRequestLock;

int size_of_rootDirName;
char *ROOTDIR;

//function declarations
void sig_func(int sig);
Node *newThreadForSubDir(char *nested_dir, Node *tail);
Node *inotifyMonitor(char *current_dir, Node *head, Node *tail);
void *directoryMonitorThread(void *dirName);
int monitor(char *rootDir);

//Catch the signall to stop thread
void sig_func(int sig)
{

    write(1, "Caught signal: ", 15);

    char id[16];
    snprintf(id, 16, "%d", (int)pthread_self());

    write(1, id, 16);
    write(1, "\n", 1);

    //signal(SIGSEGV,sig_func);
    pthread_exit(NULL);
}

//is called in order to monitor some new directory, while adding its thread to the linked list of the parent
Node *newThreadForSubDir(char *nested_dir, Node *tail)
{

    syslog(LOG_NOTICE, "NEW DIR ADDED TO BE MONITORED: \t%s\n", nested_dir);

    //recursive call to directoryMonitorThread by creating another thread with it
    pthread_t *newThread = (pthread_t *)malloc(sizeof(pthread_t));
    int err = pthread_create(newThread, NULL, &directoryMonitorThread, (void *)nested_dir);

    if (err != 0)
    {
        syslog(LOG_NOTICE, "\ncan't create thread for monitor of dir %s, ERROR:[%s]\n", nested_dir, strerror(err));
    }
    else
    { //push the newly created monitor to the linked list

        monitoredDir newMonitor;

        //fill the new monitr params, to push it to the list
        newMonitor.path = nested_dir;
        newMonitor.thread = newThread;
        newMonitor.alive = 1;

        pthread_mutex_lock(&list_lock);
        //push the monitor to the list, and update the tail
        tail->data = newMonitor;
        tail->next = (Node *)malloc(sizeof(Node));
        tail->next->next = NULL;

        tail = tail->next;
        pthread_mutex_unlock(&list_lock);
    }

    return tail;
}

//original monitor
//uses inotify to detect changes inside the directory and logs them to the syslog
Node *inotifyMonitor(char *current_dir, Node *head, Node *tail)
{

    int length, i = 0;
    int fd; //file descriptor
    int wd; //watch descriptor
    char buffer[BUF_LEN];

    //initializes a new inotify instance and returns a file descriptor
    fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
    }

    //adds a new watch for the file whose location is specified in current_dir
    //returns a unique watch descriptor for this inotify instance
    wd = inotify_add_watch(fd, current_dir,
                           IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVE | IN_MOVED_TO); //Bit mask of the events to be monitored

    length = read(fd, buffer, BUF_LEN);
    if (length < 0)
    {
        perror("read");
    }

    //for every item in the buffer as read from the fd file descriptor
    while (i < length)
    {
        struct inotify_event *event = (struct inotify_event *)&buffer[i];

        
        if (event->len && !((event->name)[0] == '.'))
        {
            char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir) + 2) * sizeof(char));
            sprintf(new_dir, "%s/%s", current_dir, event->name);

            //if ((event->name)[0] == '.') syslog(LOG_NOTICE, "FIRST CHAR = %c \n", (event->name)[0]);
            
            syslog(LOG_NOTICE, "EVENT DETECTED: modified path = %s \n", new_dir);

        //MUTEX LOCK
        pthread_mutex_lock(&serverRequestLock);

            if (event->mask & IN_CREATE)
            {
                if (event->mask & IN_ISDIR)
                {

                    syslog(LOG_NOTICE, "The directory %s was created.\n", new_dir);
                    tail = newThreadForSubDir(new_dir, tail); //monitor the newly created directory

                    //sendCreateDirectoryPetition(current_dir + size_of_rootDirName, event->name);
                }
                else
                {

                    syslog(LOG_NOTICE, "The file %s was created.\n", new_dir);
                    sendCreateFilePetition(event->name, current_dir + size_of_rootDirName);
                }
            }
            else if (event->mask & IN_DELETE)
            {
                if (event->mask & IN_ISDIR)
                {

                    syslog(LOG_NOTICE, "The directory %s was deleted.\n", new_dir);
                    sendDeleteDirectoryPetition(new_dir + size_of_rootDirName);
                }
                else
                {

                    syslog(LOG_NOTICE, "The file %s was deleted.\n", new_dir);
                    sendDeleteFilePetition(event->name, current_dir + size_of_rootDirName);
                }
            }
            else if (event->mask & IN_MODIFY)
            {
                if (event->mask & IN_ISDIR)
                {

                    syslog(LOG_NOTICE, "The directory %s was modified.\n", new_dir);
                    //
                }
                else
                {

                    syslog(LOG_NOTICE, "The file %s was modified.\n", new_dir);
                    syslog(LOG_NOTICE,"%s" ,readFile(new_dir));
                    sendModifyFilePetition(event->name, readFile(new_dir), current_dir + size_of_rootDirName);
                }
            }
            else if (event->mask & IN_MOVED_TO)
            {
                if (event->mask & IN_ISDIR)
                {

                    syslog(LOG_NOTICE, "The directory %s was moved.\n", new_dir);
                    tail = newThreadForSubDir(new_dir, tail); //monitor the new name of the modified directory

                    //sendCreateDirectoryPetition(current_dir + size_of_rootDirName, event->name);
                }
                else
                {
                    syslog(LOG_NOTICE, "The file %s was moved.\n", new_dir);
                    /* 
                        Mission impementation
                    */
                    sendModifyFilePetition(event->name, readFile(new_dir), current_dir + size_of_rootDirName);
                }
            }
            else if (event->mask & IN_MOVED_FROM)
            {
                if (event->mask & IN_ISDIR)
                {
                    char *new_string = (char *)malloc((strlen(new_dir) - size_of_rootDirName + 2) * sizeof(char));
                    sprintf(new_string, "%s", new_dir + size_of_rootDirName);
                    syslog(LOG_NOTICE, "The directory %s was moved out.\n", new_string);
                    sendDeleteDirectoryPetition(new_string);
                }
                else
                {

                    syslog(LOG_NOTICE, "The file %s was moved out.\n", new_dir);
                    sendDeleteFilePetition(event->name, current_dir + size_of_rootDirName);
                }
            }
        
        pthread_mutex_unlock(&serverRequestLock);
        }
        i += EVENT_SIZE + event->len;
    }

    (void)inotify_rm_watch(fd, wd);
    (void)close(fd);

    return tail;
}

//Monitor thread
//thread function used when monitoring a directory
//first recursively creates directoryMonitorThread for all its sub directories and then infinetly loops while monitoring its own directory.
void *directoryMonitorThread(void *dirName)
{
    //Local linked list of nested monitoredDir for this directory thread
    Node *head; //monitoredDir linked list head
    Node *tail; //used to push to the end of the linked list

    head = (Node *)malloc(sizeof(Node));
    head->next = NULL;
    tail = head;

    //create copy of the directory name
    char *current_dir = (char *)(malloc((strlen((char *)dirName) + 1) * sizeof(char)));
    sprintf(current_dir, "%s", (char *)dirName);

    syslog(LOG_NOTICE, "CREATING MONITOR-THREAD FOR DIR: %s\n", current_dir);

    syslog(LOG_NOTICE, "IS FIRST?: %d\n", strncmp(current_dir, ROOTDIR, strlen(current_dir)));

    //MUTEX LOCK
pthread_mutex_lock(&serverRequestLock);
    //if not the root dir, then create a directory in the server
    if(!(strncmp(current_dir, ROOTDIR, strlen(current_dir)) == 0)){

        char *dirWithoutRoot;

        int fullDirLen = strlen(dirName);
        int lastSlashIndex = fullDirLen - 1;
        for (; lastSlashIndex >= 0 && current_dir[lastSlashIndex] != '/'; lastSlashIndex--){}

        syslog(LOG_NOTICE, "lastSlashIndex = %d\n", lastSlashIndex);

        char *dirWithoutTarget = (char*)malloc((lastSlashIndex + 1) * sizeof(char));
        strncpy(dirWithoutTarget, current_dir, lastSlashIndex);
        dirWithoutTarget[lastSlashIndex] = '\0';
        

        sendCreateDirectoryPetition(dirWithoutTarget + size_of_rootDirName, current_dir + lastSlashIndex + 1);
    }
pthread_mutex_unlock(&serverRequestLock);


    //Get contents of the directory
    DIR *d;
    struct dirent *dir;
    d = opendir(current_dir);

    if (d)
    {
        while ((dir = readdir(d)) != NULL) //read all the contents of the directory
        {
            //execute only on nested directories, (d_type == 4 for a drectory, 8 for a file)
            //creates a new thread of directoryMonitorThread for each sub directory
            if (dir->d_type == 4 && dir->d_name[0] != '.')
            {

                //copy of the new nested dir name concatenated to the current dir
                char *nested_dir = (char *)(malloc((strlen(dir->d_name) + strlen(current_dir) + 2) * sizeof(char)));
                sprintf(nested_dir, "%s/%s", current_dir, dir->d_name);

                syslog(LOG_NOTICE, "    %s IS NESTED IN %s\n", nested_dir, current_dir);

                //recursive call to directoryMonitorThread by creating another thread with it
                pthread_t *newThread = (pthread_t *)malloc(sizeof(pthread_t));
                int err = pthread_create(newThread, NULL, &directoryMonitorThread, (void *)nested_dir);

                if (err != 0)
                {
                    syslog(LOG_NOTICE, "\ncan't create thread for monitor of dir %s, ERROR:[%s]\n", nested_dir, strerror(err));
                }
                else
                { //push the newly created monitor to the linked list

                    monitoredDir newMonitor;

                    //fill the new monitr params, to push it to the list
                    newMonitor.path = nested_dir;
                    newMonitor.thread = newThread;
                    newMonitor.alive = 1;

                 pthread_mutex_lock(&list_lock);
                    //push the monitor to the list, and update the tail
                    tail->data = newMonitor;
                    tail->next = (Node *)malloc(sizeof(Node));
                    tail->next->next = NULL;

                    tail = tail->next;
                pthread_mutex_unlock(&list_lock);
                }
            }else if (dir->d_type == 8 && dir->d_name[0] != '.'){ //for file wich arent a directory
            pthread_mutex_lock(&serverRequestLock);

                //copy of the new nested dir name concatenated to the current dir
                char *nested_dir = (char *)(malloc((strlen(dir->d_name) + strlen(current_dir) + 2) * sizeof(char)));
                sprintf(nested_dir, "%s/%s", current_dir, dir->d_name);

                //obtain name wiyhout root dir
                int fullDirLen = strlen(dirName);

                syslog(LOG_NOTICE, "CREATING NEW FILE : %s, %s, CONTENTS:\n ->%s\n", dir->d_name, current_dir + size_of_rootDirName, readFile(nested_dir));
                
                sendModifyFilePetition(dir->d_name, readFile(nested_dir), current_dir + size_of_rootDirName);

            pthread_mutex_unlock(&serverRequestLock);
            }
        }
        closedir(d);
    }

    pthread_mutex_lock(&lock);
    //check if sent to die
    pthread_mutex_unlock(&lock);

    //monitorLoop
    while (1)
    {
        syslog(LOG_NOTICE, "MONITORING %s again =====================================================================================\n", current_dir);
        tail = inotifyMonitor(current_dir, head, tail);
    }

    pthread_exit(NULL);
}

//sets the monitor root directory and starts the initial monitoring thread to such directory
//that monitoring thread will recursively monitor all its sub directories.
int monitor(char *rootDir)
{
    size_of_rootDirName = strlen(rootDir);
    ROOTDIR = rootDir;

    DIR *dir = opendir(rootDir);

    //Verify existence of directory
    if (dir)
    {
        closedir(dir);
    }
    else if (ENOENT == errno)
    { //Directory does not exist.
        syslog(LOG_NOTICE, "\nDirectory %s, not found. ERROR: %d\n", rootDir, errno);
    }
    else
    { //opendir() failed for some other reason.
        syslog(LOG_NOTICE, "\nERROR: Directory %s, not found\n", rootDir);
    }

    //iniitalize mutex locks
    if (pthread_mutex_init(&lock, NULL) != 0 || pthread_mutex_init(&list_lock, NULL) != 0 || pthread_mutex_init(&serverRequestLock, NULL) != 0 )
    {
        syslog(LOG_NOTICE, "\n mutex init failed\n");
        return 0;
    }

    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    syslog(LOG_NOTICE, "MAIN MONITOR THREAD WORKING FROM: %s\n", pwd);

    //copy the root dir path
    char *rootDir_copy = (char *)(malloc((strlen(rootDir) + 1) * sizeof(char)));
    sprintf(rootDir_copy, "%s", rootDir);

    syslog(LOG_NOTICE, "SET MONITOR ROOT: %s\n", rootDir_copy);

    pthread_t *newThread = (pthread_t *)malloc(sizeof(pthread_t));
    int err = pthread_create(newThread, NULL, &directoryMonitorThread, (void *)rootDir);

    if (err != 0)
    {
        syslog(LOG_NOTICE, "\ncan't create thread for monitor of root dir %s, ERROR:[%s]\n", rootDir, strerror(err));
        return 0;
    }

    /*sleep(2); // Leave time for initialisation
    pthread_kill(*newThread, SIGSEGV);*/

    //join thread until end of execution
    pthread_join(*newThread, NULL);
    syslog(LOG_NOTICE, "END OF JOIN\n");

    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&list_lock);

    return 1;
}

//main function for testing purposes