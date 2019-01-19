#include "../include/smtp_client.h"

int connect_to_mail_server(int socket_fd, struct sockaddr_in mail_server, char *email_domain)
{
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        log_e("Could not create socket to %s", email_domain);
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr *)&mail_server, sizeof(mail_server)) < 0)
    {
        log_e("Connection to %s Failed ", email_domain);
        return -1;
    }

    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    return socket_fd;
}

int close_all_conns(int max_fd)
{
    for (int i = 0; i <= max_fd; i++)
        //if (FD_ISSET(i, read_fds) || FD_ISSET(i, write_fds))
            close(i);

    return 0;
}

int send_msg_to_server(struct mail_domain_dscrptr *cur_mail_domain)
{
    struct timeval curr_time;
    if (cur_mail_domain->last_attempt_time != 0)
    {
        gettimeofday(&curr_time, NULL);
        int send_time = (int)curr_time.tv_sec - cur_mail_domain->last_attempt_time;
        if (send_time >= cur_mail_domain->retry_time && cur_mail_domain->total_send_time <= cur_mail_domain->total_send_time)
            cur_mail_domain->can_be_send = 1;
        else if (cur_mail_domain->total_send_time > cur_mail_domain->total_send_time)
        {
            cur_mail_domain->can_be_send = -1;
            return -3;
        }
        else
            return -2;
    }

    int code = 0;
    log_i("Socket %d of %s domain is in %d WRITE_FDS", cur_mail_domain->socket_fd, cur_mail_domain->domain, cur_mail_domain->state);
    switch (cur_mail_domain->state)
    {
    case CLIENT_FSM_ST_SEND_HELO:
        code = send_helo(cur_mail_domain->socket_fd, cur_mail_domain->request_buf);
        break;

    case CLIENT_FSM_ST_SEND_MAIL_FROM:
        cur_mail_domain->buffer = read_msg_file(cur_mail_domain->mails_list->val);
        char *email_new_name = str_replace(cur_mail_domain->mails_list->val, "new", "cur");
        log_i("new name %s \n", email_new_name);
        int ret = rename((*cur_mail_domain->mails_list).val, email_new_name);
        if (ret == 0)
        {
            log_i("%s", "File renamed successfully\n");
        }
        else
        {
            log_i("%s", "Error: unable to rename the file\n");
        }

        code = send_mail_from(cur_mail_domain->socket_fd, cur_mail_domain->buffer, cur_mail_domain->request_buf);
        break;

    case CLIENT_FSM_ST_SEND_RCPT_TO:
        code = send_rcpt_to(cur_mail_domain->socket_fd, cur_mail_domain->buffer, cur_mail_domain->request_buf);
        //cur_mail_domain->curr_rcpts_index++;
        break;

    case CLIENT_FSM_ST_SEND_DATA:
        code = send_data_msg(cur_mail_domain->socket_fd, cur_mail_domain->request_buf);
        break;

    case CLIENT_FSM_ST_SEND_BODY:
        code = send_msg_body(cur_mail_domain->socket_fd, cur_mail_domain->request_buf);
        break;

    case CLIENT_FSM_ST_SEND_QUIT:
        code = send_quit(cur_mail_domain->socket_fd, cur_mail_domain->request_buf);
        break;
    default:
        break;
    }

    return code;
}

// Отправляет сообщение HELO почтовому серверу
int send_helo(int socket_fd, char *request_buf)
{
    bzero(request_buf, MAX_BUF_LEN);
    gethostname(client_host_name, MAX_BUF_LEN);
    strcpy(request_buf, "HELO ");
    strcat(request_buf, client_host_name);
    strcat(request_buf, "\n");
    log_i("Sending HELO: %s", request_buf);
    return send_data(request_buf, socket_fd);
}

// Отправляет сообщение MAIL FROM почтовому серверу
int send_mail_from(int socket_fd, char *msg, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    bzero(request_buf, MAX_BUF_LEN);
    sprintf(request_buf, "MAIL FROM: <");
    token = strtok(msg, line);
    strcat(request_buf, token);
    strcat(request_buf, ">\n");
    log_i("Sending MAIL FROM: %s", request_buf);
    return send_data(request_buf, socket_fd);
}

