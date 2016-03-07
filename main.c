#include "main.h"

int main(int argc, char *argv[])
{
    int status;
    int sock;
    int int_port = 1234;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    char port[6];
    sprintf(port, "%d", int_port);

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

    char *buf = NULL;
    int N = 256;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = 0;
    int client;
    int flags;
    int read_bytes = 0;

    while(true)
    {
        client = accept(sock, (struct sockaddr*)&client_addr, &addr_size);
        printf("new connection\n");

        if(client == -1)
        {
            printf("Accepting error: %d\n", errno);
            break;
        }

        if(buf != NULL)
            free(buf);

        buf = malloc(N*sizeof(char));
        strcpy(buf, (char*)"");

        char chunk[256];
        flags = fcntl(client, F_GETFL, 0);
        fcntl(client, F_SETFL, flags | O_NONBLOCK);

        while(1)
        {
            read_bytes = recv(client, chunk, 256, 0);
            if(read_bytes == -1)
            {
                if(errno == EAGAIN)
                {
                    if(strlen(buf) > 0)
                    {
                        printf("%s\n", buf);
                        break;
                    }
                }
                else if(errno == EWOULDBLOCK)
                    printf("EWOULDBLOCK");
            }
            else
            {
                if(strlen(buf) + strlen(chunk) >= N)
                {
                    N *= 2;
                    buf = realloc(buf, N);
                }

                strcat(buf, chunk);
            }

            memset(chunk, 0, 256);
        }
    }

    return 0;
}
