#include "../include/smtp_client.h"

//Gets suffix of the mail server
void get_suffix(char *buf)
{
    char *token;
    char *tmp;
    const char space[3] = " ";

    /* get the first token */
    token = strtok(buf, space);
    /* walking through other tokens until we reached the last word which is the suffix */
    while (1)
    {
        tmp = malloc(strlen(token) + 1);
        strcpy(tmp, token);
        token = strtok(NULL, space);
        if (token == NULL)
            break;
        free(tmp);
    }
    suffix = malloc((sizeof(char) * strlen(tmp)) + 1);
    strcpy(suffix, tmp);
    free(tmp);
}

// Отправляет сообщение HELO почтовому серверу
int send_helo(int socket_fd)
{
    bzero(buf, MAX_BUF_LEN);
    gethostname(client_host_name, MAX_BUF_LEN);
    strcpy(buf, "HELO ");
    strcat(buf, client_host_name);
    strcat(buf, "\n");
    send_data(buf, 0, socket_fd);
    printf("SEND HELO: %s \n", buf);
    bzero(buf, MAX_BUF_LEN);
    return 1;
}

// Отправляет сообщение MAIL FROM почтовому серверу
int send_mail_from(int socket_fd, char *msg)
{
    char *token;
    const char line[3] = "\n";
    bzero(buf, MAX_BUF_LEN);
    sprintf(buf, "MAIL FROM: <");
    token = strtok(msg, line);
    strcat(buf, token);
    strcat(buf, ">\n");
    //printf("SEND MAIL FROM is : %s \n", buf);
    send_data(buf, 1, socket_fd);
    return 1;
}

// Отправляет сообщение RCPT TO почтовому серверу
int send_rcpt_to(int socket_fd, char *msg)
{
    char *token;
    const char line[3] = "\n";
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "RCPT TO:<");
    token = strtok(NULL, line);
    strcat(buf, token);
    strcat(buf, ">\n");
    //printf("SEND RCPT TO is : %s \n", buf);
    send_data(buf, 1, socket_fd);
    return 1;
}

// Отправляет сообщение DATA почтовому серверу
int send_data_msg(int socket_fd)
{
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "DATA\n");
    //printf("SEND DATA MSG is : %s \n", buf);
    send_data(buf, 1, socket_fd);
    return 1;
}

// Отправляет заголовки сообщения почтовому серверу
int send_headers(int socket_fd)
{
    char *token;
    const char line[3] = "\n";
    token = strtok(NULL, line);
    //sending the headers
    while (token != NULL)
    {
        //this condition means iterate on the token until you get to the black line(strlen=1)
        //which separates the message headers from the message body
        bzero(buf, MAX_BUF_LEN);
        strcpy(buf, token);
        strcat(buf, "\n");
        printf("SEND HEADERS: %s \n", buf);
        send_data(buf, 0, socket_fd);
        token = strtok(NULL, line);
    }
    return 1;
}

// Отправляет тело сообщения почтовому серверу
int send_msg_body(int socket_fd)
{
    printf("SEND MSG BODY FUNC: %s \n", buf);
    char *token;
    const char line[3] = "\n";
    while (token != NULL)
    {
        printf("SEND TOKEN: %s \n", buf);
        send_data(token, 0, socket_fd);
        printf("\n");
        token = strtok(NULL, line);
    }

    //sending point to end body
    send_data("\r\n.\r\n", 1, socket_fd);
    return 1;
}

// Отправляет сообщение QUIT почтовому серверу
int send_quit(int socket_fd)
{
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "QUIT\n");
    printf("SEND QUIT: %s \n", buf);
    send_data(buf, 1, socket_fd);
    return 1;
}

//TODO OBSOLETE
void send_msg_to_server(int socket_fd, char *msg)
{
    char buf[MAX_BUF_LEN];
    char *token;
    const char line[3] = "\n";

    //sending HELO
    bzero(buf, MAX_BUF_LEN);
    gethostname(client_host_name, MAX_BUF_LEN);
    strcpy(buf, "HELO ");
    strcat(buf, client_host_name);
    strcat(buf, "\n");
    send_data(buf, 0, socket_fd);
    bzero(buf, MAX_BUF_LEN);
    read_fd_line(socket_fd, buf, MAX_BUF_LEN);
    check_server_response_code(buf);
    printf("%s", s);

    //sending MAIL FROM
    bzero(buf, MAX_BUF_LEN);
    sprintf(buf, "MAIL FROM: <");
    token = strtok(msg, line);
    strcat(buf, token);
    strcat(buf, ">\n");
    send_data(buf, 1, socket_fd);

    //sending RCPT TO
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "RCPT TO:<");
    token = strtok(NULL, line);
    strcat(buf, token);
    strcat(buf, ">\n");
    printf("%s\n", buf);
    send_data(buf, 1, socket_fd);

    //sending DATA
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "DATA\n");
    printf("%s\n", buf);
    send_data(buf, 1, socket_fd);

    printf("sending the body \n");
    //sending the body
    token = strtok(NULL, line);
    //sending the headers
    while (token != NULL)
    {
        //this condition means iterate on the token until you get to the black line(strlen=1)
        //which separates the message headers from the message body
        bzero(buf, MAX_BUF_LEN);
        strcpy(buf, token);
        strcat(buf, "\n");
        send_data(buf, 0, socket_fd);
        token = strtok(NULL, line);
    }
    printf("sending the msg body \n");
    //sending the msg body
    while (token != NULL)
    {
        send_data(token, 0, socket_fd);
        printf("\n");
        token = strtok(NULL, line);
    }

    //sending point to end body
    send_data("\r\n.\r\n", 1, socket_fd);

    //sending quit to end connection
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "QUIT\n");
    send_data(buf, 1, socket_fd);
}

//Writes data to output socket
void send_data(char *data, int to_read, int socket_fd)
{   
    int n = 0;
    n = write(socket_fd, data, strlen(data));
    if (n < 0)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }
}

//Gets code of response msg
void check_server_response_code(char *buf)
{
    char server_returned_code[4] = "   ";
    memcpy(server_returned_code, buf, strlen(server_returned_code));
    int code = 0;
    code = atoi(server_returned_code);
    if (code < 200 || code > 300)
    {
        printf("code number:%d\n", code);
        printf("ERROR:%s\n", buf);
        exit(0);
    }
}

int get_server_response_code(int socket_fd)
{
    read_fd_line(socket_fd, buf, MAX_BUF_LEN);
    char server_returned_code[4] = "   ";
    memcpy(server_returned_code, buf, strlen(server_returned_code));
    int code = 0;
    code = atoi(server_returned_code);
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
    //log_i("Socket %d SERVER RESPONSE %s", fd, line);
    return i;
}



void send_body_to_server(int socket_fd, char *msg)
{
    char buf[MAX_BUF_LEN];
    char *token;
    const char line[3] = "\n";

    //sending DATA
    bzero(buf, MAX_BUF_LEN);
    printf("sending the body \n");
    //sending the body
    token = strtok(NULL, line);
    //sending the headers
    while (token != NULL)
    {
        bzero(buf, MAX_BUF_LEN);
        strcpy(buf, token);
        strcat(buf, "\n");
        //send_data(buf, 0, socket_fd);
        token = strtok(NULL, line);
    }
    printf("sending the msg body \n");
    //sending the msg body
    while (token != NULL)
    {
        send_data(token, 0, socket_fd);
        printf("\n");
        token = strtok(NULL, line);
    }

    //sending point to end body
    send_data("\r\n.\r\n", 1, socket_fd);
}