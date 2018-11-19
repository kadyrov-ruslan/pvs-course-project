#ifndef _SMTP_CLIENT_H_
#define _SMTP_CLIENT_H_

#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <assert.h>

#include "../../common/include/map.h"
#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/client_types.h"
#include "../include/msg.h"

#define MAX_BUF_LEN 1024

char s[3];    //string to store 's: '
char c[3];    //string to store 'c: '
char *suffix; //string to store server suffix

char buf[MAX_BUF_LEN];

char *buffer;                 //dynamically allocated char array to get messages from the server
int length;                   //current size of buffer
char myHostName[MAX_BUF_LEN]; //string to store my host name

void get_suffix(char *buf);
void send_msg_to_server(int socket_fd, char *msg);
void send_data(char *data, int toRead, int socket_fd);
void check_server_response_code(char *buf);

#endif