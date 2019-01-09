#include "../../include/smtp_client.h"
#include "../../include/msg.h"
#include "../../include/map.h"
#include "../../include/mx_utils.h"
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
#include <time.h>
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

    struct sockaddr_in mail_server = get_domain_server_info("localhost");
    mail_server.sin_family = AF_INET;
    mail_server.sin_port = htons(25);

    int socket_fd = 0;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Could not create socket");
        return -1;
    }

    //printf("Socket fd : %d", socket_fd);
    if (connect(socket_fd, (struct sockaddr *)&mail_server, sizeof(mail_server)) < 0)
    {
        printf("Connection Failed");
        return -1;
    }
    else
    {
        mock_dscrptrs[0].socket_fd = socket_fd;                
        fcntl(socket_fd, F_SETFL, O_NONBLOCK);
        FD_SET(socket_fd, &read_fds);
        FD_SET(socket_fd, &write_fds);
        FD_SET(socket_fd, &except_fds);
    }

    mock_dscrptrs[0].mails_list = malloc(sizeof(node_t));
    mock_dscrptrs[0].mails_list->next = NULL;

    add_first(&mock_dscrptrs[0].mails_list, "/home/dev/mail/user1/new/1.1.localhost.com,S=41.mbox");
    return 0; 
}

int clean_suite(void) { return 0; }

void ONE_EMAIL_SENT_SUCCESS(void)
{
    int response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
    
    send_helo(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    mock_dscrptrs[0].state = READY;
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
    
    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_headers(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = 0;
    while (response_code == 0)
        response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_quit(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
}

void TWO_EMAILS_ONE_SESSION_SENT_SUCCESS(void)
{
    init_suite();
    int response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
    
    /* First EMAIL */
    send_helo(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    mock_dscrptrs[0].state = READY;
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
    
    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_headers(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = 0;
    while (response_code == 0)
        response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);

    CU_ASSERT(response_code > 200 && response_code < 400);

    /* Second EMAIL */
    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    mock_dscrptrs[0].state = READY;
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
    
    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);

    send_headers(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = 0;
    while (response_code == 0)
        response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);

    CU_ASSERT(response_code > 200 && response_code < 400);

    send_quit(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    response_code = get_server_response_code(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].response_buf);
    CU_ASSERT(response_code > 200 && response_code < 400);
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
    if ((NULL == CU_add_test(pSuite, "ONE_EMAIL_SENT_SUCCESS", ONE_EMAIL_SENT_SUCCESS)) ||
        (NULL == CU_add_test(pSuite, "TWO_EMAILS_ONE_SESSION_SENT_SUCCESS", TWO_EMAILS_ONE_SESSION_SENT_SUCCESS)))
        // (NULL == CU_add_test(pSuite, "RCPT_TO_SENT_SUCCESSFULLY", RCPT_TO_SENT_SUCCESS))||
        // (NULL == CU_add_test(pSuite, "DATA_MSG_SENT_SUCCESSFULLY", DATA_MSG_SENT_SUCCESS))||
        // (NULL == CU_add_test(pSuite, "MAIL_BODY_SENT_SUCCESSFULLY", MAIL_BODY_SENT_SUCCESS)) ||
        // (NULL == CU_add_test(pSuite, "QUIT_SENT_SUCCESS", QUIT_SENT_SUCCESS)))
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

    /* Clean up registry and return */
    CU_cleanup_registry();
    return CU_get_error();
}
