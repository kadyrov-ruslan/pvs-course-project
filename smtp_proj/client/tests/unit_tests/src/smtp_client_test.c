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
    mock_dscrptrs[0].socket_fd = connect_to_mail_server(0, mail_server, "localhost.com");
    mock_dscrptrs[0].domain_mail_server = mail_server;
    mock_dscrptrs[0].total_send_time = 120;
    mock_dscrptrs[0].retry_time = 15;
    mock_dscrptrs[0].mails_list = malloc(sizeof(node_t));
    mock_dscrptrs[0].mails_list->next = NULL;
    mock_dscrptrs[0].state = CLIENT_FSM_ST_CONNECT;

    add_first(&mock_dscrptrs[0].mails_list, "/home/dev/pvs-course-project/smtp_proj/client/tests/mail/user1/new/1.2.localhost.com,S=41.mbox");
    return 0;
}

int clean_suite(void) { return 0; }

void EMAIL_FILE_READ_SUCCESS(void)
{
    char *mail_path = "/home/dev/pvs-course-project/smtp_proj/client/tests/mail/user1/new/1.3.localhost.com,S=41.mbox";
    FILE *fp = fopen(mail_path, "a+");
    const char *text = "Write this to the file";
    fprintf(fp, "Some text: %s\n", text);
    fclose(fp);
    char *read_msg = read_msg_file(mail_path);
    CU_ASSERT_NOT_EQUAL(read_msg, NULL);
    remove(mail_path);
}

void EMAIL_FILE_READ_FAILED(void)
{
    char *read_msg = read_msg_file("/home/dev/pvs-course-project/smtp_proj/client/tests/mail/user1/new/1.4.localhost.com,S=41.mbox");
    CU_ASSERT_EQUAL(read_msg, NULL);
}

void DOMAIN_SERVER_INFO_GOT_SUCCESS(void)
{
    struct sockaddr_in mail_server = get_domain_server_info("gmail.com");
    CU_ASSERT_EQUAL(mail_server.sin_port, htons(25));
}

void DOMAIN_SERVER_INFO_GOT_FAILED(void)
{
    struct sockaddr_in mail_server = get_domain_server_info("dsdsdsdsds.net");
    CU_ASSERT_EQUAL(mail_server.sin_port, htons(0));
}

void GMAIL_MX_SERVER_NAME_GOT_SUCCESS(void)
{
    char *server_name = get_domain_mx_server_name("gmail.com");
    CU_ASSERT_NOT_EQUAL(server_name, NULL);
}

void LOCALHOST_MX_SERVER_NAME_GOT_SUCCESS(void)
{
    char *server_name = get_domain_mx_server_name("localhost.com");
    CU_ASSERT_STRING_EQUAL(server_name, "localhost");
}

void FAKE_MX_SERVER_NAME_GOT_FAILED(void)
{
    char *server_name = get_domain_mx_server_name("noname.com");
    CU_ASSERT_EQUAL(server_name, NULL);
}

void ONE_EMAIL_SENT_SUCCESS(void)
{
    char *server_response;
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }

    te_client_fsm_event event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_helo(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_msg_body(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_quit(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);
}

void TWO_EMAILS_ONE_SESSION_SENT_SUCCESS(void)
{
    init_suite();
    char *server_response;
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    te_client_fsm_event event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    /* First EMAIL */
    send_helo(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_msg_body(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    /* Second EMAIL */
    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_msg_body(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_quit(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);
}

void TWO_EMAILS_TWO_SESSIONS_SENT_SUCCESS(void)
{
    init_suite();
    char *server_response;
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    te_client_fsm_event event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    /* First EMAIL */
    send_helo(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_msg_body(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);
    close(mock_dscrptrs[0].socket_fd);
    mock_dscrptrs[0].socket_fd = connect_to_mail_server(0, mock_dscrptrs[0].domain_mail_server, "localhost.com");

    /* Second EMAIL */
    mock_dscrptrs[0].buffer = read_msg_file(mock_dscrptrs[0].mails_list->val);
    send_mail_from(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_rcpt_to(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].buffer, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_data_msg(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_msg_body(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);

    send_quit(mock_dscrptrs[0].socket_fd, mock_dscrptrs[0].request_buf);
    while ((server_response = read_data_from_server(mock_dscrptrs[0].socket_fd)) == NULL)
    {
        ;
    }
    event = check_server_code(server_response);
    CU_ASSERT(event == CLIENT_FSM_EV_OK);
}

int main(void)
{
    CU_pSuite smtp_client_suite = NULL;
    CU_pSuite msg_unit_suite = NULL;
    CU_pSuite mx_utils_unit_suite = NULL;
    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    smtp_client_suite = CU_add_suite("SMTP client tests suite", init_suite, clean_suite);
    msg_unit_suite = CU_add_suite("MSG unit tests suite", NULL, clean_suite);
    mx_utils_unit_suite = CU_add_suite("MX utils unit tests suite", NULL, clean_suite);
    // if (NULL == smtp_client_suite)
    // {
    //     CU_cleanup_registry();
    //     return CU_get_error();
    // }

    if (NULL == msg_unit_suite)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(smtp_client_suite, "ONE_EMAIL_SENT_SUCCESS", ONE_EMAIL_SENT_SUCCESS)) ||
        (NULL == CU_add_test(smtp_client_suite, "TWO_EMAILS_ONE_SESSION_SENT_SUCCESS", TWO_EMAILS_ONE_SESSION_SENT_SUCCESS)) ||
        /* (NULL == CU_add_test(smtp_client_suite, "TWO_EMAILS_TWO_SESSIONS_SENT_SUCCESS", TWO_EMAILS_TWO_SESSIONS_SENT_SUCCESS)) ||*/
        (NULL == CU_add_test(msg_unit_suite, "EMAIL_FILE_READ_SUCCESS", EMAIL_FILE_READ_SUCCESS)) ||
        (NULL == CU_add_test(msg_unit_suite, "EMAIL_FILE_READ_FAILED", EMAIL_FILE_READ_FAILED)) ||
        (NULL == CU_add_test(mx_utils_unit_suite, "DOMAIN_SERVER_INFO_GOT_SUCCESS", DOMAIN_SERVER_INFO_GOT_SUCCESS)) ||
        (NULL == CU_add_test(mx_utils_unit_suite, "DOMAIN_SERVER_INFO_GOT_FAILED", DOMAIN_SERVER_INFO_GOT_FAILED)) ||
        (NULL == CU_add_test(mx_utils_unit_suite, "GMAIL_MX_SERVER_NAME_GOT_SUCCESS", GMAIL_MX_SERVER_NAME_GOT_SUCCESS)) ||
        (NULL == CU_add_test(mx_utils_unit_suite, "LOCALHOST_MX_SERVER_NAME_GOT_SUCCESS", LOCALHOST_MX_SERVER_NAME_GOT_SUCCESS)) ||
        (NULL == CU_add_test(mx_utils_unit_suite, "FAKE_MX_SERVER_NAME_GOT_FAILED", FAKE_MX_SERVER_NAME_GOT_FAILED)))
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
