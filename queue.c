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
    pthread_mutex_t *mutex;
};

struct queue * new_queue()
{
    struct queue *q = malloc(sizeof(struct queue));
    q->head = NULL;
    q->tail = NULL;

    pthread_mutex_init(q->mutex, NULL);

    return q;
}

struct node * new_node(char *string)
{
    struct node *n = malloc(sizeof(struct node));
    n->next = n->prev = NULL;
    n->data = malloc(strlen(string) * sizeof(char));
    strcpy(n->data, string);

    return n;
}

void push(struct queue *q, struct node *n)
{
    if(q->head == NULL)
    {
        q->head = n;
        q->tail = n;
    }
    else
    {
        n->next = q->head;
        q->head->prev = n;
        q->head = n;
    }
}

struct node * pop(struct queue *q)
{
    if(q->tail == NULL)
        return NULL;
    else
    {
        struct node *n = q->tail;
        q->tail = q->tail->prev;

        if(q->tail != NULL)
            q->tail->next = NULL;
        else
            q->head = NULL;

        return n;
    }
}

int lock_queue(struct queue *q)
{
    return pthread_mutex_lock(q->mutex);
}

int unlock_queue(struct queue *q)
{
    return pthread_mutex_unlock(q->mutex);
}
