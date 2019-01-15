#include "../include/smtp_client.h"

// Отправляет сообщение HELO почтовому серверу
int send_helo(int socket_fd, char *request_buf)
{
    bzero(request_buf, MAX_BUF_LEN);
    gethostname(client_host_name, MAX_BUF_LEN);
    strcpy(request_buf, "HELO ");
    strcat(request_buf, client_host_name);
    strcat(request_buf, "\n");
    send_data(request_buf, 0, socket_fd);
    log_i("Sending HELO: %s", request_buf);
    bzero(request_buf, MAX_BUF_LEN);
    return 1;
}

// Отправляет сообщение MAIL FROM почтовому серверу
int send_mail_from(int socket_fd, char *msg, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    sprintf(request_buf, "MAIL FROM: <");
    token = strtok(msg, line);
    strcat(request_buf, token);
    strcat(request_buf, ">\n");
    log_i("Sending MAIL FROM: %s", request_buf);
    send_data(request_buf, 1, socket_fd);
    bzero(request_buf, MAX_BUF_LEN);
    return 1;
}

// Отправляет сообщение RCPT TO почтовому серверу
int send_rcpt_to(int socket_fd, char *msg, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    strcpy(request_buf, "RCPT TO:<");
    token = strtok(NULL, line);
    strcat(request_buf, token);
    strcat(request_buf, ">\n");
    log_i("Sending RCPT TO: %s", request_buf);
    send_data(request_buf, 1, socket_fd);
    bzero(request_buf, MAX_BUF_LEN);
    return 1;
}

// Отправляет сообщение DATA почтовому серверу
int send_data_msg(int socket_fd, char *request_buf)
{
    strcpy(request_buf, "DATA\n");
    log_i("Sending DATA message: %s", request_buf);
    send_data(request_buf, 1, socket_fd);
    bzero(request_buf, MAX_BUF_LEN);
    return 1;
}

// Отправляет заголовки сообщения почтовому серверу
int send_headers(int socket_fd, char *request_buf)
{
    char *token;
    const char line[3] = "\n";
    token = strtok(NULL, line);
    log_i("Sending EMAIL BODY: %s \n", request_buf);
    while (token != NULL)
    {
        //this condition means iterate on the token until you get to the black line(strlen=1)
        //which separates the message headers from the message body
        bzero(request_buf, MAX_BUF_LEN);
        strcpy(request_buf, token);
        strcat(request_buf, "\n");
        send_data(request_buf, 0, socket_fd);
        //printf("SEND body: %s \n", request_buf);
        token = strtok(NULL, line);
    }
    bzero(request_buf, MAX_BUF_LEN);
    //printf("SEND body: %s \n", "\r\n.\r\n");
    send_data("\r\n.\r\n", 0, socket_fd);
    return 1;
}

// Отправляет тело сообщения почтовому серверу
int send_msg_body(int socket_fd)
{
    char *token;
    const char line[3] = "\n";
    while (token != NULL)
    {
        send_data(token, 0, socket_fd);
        token = strtok(NULL, line);
    }

    //sending point to end body
    log_i("%s", "Sending EMAIL BODY end \n");
    send_data("\r\n.\r\n", 1, socket_fd);
    return 1;
}

// Отправляет сообщение QUIT почтовому серверу
int send_quit(int socket_fd, char *request_buf)
{
    strcpy(request_buf, "QUIT\n");
    //printf("SEND QUIT: %s \n", buf);
    log_i("%s", "Sending QUIT \n");
    send_data(request_buf, 1, socket_fd);
    bzero(request_buf, MAX_BUF_LEN);
    return 1;
}

//Writes data to output socket
void send_data(char *data, int to_read, int socket_fd)
{
    int n = 0;
    //printf("SENDING DATA %s \n", data);
    n = write(socket_fd, data, strlen(data));
    if (n < 0)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }
}

int get_server_response_code(int socket_fd, char *response_buf)
{
    bzero(response_buf, MAX_BUF_LEN);
    read_fd_line(socket_fd, response_buf, MAX_BUF_LEN);
    char server_returned_code[4] = "   ";
    memcpy(server_returned_code, response_buf, strlen(server_returned_code));
    int code = 0;
    code = atoi(server_returned_code);
    log_i("Socket %d SERVER code %d", socket_fd, code);
    //printf("code number:%d\n", code);
    return code;
}

//reads a line from fd to a char array
int read_fd_line(int fd, char *line, int lim)
{
    int i;
    char c;

    i = 0;
    while (--lim > 0 && read(fd, &c, 1) > 0 && c != '\n' && c != '\0')
    {
        line[i++] = c;
    }
    if (c == '\n')
        line[i++] = c;
    line[i] = '\0';
    //printf("Socket %d SERVER RESPONSE %s \n", fd, line);
    log_i("Socket %d SERVER RESPONSE %s", fd, line);
    return i;
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
        log_e("%s","Could not read data from server");
        return NULL;
    }

    log_i("Socket %d SERVER RESPONSE %s", socket_fd, msg);
    return msg;
}

te_client_fsm_event check_server_code(char *response)
{
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