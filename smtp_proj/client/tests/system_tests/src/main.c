#include "../../../include/smtp_client.h"
#include "../../../include/msg.h"
#include "../../../include/map.h"
#include "../../../include/mx_utils.h"
#include "../../../include/log.h"
#include "../../../include/list.h"

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <resolv.h>

int main(void)
{
    char *mail_path = "/home/dev/pvs-course-project/smtp_proj/client/tests/system_tests/mail/user1/new/1.1.localhost.com,S=41.mbox";
    FILE *fp = fopen(mail_path, "a+");
    if (fp != NULL)
    {
        fputs("somemail@gmail.com\n", fp);
        fputs("dev@localhost.com\n", fp);
        fputs("From: somemail@gmail.com\n", fp);
        fputs("TO: dev@localhost.com\n", fp);
        fputs("Subject:test mail for cource SMTP client\n\n\n", fp);
        fputs("This is test EMAIL for check working SMTP client\n", fp);
    }
    fclose(fp);
}