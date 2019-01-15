#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/mx_utils.h"
#include "../include/smtp_client.h"
#include "../include/client_types.h"
#include "../include/msg.h"
#include "../include/map.h"
#include "../include/client-fsm.h"
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <time.h>

#define MAX_MAIL_DOMAIN_NUM 50
#define RETRY_DIR_READ_TIME 25

int run_client(int proc_num);
void create_child_proc(int idx, int proc_num);

int master_process_worker_start(struct mail_process_dscrptr *mail_procs, int proc_num);
int child_process_worker_start(int proc_idx);

int get_mail_proc_idx(char *domain_name, int domains_count, struct mail_process_dscrptr *mail_procs);

int get_domains_mails(struct domain_mails *domains_mails, int domains_count);
int register_new_email(char *email_path, struct mail_domain_dscrptr *mail_domains_dscrptrs,
    fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);

void process_mail_domain(int maxfd, struct mail_domain_dscrptr *cur_mail_domain,
    fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);   

void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);
void handle_read_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);  

void wait_for (unsigned int secs);
void shutdown_properly(int code);

#endif