// Отправляет сообщение RCPT TO почтовому серверу
int send_rcpt_to(int socket_fd, char *msg, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    bzero(request_buf, MAX_BUF_LEN);
    strcpy(request_buf, "RCPT TO:<");
    token = strtok(NULL, line);
    strcat(request_buf, token);
    strcat(request_buf, ">\n");
    log_i("Sending RCPT TO: %s", request_buf);
    return send_data(request_buf, socket_fd);
}

// Отправляет сообщение DATA почтовому серверу
int send_data_msg(int socket_fd, char *request_buf)
{
    bzero(request_buf, MAX_BUF_LEN);
    strcpy(request_buf, "DATA\n");
    log_i("Sending DATA message: %s", request_buf);
    return send_data(request_buf, socket_fd);
}

// Отправляет subject и тело сообщения почтовому серверу
int send_msg_body(int socket_fd, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    bzero(request_buf, MAX_BUF_LEN);
    token = strtok(NULL, line);
    log_i("Sending EMAIL BODY: %s \n", request_buf);
    while (token != NULL)
    {
        //this condition means iterate on the token until you get to the black line(strlen=1)
        //which separates the message headers from the message body
        bzero(request_buf, MAX_BUF_LEN);
        strcpy(request_buf, token);
        strcat(request_buf, "\n");
        send_data(request_buf, socket_fd);
        token = strtok(NULL, line);
    }
    bzero(request_buf, MAX_BUF_LEN);
    return send_data("\r\n.\r\n", socket_fd);
}

// Отправляет сообщение QUIT почтовому серверу
int send_quit(int socket_fd, char *request_buf)
{
    bzero(request_buf, MAX_BUF_LEN);
    strcpy(request_buf, "QUIT\n");
    log_i("%s", "Sending QUIT \n");
    return send_data(request_buf, socket_fd);
}

//Writes data to output socket
int send_data(char *data, int socket_fd)
{
    int n = 0;
    //printf("SENDING DATA %s \n", data);
    n = write(socket_fd, data, strlen(data));
    if (n < 0)
    {
        log_e("%s", "Unable to send data to mail server \n");
        return -1;
    }

    return 0;
}

char *read_data_from_server(int socket_fd)
{
    char buf[MAX_BUF_LEN];
    char *msg = (char *)malloc(MAX_BUF_LEN);

    ssize_t nread = 0;
    ssize_t read_bytes = 0;
    ssize_t msg_size = MAX_BUF_LEN;
    ssize_t buf_size = MAX_BUF_LEN;

    bzero(buf, buf_size);
    bzero(msg, msg_size);
    msg[0] = '\0';

    while ((nread = recv(socket_fd, buf, buf_size - 1, 0)) > 0)
    {
        if (nread > buf_size)
            break;

        read_bytes += nread;
        if (read_bytes < msg_size)
        {
            strcat(msg, buf);
        }
        else
        {
            msg_size = read_bytes;
            msg = (char *)realloc(msg, msg_size + 1);
            strcat(msg, buf);
        }

        if (strstr(msg, "\r\n") != NULL)
            break;

        bzero(buf, buf_size);
    }

    if (nread < 0)
    {
        log_e("%s", "Could not read data from server");
        return NULL;
    }
    return msg;
}

te_client_fsm_event check_server_code(char *response)
{
    log_i("SERVER RESPONSE %s", response);
    char server_returned_code[4] = "   ";
    memcpy(server_returned_code, response, strlen(server_returned_code));
    int code = atoi(server_returned_code);
    if (code < 200 || (code > 300 && code != 354))
    {
        char error_msg[1024];
        bzero(error_msg, 1024);
        strcpy(error_msg, "Server code = ");
        strcat(error_msg, server_returned_code);
        log_e("SERVER error msg %s", error_msg);

        if (code == 502 && strstr(response, "not implemented") != NULL)
            return CLIENT_FSM_EV_COMMAND_NOT_IMPLEMENTED;
        else
            return CLIENT_FSM_EV_ERROR;
    }

    return CLIENT_FSM_EV_OK;
}