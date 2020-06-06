#include"fileManager.c"

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

void createFile(){
     FILE * fPtr;

    fPtr = fopen("file1.txt", "w");


    if(fPtr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }


}

void monitor() {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, "MonitoredFolder",
        IN_MODIFY | IN_CREATE | IN_DELETE);
    length = read(fd, buffer, BUF_LEN);


    if (length < 0) {
        perror("read");
    }

    while (i < length) {
        struct inotify_event *event =
            (struct inotify_event *) &buffer[i];
        if (event->len) {
            if (event->mask & IN_CREATE) {
                syslog (LOG_NOTICE,"The file %s was created.\n", event->name);
            } else if (event->mask & IN_DELETE) {
                syslog (LOG_NOTICE,"The file %s was deleted.\n", event->name);
            } else if (event->mask & IN_MODIFY) {
                syslog (LOG_NOTICE,"The file %s was modified.\n", event->name);
            }
        }
        i += EVENT_SIZE + event->len;
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);

}