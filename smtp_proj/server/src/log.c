#include "log.h"

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "queue.h"

#define PROJ_ID 16
#define FD_QUEUE_PATH "/tmp"

key_t key;
int qid, fd_log = 0;

int save_message(const char *message, ssize_t size);

int logger_start(log_options_t *opts)
{
  printf("Log path: %s\n", opts->path);

  if ((access(FD_QUEUE_PATH, F_OK)) < 0)
    return logger_stop();
  if ((fd_log = open(opts->path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
    return logger_stop();
  if ((key = ftok(FD_QUEUE_PATH, PROJ_ID)) < 0)
    return logger_stop();
  if ((qid = msgget(key, 0644 | IPC_CREAT)) < 0)
    return logger_stop();

  ssize_t message_size;
  queue_msg_t queue_msg;
  while (1)
  {
    if (msgrcv(qid, &queue_msg, sizeof(queue_msg), 1, MSG_NOERROR) < 0)
      return logger_stop();

    message_size = strlen(queue_msg.mtext);
    if (save_message(queue_msg.mtext, message_size) < 0)
      return logger_stop();
  }

  return 0;
}

int logger_stop(void)
{
  if (errno != 0)
    fprintf(stderr, "%s\n", strerror(errno));
  if (fd_log > 0)
    close(fd_log);
  printf("%s\n", "Log process stopped");
  return 0;
}

int log_message(log_level level, const char *message)
{
  if ((key = ftok(FD_QUEUE_PATH, PROJ_ID)) < 0)
    return -1;
  if ((qid = msgget(key, 0644 | IPC_CREAT)) < 0)
    return -1;
  
  queue_msg_t queue_msg;
  queue_msg.mtype = 1;
  strcpy(queue_msg.mtext, message);
  if (msgsnd(qid, &queue_msg, sizeof(queue_msg), IPC_NOWAIT) < 0)
    return -1;

  return 0;
}

int save_message(const char *message, ssize_t size)
{
  if (fd_log <= 0 || size < 0)
    return -1;
  if (size == 0)
    return 0;

  struct tm *now;
  char timestr[20];
  time_t timenow = time(NULL);
  now = localtime(&timenow);
  sprintf(timestr, "%d-%02d-%02dT%02d:%02d:%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  const int log_msg_size = 20 + 1 + size;
  char *log_msg = malloc(log_msg_size);
  strcpy(log_msg, timestr);
  strcat(log_msg, " ");
  strcat(log_msg, message);

  write(fd_log, log_msg, log_msg_size);
  free(log_msg);

  return 0;
}