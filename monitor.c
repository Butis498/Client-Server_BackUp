#include "fileManager.c"

#include <dirent.h>
#include <pthread.h>

#define EVENT_SIZE (sizeof(struct inotify_event)) //Size of the inotify_event structure that describes a watched filesystem event
#define BUF_LEN (1024 * (EVENT_SIZE + 16)) 

//structure to store a monitored directory
typedef struct monitoredDir
{
    char* path;
    pthread_t* thread;
    int alive; //set to 0, to kill the tread, and stop monitoring
}monitoredDir;

//Node for a linked list of monitored directories
typedef struct Node { 
    monitoredDir data; 
    struct Node* next; 
} Node;


pthread_mutex_t lock;
pthread_mutex_t list_lock;

void inotifyMonitor(char* current_dir, Node* head){
    
    int length, i = 0;
    int fd; //file descriptor
    int wd; //watch descriptor
    char buffer[BUF_LEN];
   
    /*char *current_dir = (char *)(malloc(strlen(dirName) * sizeof(char)));
    sprintf(current_dir, "%s", dirName);*/

    //initializes a new inotify instance and returns a file descriptor
    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
    }

    //adds a new watch for the file whose location is specified in current_dir
    //returns a unique watch descriptor for this inotify instance
    wd = inotify_add_watch(fd, current_dir,
                           IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVE | IN_MOVED_TO);//Bit mask of the events to be monitored
    
    length = read(fd, buffer, BUF_LEN);
    if (length < 0) {
        perror("read");
    }

    //for every item in the buffer as read from the fd file descriptor
    while (i < length)
    {
        //Cast the item wich beggins at position 'i' in the buffer as an inotify_event
        struct inotify_event *event = (struct inotify_event *)&buffer[i];

        if (event->len) {

            //new_dir is the path to the affected file inside the watched directory
            char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir) + 2) * sizeof(char));
            sprintf(new_dir, "%s/%s", current_dir, event->name);

            printf("Modified path: %s \n", new_dir);

            //Check the mask of the event, and behave acordingly (the mask describes the reason of the event)
            if (event->mask & IN_CREATE) { //file/directory created in the watched directory
            
                if (event->mask & IN_ISDIR) { //if the subject of this event is a directory
                    syslog(LOG_NOTICE, "The directory %s was created.\n", new_dir);
                    //monitor(new_dir);
                }
                else { //if not, then it was a file
                    syslog(LOG_NOTICE, "The file %s was created.\n", new_dir);
                }
            } 
            else if (event->mask & IN_DELETE) { // file/directory deleted from watched directory

                if (event->mask & IN_ISDIR) {
                    syslog(LOG_NOTICE, "The directory %s was deleted.\n", new_dir);
                }
                else {
                    syslog(LOG_NOTICE, "The file %s was deleted.\n", new_dir);
                }
            } 
            else if (event->mask & IN_MODIFY) { //File was modified (ONLY FOR FILES     )
                if (event->mask & IN_ISDIR) {                 
                    syslog(LOG_NOTICE, "The directory %s was modified.\n", new_dir);
                    //monitor(new_dir);
                }
                else {                
                    syslog(LOG_NOTICE, "The file %s was modified.\n", new_dir);
                }
            }
            else if (event->mask & IN_MOVED_TO) { //Generated for the directory containing the new filename when a file is renamed

                if (event->mask & IN_ISDIR) {                
                    syslog(LOG_NOTICE, "The directory %s was moved.\n", new_dir);
                }
                else {                
                    syslog(LOG_NOTICE, "The file %s was moved.\n", new_dir);
                }
            }
            else if (event->mask & IN_MOVED_FROM) { //Generated for the directory containing the old filename when a file is renamed

                if (event->mask & IN_ISDIR) {
                    syslog(LOG_NOTICE, "The directory %s was moved out.\n", new_dir);
                }
                else {
                    syslog(LOG_NOTICE, "The file %s was moved out.\n", new_dir);
                }
            }
        }

        //move the index to the next item according to the EVENT_SIZE and the current event lenght
        i += EVENT_SIZE + event->len;
    }

    //remove the watch wd from the fd 
    (void)inotify_rm_watch(fd, wd);
    //close the inotify event file descritor
    (void)close(fd);
}

