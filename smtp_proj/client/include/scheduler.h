#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

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

int run_client();
int get_out_mail_domains(char **domains);
int get_domains_diff(int new_domains_count, char **new_mail_domains, char **dif);
int process_output_mails();

char *get_domain_mx_server(char *domain_name);
struct sockaddr_in get_domain_server_info(char *domain_name);

int read_fd_line(int fd, char *line, int lim);
void get_suffix(char *buf);
void send_msg_to_server(int socket_fd, char *msg);
void send_data(char *data, int toRead, int socket_fd);
void check_server_response_code(char *buf);
void get_suffix(char* buf);

#endif // _SCHEDULER_H_