#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct
{
    long mtype;
    char mtext[1024];
} queue_msg_t;

#endif // _QUEUE_H_