//Monitor thread
//
void* directoryMonitorThread(void *dirName){
    //Local linked list of nested monitoredDir for this directory thread
    Node* head; //monitoredDir linked list head
    Node* tail; //used to push to the end of the linked list

    head = (Node*)malloc(sizeof(Node));
    head->next = NULL;
    tail = head;

    //create copy of the directory name
    char *current_dir = (char *)(malloc(strlen((char*)dirName) * sizeof(char)));
    sprintf(current_dir, "%s", (char*)dirName);

    printf("Nested dirs of: %s\n", current_dir);
    //Get contents of the directory
    DIR *d;
    struct dirent *dir;
    d = opendir(current_dir);

    if (d) {
        while ((dir = readdir(d)) != NULL) //read all the contents of the directory
        {
            //execute only on nested directories, (d_type == 4 for a drectory, 8 for a file)
            if(dir->d_type == 4 && dir->d_name[0] != '.' ){

                //copy of the new nested dir name concatenated to the current dir
                char *nested_dir = (char *)(malloc((strlen(dir->d_name) + strlen(current_dir) + 2) * sizeof(char)));
                sprintf(nested_dir, "%s/%s", current_dir, dir->d_name);

                printf("\t%s\n", nested_dir);
                
                //recursive call to directoryMonitorThread by creating another thread with it
                pthread_t* newThread = (pthread_t*)malloc(sizeof(pthread_t));
                int err = pthread_create(newThread, NULL, &directoryMonitorThread, (void*)nested_dir);
                
                if (err != 0){
                    printf("\ncan't create thread for monitor of dir %s, ERROR:[%s]\n", nested_dir, strerror(err));
                }
                else{//push the newly created monitor to the linked list
                    
                    monitoredDir newMonitor;

                    //fill the new monitr params, to push it to the list
                    newMonitor.path = nested_dir;
                    newMonitor.thread = newThread;
                    newMonitor.alive = 1;

                    pthread_mutex_lock(&list_lock);
                    //push the monitor to the list, and update the tail
                    tail->data = newMonitor;
                    tail->next = (Node*)malloc(sizeof(Node));
                    tail->next->next = NULL;

                    //printf("\tPath: %s, Alive = %d\n", tail->data.path, tail->data.alive);

                    tail = tail->next;
                    pthread_mutex_unlock(&list_lock);
                }
            }
        }
        //printf("HEAD:\tPath: %s, Alive = %d\n", head->data.path, head->data.alive);
        closedir(d);
    }

    pthread_mutex_lock(&lock);
    //check if sent to die
    pthread_mutex_unlock(&lock);

    //monitorLoop
    while (1){
        printf("Monitoring %s again ====================================================\n", current_dir);
        inotifyMonitor(current_dir, head);
    }

    //printf("End monitor of dir: %s\n", current_dir);

    pthread_exit(NULL);
}

//sets the monitor root directory and starts the initial monitoring thread to such directory
//that monitoring thread will recursively monitor all its sub directories.
int monitor(char* rootDir)
{   
    DIR* dir = opendir(rootDir);
    if (dir) {
        closedir(dir);
    } else if (ENOENT == errno) { //Directory does not exist.
        printf("\nDirectory %s, not found. ERROR: %d\n", rootDir, errno);
    } else { //opendir() failed for some other reason.
        printf("\nERROR: Directory %s, not found\n", rootDir);
    }

    //iniitalize mutex locks
    if (pthread_mutex_init(&lock, NULL) != 0 || pthread_mutex_init(&list_lock, NULL)){
        printf("\n mutex init failed\n");
        return 0;
    }
        
    //char* rootDir = "MonitoredFolder";
    //copy the root dir path
    char *rootDir_copy = (char *)(malloc((strlen(rootDir) + 1) * sizeof(char)));
    sprintf(rootDir_copy, "%s", rootDir);

    printf("ROOT: %s\n", rootDir_copy);    

    pthread_t* newThread = (pthread_t*)malloc(sizeof(pthread_t));
    int err = pthread_create(newThread, NULL, &directoryMonitorThread, (void*)rootDir);
    
    if (err != 0){
        printf("\ncan't create thread for monitor of root dir %s, ERROR:[%s]\n", rootDir, strerror(err));
        return 0;
    }
    
    //join thread until end of execution
    pthread_join(*newThread, NULL);
    printf("END OF JOIN\n");

    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&list_lock);

    return 1;
}

//main function for testing purposes
int main(){
    
    monitor("MonitoredFolder");

    /*while (1)
    {
        printf("Monitoring again ====================================================\n");
        monitor("MonitoredFolder");
        
    }*/
}