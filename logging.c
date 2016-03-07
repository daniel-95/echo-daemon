#include "main.h"

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

void *logging_thread(void *log_queue)
{
    struct node *n;
    struct queue *q = (struct queue*)log_queue;

    for(;;)
    {
        lock_queue(q);
        n = pop(q);

        if(n != NULL)
        {
            write_log(n->data);
            free(n->data);
            free(n);
        }

        unlock_queue(q);
    }

    return (void*)0;
}

