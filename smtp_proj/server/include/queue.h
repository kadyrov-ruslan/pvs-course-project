#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct queue_msg_t
{
	long mtype;
	char mtext[255];
} queue_msg_t;

#endif // _QUEUE_H_