#include "../include/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

log_level cur_lvl;

int save_log(char *message, char *logs_dir)
{
    struct tm *timenow;

    time_t now = time(NULL);
    timenow = gmtime(&now);

    char timeString[12];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", timenow);

    char *log_path = malloc(strlen(logs_dir) + 50);
    strcpy(log_path, logs_dir);

    char filename[40];
    strftime(filename, sizeof(filename), "log_%Y-%m-%d", timenow);
    strcat(log_path, filename);
    strcat(log_path, ".log");

    FILE *fp;
    if (access(log_path, F_OK) != -1)
        fp = fopen(log_path, "ab");
    else
        fp = fopen(log_path, "a+");

    if (fp != NULL)
    {
        char *full_log_msg = malloc(strlen(timeString) + 2 + strlen(message));
        strcpy(full_log_msg, timeString);
        strcat(full_log_msg, " ");
        strcat(full_log_msg, message);
        fputs(full_log_msg, fp);
        free(full_log_msg);
        fclose(fp);
    }
    free(log_path);
    return 1;
}

int start_logger(char *log_filename_base)
{
    printf("Log path : %s \n", log_filename_base);
    key_t key = ftok("/tmp", 33);
    int log_queue_id = msgget(key, 0644);
    struct queue_msg cur_msg;
    while (1)
    {
        if (msgrcv(log_queue_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
        {
            if (strlen(cur_msg.mtext) != 0)
                save_log(cur_msg.mtext, log_filename_base);
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
    key_t key = ftok("/tmp", 33);
    int log_queue_id = msgget(key, 0666 | IPC_CREAT);
    struct queue_msg new_msg;
    new_msg.mtype = 1;
    strcpy(new_msg.mtext, message);
    msgsnd(log_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);
    return 1;
}
