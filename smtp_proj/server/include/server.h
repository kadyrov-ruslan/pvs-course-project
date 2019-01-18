#ifndef _SERVER_H_
#define _SERVER_H_

typedef struct server_opts_t
{
  const char *ip;
  int port;
  const char *user;
  const char *group;
  int process_count;
} server_opts_t;

/* Обработка сигналов */
void handle_signal(int signal);

/* Установка ID пользователя и группы во время выполнения */
int set_id(const char *user, const char *group);

/* Текст сообщения о ошибке при установке uid | gid */
char *set_id_err(int code);

#endif // _SERVER_H_
