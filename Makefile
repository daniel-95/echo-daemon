all:
	gcc main.c queue.c logging.c daemonize.c -pthread -Wall -Werror -Wextra -o echo-daemon
