#ifndef QUEUE_H
#define QUEUE_H

struct node
{
    char *data;
    struct node *next;
    struct node *prev;
};

struct queue
{
    struct node *head;
    struct node *tail;
    pthread_mutex_t mutex;
};

extern struct queue * new_queue();
extern struct node * new_node(char *string);
extern void push(struct queue *q, struct node *n);
extern struct node * pop(struct queue *q);

extern int lock_queue(struct queue *q);
extern int unlock_queue(struct queue *q);

#endif // QUEUE_H
