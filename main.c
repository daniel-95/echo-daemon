#include "main.h"
#define BUFSIZE 32

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


int main(int argc, char *argv[])
{
    int status, cur_size;
    int i, j;
    int flags;
    int sock;
    pthread_t log_thread;
    struct queue *q;
    int int_port = 1234;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    char port[6];

    struct pollfd fds[200];
    int nfds = 1;

    sprintf(port, "%d", int_port);

    q = new_queue();
    pthread_create(&log_thread, NULL, logging_thread, (void*)q);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        printf("getaddrinfo() error");
        exit(1);
    }

    for(p = servinfo; p->ai_next != NULL; p = p->ai_next)
    {
        if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
        {
            perror("socket");
            continue;
        }

        int yes = 1;

        if (setsockopt(sock ,SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if(bind(sock, p->ai_addr, p->ai_addrlen) < 0)
        {
        //    close(sock);
            perror("bind");
            continue;
        }

        flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

    if(p == NULL)
    {
        printf("Failed to bind");
        return -1;
    }

    freeaddrinfo(servinfo);

    if(listen(sock, SOMAXCONN) < 0)
    {
        perror("listen");
        return -1;
    }

    memset(fds, 0, sizeof(fds));

    fds[0].fd = sock;
    fds[0].events = POLLIN;

    char *buf = NULL;
    int N = BUFSIZE;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = 0;
    int client;
    int read_bytes = 0;

    while(true)
    {
        status = poll(fds, nfds, -1);
        if(status < 0)
        {
            perror("poll error\n");
            break;
        }

        cur_size = nfds;
        for(i = 0; i < cur_size; i++)
        {

            if(fds[i].revents == 0)
                continue;

            if(fds[i].fd == sock)
            {
                do
                {
                    client = accept(sock, (struct sockaddr*)&client_addr, &addr_size);

                    if(client != -1)
                    {
                        printf("new connection\n");

                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN | POLLOUT;
                        nfds++;
                    }
                    else if(errno != EAGAIN)
                    {
                        printf("Accepting error: %d\n", errno);
                        break;
                    }
                }
                while(client != -1);
            }
            else
            {

                if(buf != NULL)
                    free(buf);

                buf = malloc(N*sizeof(char));
                strcpy(buf, (char*)"");

                char chunk[BUFSIZE];
                flags = fcntl(fds[i].fd, F_GETFL, 0);
                fcntl(fds[i].fd, F_SETFL, flags | O_NONBLOCK);

                while(1)
                {
                    read_bytes = recv(fds[i].fd, chunk, BUFSIZE-1, 0);
                    if(read_bytes == -1)
                    {
                        if(errno == EAGAIN)
                        {
                            if(strlen(buf) > 0)
                            {
                                struct node *n = new_node(buf);
                                lock_queue(q);
                                push(q, n);
                                unlock_queue(q);

                                printf("Sending back...\n");

                                send(fds[i].fd, buf, strlen(buf), MSG_DONTWAIT);

                                close(fds[i].fd);
                                fds[i].fd = -1; //shutdown?
                                break;
                            }
                        }
                        else if(errno == EWOULDBLOCK)
                            printf("EWOULDBLOCK");
                    }
                    else
                    {
                        chunk[read_bytes] = '\0';
                        if(strlen(buf) + strlen(chunk) >= N)
                        {
                            N *= 2;
                            buf = realloc(buf, N);
                        }

                        strcat(buf, chunk);
                    }

                    memset(chunk, 0, BUFSIZE);
               }
            }
        }

        for(i = 0; i < nfds; i++)
        {
            if(fds[i].fd == -1)
            {
                for(j = i; j < nfds-1; j++)
                    fds[i].fd = fds[i+1].fd;
            }
        }
    }

    return 0;
}
