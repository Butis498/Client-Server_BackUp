#include "fileManager.c"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

void monitor(char *dirName)
{
    int length, i = 0;
    int fd;
    int wd;
    char buffer[BUF_LEN];
    char *current_dir = (char *)(malloc(strlen(dirName) * sizeof(char)));
    sprintf(current_dir, "%s", dirName);

    fd = inotify_init();

    if (fd < 0)
    {
        perror("inotify_init");
    }

    wd = inotify_add_watch(fd, current_dir,
                           IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVE | IN_MOVED_TO);
    length = read(fd, buffer, BUF_LEN);

    if (length < 0)
    {
        perror("read");
    }

    while (i < length)
    {
        struct inotify_event *event =
            (struct inotify_event *)&buffer[i];
        if (event->len)
        {
            if (event->mask & IN_CREATE)
            {
                if (event->mask & IN_ISDIR)
                {

                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The directory %s was created.\n", new_dir);

                    monitor(new_dir);
                }
                else
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The file %s was created.\n", new_dir);
                }
            }
            else if (event->mask & IN_DELETE)
            {
                if (event->mask & IN_ISDIR)
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The directory %s was deleted.\n", new_dir);
                }
                else
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The file %s was deleted.\n", new_dir);
                }
            }
            else if (event->mask & IN_MODIFY)
            {
                if (event->mask & IN_ISDIR)
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The directory %s was modified.\n", new_dir);
                    monitor(new_dir);
                    
                }
                else
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The file %s was modified.\n", new_dir);
                }
            }
            else if (event->mask & IN_MOVED_TO)
            {
                if (event->mask & IN_ISDIR)
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The directory %s was moved.\n", new_dir);
                }
                else
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The file %s was moved.\n", new_dir);
                }
            }
            else if (event->mask & IN_MOVED_FROM)
            {
                if (event->mask & IN_ISDIR)
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The directory %s was moved out.\n", new_dir);
                }
                else
                {
                    char *new_dir = (char *)malloc((strlen(event->name) + strlen(current_dir)) * sizeof(char));
                    sprintf(new_dir, "%s/%s", current_dir, event->name);
                    syslog(LOG_NOTICE, "The file %s was moved out.\n", new_dir);
                }
            }
        }
        i += EVENT_SIZE + event->len;
    }

    (void)inotify_rm_watch(fd, wd);
    (void)close(fd);
}