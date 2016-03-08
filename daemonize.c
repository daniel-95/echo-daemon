#include "main.h"

void daemonize()
{
    unsigned int i;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        exit(-1);

    if ((pid = fork()) < 0) // forking error
        exit(-2);
    else if (pid != 0)      // parent
        exit(0);

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
        exit(-3);

    //forking from fork
    if ((pid = fork()) < 0)
        exit(-2);
    else if (pid != 0)
        exit(0);

    if (chdir("/") < 0)
        exit(-3);

    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for (i = 0; i < rl.rlim_max; i++)
        close(i);

    open("/dev/null", O_RDWR);
    dup(0);
    dup(0);
}
