#include "protocol.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "pattern.h"
#include "smtp-fsm.h"

patterns_t patterns;

int protocol_init()
{
    if (pattern_init() != 0)
        return -1;
    return 0;
}

int protocol_update()
{
    for (int i = 0; i < CONN_SIZE; i++)
    {
        conn_t *conn;
        if ((conn = connections[i]) == NULL)
            continue;
        if (conn->recv_buf[0] != '\0')
            printf("%s\n", conn->recv_buf);
        strcpy(conn->send_buf, conn->recv_buf);
        memset(conn->recv_buf, 0, sizeof(char) * RECV_BUF_SIZE);
    }
    return 0;
}

int process_helo(conn_t *conn, const char* data)
{
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    strcpy(conn->send_buf, "250 example.com\r\n");
    return 0;
}

int process_ehlo(conn_t *conn, const char* data)
{
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    strcpy(conn->send_buf, "250 example.com\r\n");
    return 0;
}

int process_mail(conn_t *conn, const char* data)
{
    if (data == NULL) {
        log_w("%s", "No data in MAIL FROM:");
        conn->state = server_step(conn->state, SERVER_EV_ERR, NULL);
        strcpy(conn->send_buf, response_451);
    } else {
        conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
        conn->letter = letter_create();
        conn->letter->mail_from = (char *)data;
        strcpy(conn->send_buf, response_250);
    }
    return 0;
}

int process_rcpt(conn_t *conn, const char* data)
{
    if (data == NULL) {
        conn->state = server_step(conn->state, SERVER_EV_ERR, NULL);
        strcpy(conn->send_buf, response_501);
    } else {
        const char* content;
        conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
        pattern_compute(PT_EMAIL, data, &content);
        strcpy(conn->letter->rcpt_to, data);
        strcpy(conn->letter->rcpt_username, &content[0]);
        strcpy(conn->send_buf, response_250);
    }
    return 0;
}

int process_data(conn_t *conn, const char* data)
{
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    strcpy(conn->send_buf, response_354);
    return 0;
}

int process_data_recv(conn_t *conn, const char* data);

int process_data_end(conn_t *conn, const char* data);

int process_rset(conn_t *conn, const char* data)
{
    letter_free(conn->letter);
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    strcpy(conn->send_buf, response_250);
    return 0;
}

int process_vrfy(conn_t *conn, const char* data)
{
    conn->state = conn->old_state;
    strcpy(conn->send_buf, response_502);
    return 0;
}

int process_quit(conn_t *conn, const char* data)
{
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    strcpy(conn->send_buf, response_221);
    return 0;
}
