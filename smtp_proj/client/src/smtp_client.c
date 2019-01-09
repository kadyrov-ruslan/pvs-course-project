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
        token = strtok(NULL, line);
    }
    bzero(request_buf, MAX_BUF_LEN);
    send_data("\r\n.\r\n", 1, socket_fd);
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
    //log_i("Socket %d SERVER code %d", socket_fd, code);
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