#include "monitor.c"

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    /*TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("~/Documents/ProgramacionAvanzada/Mission6");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
    {
        close(x);
    }

    /* Open the log file */
    openlog("firstdaemon", LOG_PID, LOG_DAEMON);
}


int main(int argc, char const *argv[])
{
    //skeleton_daemon();

    if(argc == 2){
        PORT = atoi(argv[1]);
    }else if (argc >= 3){
        PORT = atoi(argv[1]);
        HOST = (char*)malloc((strlen(argv[2]) + 1 )* sizeof(char));
        strncpy(HOST, argv[2], strlen(argv[2]) + 1 );
    }

    syslog(LOG_NOTICE, "+++> SERVICE INITIATED: serviceBackUp/firstdaemon ==========================================================================================================================================================\n");

    printf("USING HOST: %s\n", HOST);
    syslog(LOG_NOTICE, "USING HOST: %s\n", HOST);
    printf("USING PORT: %d\n", PORT);
    syslog(LOG_NOTICE, "USING PORT: %d\n", PORT);

    signal(SIGSEGV,sig_func);
    monitor("MonitoredClientFolder");

    return EXIT_SUCCESS;
}
