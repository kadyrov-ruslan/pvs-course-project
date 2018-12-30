#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

log_level cur_lvl;

int start_logger(const char *log_filename_base)
{
    int log_proc = fork();
    if (log_proc == 0)
    {
        key_t key = ftok("/tmp", 0);
        int log_queue_id = msgget(key, 0644);
        struct queue_msg cur_msg;
        while (1)
        {
            if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
            {
                if (strlen(cur_msg.mtext) != 0)
                {
                    //write msg to file
                    printf("maxfd .%d\n", maxfd);
                }
            }
        }
    }
    return 0;
}

int stop_logger(void)
{
    int result = 0;
    return result;
}

int send_log_message(log_level log_lvl, char *message)
{
    int result = 0;
    key_t key = ftok("/tmp", 0);
    int log_queue_id = msgget(key, 0644);
    struct queue_msg new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);
    msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);

    return result;
}
