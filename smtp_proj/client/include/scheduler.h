#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/mx_utils.h"
#include "../include/smtp_client.h"
#include "../include/domain_proc.h"
#include "client_types.h"
#include "msg.h"
#include "map.h"
#include "client-fsm.h"
#include <sys/select.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <time.h>
#include <sys/time.h>

#define MAX_MAIL_DOMAIN_NUM 50
#define RETRY_DIR_READ_TIME 25

int run_client(int proc_num, int total_send_time, int retry_time);
void create_child_proc(int proc_num, int total_send_time, int retry_time);

int master_process_worker_start(struct mail_process_dscrptr *mail_procs, int proc_num);
int child_process_worker_start(int proc_idx, int total_send_time, int retry_time);
int get_mail_proc_idx(char *domain_name, int domains_count, struct mail_process_dscrptr *mail_procs);

void wait_for(unsigned int secs);
void shutdown_properly(int code);

#endif