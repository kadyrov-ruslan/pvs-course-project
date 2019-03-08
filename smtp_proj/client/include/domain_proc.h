#ifndef _DOMAIN_PROC_H_
#define _DOMAIN_PROC_H_

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "mx_utils.h"
#include "smtp_client.h"
#include "scheduler.h"
#include "msg.h"
#include "map.h"
#include "client-fsm.h"
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <time.h>
#include <sys/time.h>

int get_domains_mails(struct domain_mails *domains_mails, int domains_count);

int register_new_email(char *email_path, struct mail_domain_dscrptr *mail_domains_dscrptrs,
                       fd_set *read_fds, fd_set *write_fds, fd_set *except_fds, int total_send_time, int retry_time, int old_ready_domains_count);

void init_mail_domain_conn_settings(struct mail_domain_dscrptr *mail_domains_dscrptrs, int cur_domain_idx,
                      char *cur_email_domain, fd_set *read_fds, fd_set *except_fds);

void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);

void handle_read_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds);

#endif