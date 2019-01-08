#ifndef _SMTP_CLIENT_H_
#define _SMTP_CLIENT_H_

#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <assert.h>

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/map.h"
#include "../include/log.h"
#include "../include/client_types.h"
#include "../include/msg.h"

#define MAX_BUF_LEN 1024
#define INITIAL_SIZE 10

char client_host_name[MAX_BUF_LEN]; //string to store my host name

int send_helo(int socket_fd, char *request_buf);
int send_mail_from(int socket_fd, char *msg, char *request_buf);
int send_rcpt_to(int socket_fd, char *msg, char *request_buf);
int send_data_msg(int socket_fd, char *request_buf);
int send_headers(int socket_fd, char *request_buf);
int send_msg_body(int socket_fd);
int send_quit(int socket_fd, char *request_buf);

void send_data(char *data, int to_read, int socket_fd);
int get_server_response_code(int socket_fd, char *response_buf);

int read_fd_line(int fd, char *line, int lim);
void send_body_to_server(int socket_fd, char *msg);

#endif