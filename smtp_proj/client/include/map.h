#ifndef _MAP_H_
#define _MAP_H_

#include <resolv.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "list.h"
#include "client-fsm.h"

#define MAX_BUF_LEN 1024

//Описывает один почтовый домен
//Сетевая информация, сокет и число писем
struct mail_domain_dscrptr
{
    char *domain;
    struct sockaddr_in domain_mail_server;
    int socket_fd;
    int mails_count;
    struct node_t *mails_list;
    te_client_fsm_state state;
    char *buffer;  // Буфер для хранения прочитанного письма

    int retry_time;
    int total_send_time;

    int number_of_attempts;
    int last_attempt_time;
    int can_be_send;
    int curr_rcpts_index;

    char request_buf[MAX_BUF_LEN];   // Буфер для отправляемых данных
    //char response_buf[MAX_BUF_LEN];  // Буфер для ответа от сервера
};

struct mail_process_dscrptr
{
    pid_t pid;         // pid дочернего процесса
    int msg_queue_id;  // id очереди сообщений, из которой процесс получает инфо о письмах
    int domains_count; // число доменов, обрабатываемых процессом
    int mails_count;   // число писем, обрабатываемых процессом
    char *domains[60]; // названия обрабатываемых доменов
};

//Describes single domain mails, that have to be sent
struct domain_mails
{
    char *domain;
    char *mails_paths[100];
    int mails_count;
};

typedef struct queue_msg
{
    long mtype;
    char mtext[500];
} queue_msg;

#endif