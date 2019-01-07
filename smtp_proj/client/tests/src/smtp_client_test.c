#include "../../include/smtp_client.h"
#include "../../include/msg.h"
#include "../../include/map.h"
#include "../../include/log.h"
#include "../../include/list.h"

#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>

fd_set read_fds;
fd_set write_fds;
fd_set except_fds;

struct mail_domain_dscrptr mock_dscrptrs[1];

/* Test Suite setup and cleanup functions: */
int init_suite(void) 
{ 
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    struct sockaddr_in mail_server;
    bzero(&mail_server, sizeof(mail_server));
    mail_server.sin_family = AF_INET;
    mail_server.sin_port = htons(55);
    mail_server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int socket_fd = 0;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Could not create socket");
        return -1;
    }

    printf("Socket fd : %d", socket_fd);
    if (connect(socket_fd, (struct sockaddr *)&mail_server, sizeof(mail_server)) < 0)
    {
        printf("Connection Failed");
        return -1;
    }
    else
    {
        fcntl(socket_fd, F_SETFL, O_NONBLOCK);
        mock_dscrptrs[0].socket_fd = socket_fd;

        //FD_SET(cur_domain_socket_fd, read_fds);
        FD_SET(socket_fd, &write_fds);
        FD_SET(socket_fd, &except_fds);
    }

    mock_dscrptrs[0].mails_list = malloc(sizeof(node_t));
    mock_dscrptrs[0].mails_list->next = NULL;

    add_first(&mock_dscrptrs[0].mails_list, "/home/dev/mail/user1/new/1.1.mail.ru,S=41.mbox");
    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    mock_dscrptrs[0].state = READY;

    //register_new_email("/home/dev/mail/user1/new/1.1.mail.ru,S=41.mbox", mock_dscrptrs, &read_fds, &write_fds, &except_fds);

    return 0; 
}

int clean_suite(void) { return 0; }

void test_case_sample(void)
{
    CU_ASSERT(CU_TRUE);
    CU_ASSERT_NOT_EQUAL(2, -1);
    CU_ASSERT_STRING_EQUAL("string #1", "string #1");
    CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #2");

    CU_ASSERT(CU_FALSE);
    CU_ASSERT_EQUAL(2, 3);
    CU_ASSERT_STRING_NOT_EQUAL("string #1", "string #1");
    CU_ASSERT_STRING_EQUAL("string #1", "string #2");
}

void HELO_SENT_SUCCESS(void)
{
    FD_SET(mock_dscrptrs[0].socket_fd, &read_fds);
    send_helo(mock_dscrptrs[0].socket_fd);
    FD_CLR(mock_dscrptrs[0].socket_fd, &write_fds);
    FD_SET(mock_dscrptrs[0].socket_fd, &read_fds);
    int response_code = get_server_response_code(mock_dscrptrs[0].socket_fd);
    CU_ASSERT(response_code > 200 || response_code < 400);
}

void MAIL_FROM_SENT_SUCCESS(void)
{
    FD_SET(mock_dscrptrs[0].socket_fd, &read_fds);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer);
    FD_CLR(mock_dscrptrs[0].socket_fd, &write_fds);
    FD_SET(mock_dscrptrs[0].socket_fd, &read_fds);
    int response_code = get_server_response_code(mock_dscrptrs[0].socket_fd);
    CU_ASSERT(response_code > 200 || response_code < 400);
}

void HELO_RCPT_TO_SENT_SUCCESS(void)
{
    CU_ASSERT_EQUAL(-1, -1);
}

int main(void)
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("smtp_client_test_suite", init_suite, clean_suite);
    if (NULL == pSuite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "HELO_SENT_SUCCESS", HELO_SENT_SUCCESS))||
        (NULL == CU_add_test(pSuite, "MAIL_FROM_SENT_SUCCESS", MAIL_FROM_SENT_SUCCESS)))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    printf("\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n\n");
    /*
   // Run all tests using the automated interface
   CU_automated_run_tests();
   CU_list_tests_to_file();

   // Run all tests using the console interface
   CU_console_run_tests();
*/
    /* Clean up registry and return */
    CU_cleanup_registry();
    return CU_get_error();
}
