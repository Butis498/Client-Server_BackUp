#include "fileManager.c"
#include <dirent.h>

#define EVENT_SIZE (sizeof(struct inotify_event)) //Size of the inotify_event structure that describes a watched filesystem event
#define BUF_LEN (1024 * (EVENT_SIZE + 16)) 

//monitor changes inside a given directory
void monitor(char *dirName)
{
    int length, i = 0;
    int fd; //file descriptor
    int wd; //watch descriptor
    char buffer[BUF_LEN];
    char *current_dir = (char *)(malloc(strlen(dirName) * sizeof(char)));
    sprintf(current_dir, "%s", dirName);

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
            char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
            sprintf(new_dir, "%s/%s", current_dir, event->name);

            printf("Modified path: %s \n", new_dir);

            //Check the mask of the event, and behave acordingly (the mask describes the reason of the event)
            if (event->mask & IN_CREATE) { //file/directory created in the watched directory
            
                if (event->mask & IN_ISDIR) { //if the subject of this event is a directory
                    syslog(LOG_NOTICE, "The directory %s was created.\n", new_dir);
                    monitor(new_dir);
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
                    monitor(new_dir);
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

/*int main()
{
    while (1)
    {
        printf("Monitoring again ====================================================\n");
        monitor("MonitoredFolder");
        
    }

    //DIR *d;
    //struct dirent *dir;
    //d = opendir(".");
    //if (d)
    //{
    //    while ((dir = readdir(d)) != NULL)
    //    {
    //        printf("%s, %u\n", dir->d_name, dir->d_type);
    //    }
    //    closedir(d);
    //}
    

    return 0;
}*/