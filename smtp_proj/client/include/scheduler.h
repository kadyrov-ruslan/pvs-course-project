#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/mx_utils.h"
#include "../include/smtp_client.h"
#include "../include/client_types.h"
#include "../include/msg.h"
#include "../include/map.h"
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <time.h>

#define PROC_NUM 2
#define MAX_MAIL_DOMAIN_NUM 50

int run_client();
int master_process_worker_start(struct mail_process_dscrptr *mail_procs);
int child_process_worker_start(int proc_idx);

int get_domains_mails(struct domain_mails *domains_mails, int domains_count);
int register_new_email(char *email_path, struct mail_domain_dscrptr *mail_domains_dscrptrs,
    fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);

void process_mail_domain(int maxfd, struct mail_domain_dscrptr *cur_mail_domain,
    fd_set *read_fds, fd_set *write_fds, fd_set *except_fds);   

void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);
void handle_read_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);  

void update_mail_state(int response_code, mail_process_state new_state,
                       struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);

void wait_for (unsigned int secs);
void shutdown_properly(int code);

#endif