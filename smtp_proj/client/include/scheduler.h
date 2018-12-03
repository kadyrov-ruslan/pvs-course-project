#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "../../common/include/map.h"
#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "../include/mail_domain.h"
#include "../include/mx_utils.h"
#include "../include/smtp_client.h"
#include "../include/client_types.h"
#include "../include/msg.h"
#include <sys/select.h>
#include <sys/wait.h>

#define MAX_FD_CNT 1024

int run_client();
int get_out_mail_domains(char **domains);
int get_domains_diff(int new_domains_count, char **new_mail_domains, char **dif);
int process_output_mails();

#endif // _SCHEDULER_H_