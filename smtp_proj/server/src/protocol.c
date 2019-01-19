#include "protocol.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "maildir.h"
#include "smtp-fsm.h"
#include "stopwatch.h"

int protocol_init()
{
    if (pattern_init() != 0)
        return -1;

    event_map[PT_HELO] = SERVER_EV_HELO;
    event_map[PT_EHLO] = SERVER_EV_EHLO;
    event_map[PT_MAIL] = SERVER_EV_MAIL;
    event_map[PT_DATA] = SERVER_EV_DATA;
    event_map[PT_RCPT] = SERVER_EV_RCPT;
    event_map[PT_DATA_END] = SERVER_EV_DATA_END;
    event_map[PT_DATA_RECV] = SERVER_EV_DATA_RECV;
    event_map[PT_VRFY] = SERVER_EV_VRFY;
    event_map[PT_RSET] = SERVER_EV_RSET;
    event_map[PT_QUIT] = SERVER_EV_QUIT;

    process_bind[SERVER_ST_PROCESS_HELO] = process_helo;
    process_bind[SERVER_ST_PROCESS_EHLO] = process_ehlo;
    process_bind[SERVER_ST_PROCESS_MAIL] = process_mail;
    process_bind[SERVER_ST_PROCESS_RCPT] = process_rcpt;
    process_bind[SERVER_ST_PROCESS_DATA] = process_data;
    process_bind[SERVER_ST_PROCESS_DATA_END] = process_data_end;
    process_bind[SERVER_ST_PROCESS_DATA_RECV] = process_data_recv;
    process_bind[SERVER_ST_PROCESS_VRFY] = process_vrfy;
    process_bind[SERVER_ST_PROCESS_RSET] = process_rset;
    process_bind[SERVER_ST_PROCESS_QUIT] = process_quit;

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
        {
            log_d("%s\n", conn->recv_buf);

            const char* content = {0};
            pattern_type type = PT_START;
            while (type < PT_END)
            {
                if (pattern_compute(type, conn->recv_buf, &content) == 0)
                    break;
                type++;
            }

            log_d("%d: %s\n", type, content);
            if (type == PT_END || (type == PT_DATA_RECV && conn->state != SERVER_ST_EXPECT_DATA_RECV))
            {
                log_i("%s\n", "No match query for command");
                strcpy(conn->send_buf, response_500);
            } else
            {
                te_server_event event = event_map[type];
                conn->old_state = conn->state;
                conn->state = server_step(conn->state, event, NULL);
                if (type == PT_DATA_RECV)
                    process_bind[conn->state](conn, conn->recv_buf);
                else
                    process_bind[conn->state](conn, content);
            }

            memset(conn->recv_buf, 0, sizeof(char) * RECV_BUF_SIZE);
            stopwatch_start(conn->watch);
        }

        if (conn->state == SERVER_ST_INIT)
        {
            conn->old_state = conn->state;
            conn->state = server_step(conn->state, SERVER_EV_CONNECT, NULL);
            conn->old_state = conn->state;
            conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
            strcpy(conn->send_buf, response_220);
        }

        if (stopwatch_watch(conn->watch) > CONN_TIMEOUT)
        {
            conn->old_state = conn->state;
            conn->state = SERVER_ST_DISCONNECTED;
            strcpy(conn->send_buf, "421 Server Error: timeout exceeded\r\n");
        }
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
    if (data == NULL)
    {
        log_w("%s", "No data in MAIL FROM:");
        conn->state = server_step(conn->state, SERVER_EV_ERR, NULL);
        strcpy(conn->send_buf, response_451);
    }
    else
    {
        conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
        conn->letter = letter_create();
        conn->letter->mail_from = (char *)data;
        strcpy(conn->send_buf, response_250);
    }
    return 0;
}

int process_rcpt(conn_t *conn, const char* data)
{
    if (data == NULL)
    {
        conn->state = server_step(conn->state, SERVER_EV_ERR, NULL);
        strcpy(conn->send_buf, response_501);
    }
    else
    {
        const char* content;
        conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
        pattern_compute(PT_EMAIL, data, &content);
        strcpy(conn->letter->rcpt_to, data);

        const char **email = pattern_email(data);
        strcpy(conn->letter->rcpt_username, email[0]);
        strcpy(conn->letter->rcpt_domain, email[1]);

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

int process_data_recv(conn_t *conn, const char* data)
{
    char ending[6] = {0};
    strcpy(ending, &data[strlen(data) - 5]);
    if (strcmp(ending, "\r\n.\r\n") == 0)
    {
        conn->old_state = conn->state;
        conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
        conn->state = server_step(conn->state, SERVER_EV_DATA_END, NULL);
        return process_data_end(conn, data);
    }
    strcat(conn->letter->body, data);
    strcat(conn->letter->body, "\r\n");
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    return 0;
}

int process_data_end(conn_t *conn, const char* data)
{
    if (data != NULL && strlen(data))
    {
        strcat(conn->letter->body, data);
        strcat(conn->letter->body, "\r\n");
    }

    const char* filename;
    maildir_ensure_user(conn->letter->rcpt_username, USR_LOCAL);
    maildir_get_fname(conn->letter->rcpt_username, conn->letter->rcpt_domain, &filename);

    FILE *f_letter = NULL;
    if ((f_letter = fopen(filename, "ab+")) == NULL)
    {
        log_w("Could not create file %s", filename);
        strcpy(conn->send_buf, response_554);
        conn->state = SERVER_ST_READY;
        return -1;
    }

    fprintf(f_letter, "%s\n", conn->letter->mail_from);
    fprintf(f_letter, "%s\n", conn->letter->rcpt_to);
    fprintf(f_letter, "%s\n", conn->letter->body);
    fclose(f_letter);
    letter_free(conn->letter);
    strcpy(conn->send_buf, "250 Mail is saved on disk and will be queued for forwarding\r\n");
    conn->state = server_step(conn->state, SERVER_EV_OK, NULL);
    return 0;
}

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
