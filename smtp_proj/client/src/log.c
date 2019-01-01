#include "../include/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

log_level cur_lvl;

int save_log(char *message)
{
    char filename[150];
    struct tm *timenow;

    time_t now = time(NULL);
    timenow = gmtime(&now);

    char timeString[12];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timenow);
    strftime(filename, sizeof(filename), "/home/dev/pvs-course-project/smtp_proj/client/logs/log_%Y-%m-%d", timenow);
    char *log_full_path = malloc(strlen(filename) + 4);
    strcpy(log_full_path, filename);
    strcat(log_full_path, ".log");

    if (access(log_full_path, F_OK) != -1)
    {
        FILE *fp = fopen(log_full_path, "ab");
        if (fp != NULL)
        {
            char *full_log_msg = malloc(strlen(timeString) + 2 + strlen(message));
            strcpy(full_log_msg, timeString);
            strcat(full_log_msg, " ");
            strcat(full_log_msg, message);
            fputs(full_log_msg, fp);
            fclose(fp);
        }
    }
    else
    {
        FILE *f;
        f = fopen(log_full_path, "a+");
        if (f != NULL)
        {
            char *full_log_msg = malloc(strlen(timeString) + 2 + strlen(message));
            strcpy(full_log_msg, timeString);
            strcat(full_log_msg, " ");
            strcat(full_log_msg, message);
            fputs(full_log_msg, f);
            fclose(f);
        }
    }

    return 1;
}

int start_logger(const char *log_filename_base)
{
    key_t key = ftok("/tmp", 65);
    int log_queue_id = msgget(key, 0644);
    struct queue_msg cur_msg;
    while (1)
    {
        if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
        {
            if (strlen(cur_msg.mtext) != 0)
                save_log(cur_msg.mtext);
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
    key_t key = ftok("/tmp", 65);
    int log_queue_id = msgget(key, 0666 | IPC_CREAT);
    struct queue_msg new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);
    msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);
    return 1;
}
