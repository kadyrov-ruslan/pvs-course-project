#ifndef _SERVER_H_
#define _SERVER_H_

typedef struct server_options_t
{
  int port;
  const char *ip;
  const char *user;
  const char *group;
  int process_count;
} server_options_t;

#endif // _SERVER_H_
