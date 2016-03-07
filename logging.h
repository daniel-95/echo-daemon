#ifndef LOGGING_H
#define LOGGING_H

extern void write_log(char * info);
extern void *logging_thread(void *log_queue);

#endif // LOGGING_H
