#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

void hello_world()
{
    printf("soon there will be a daemon\n");
}

void write_log(char * info)
{
    time_t raw_time;
    struct tm *cur_time;
    char *printable_time;

    time(&raw_time);
    cur_time = localtime(&raw_time);

    printable_time = (char *)asctime(cur_time);
    printable_time[strlen(printable_time) - 1] = '\0';

    FILE *fp;
    fp = fopen("./echo-daemon.log", "a");
    fprintf(fp, "[%s]: %s\n", printable_time, info);
    fclose(fp);
}

