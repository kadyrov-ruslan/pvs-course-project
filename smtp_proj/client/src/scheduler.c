#include "../include/scheduler.h"

#define N 4096
#define MAX_FD_CNT 1024
#define MAX_Q_LEN 10
#define MAX_BUF_LEN 1024
#define INITIAL_SIZE 10 //initial size of 'buffer'

struct mail_domain_dscrptr mail_domains_dscrptrs[60];
int ready_domains_count = 0;

char s[3];    //string to store 's: '
char c[3];    //string to store 'c: '
char *suffix; //string to store server suffix
char *msg;    //string to store msg body

char *buffer;                 //dynamically allocated char array to get messages from the server
int length;                   //current size of buffer
char myHostName[MAX_BUF_LEN]; //string to store my host name

int run_client()
{
    while (1)
    {
        printf("\n ########### NEW PERIOD ############## \n");
        char *raw_mail_domains[60];
        int raw_domains_count = get_out_mail_domains(raw_mail_domains);
        printf("WHOLE domains count %d\n", raw_domains_count);

        // выбираем только новые почтовые домены для получения mx записей и созданя сокета для них
        char *mail_domains[60];
        int domains_count = get_domains_diff(raw_domains_count, raw_mail_domains, mail_domains);
        //free(raw_mail_domains);

        printf("NEW domains count %d\n", domains_count);
        for (int i = 0; i < domains_count; i++)
        {
            printf(" ------------------------------- \n");
            printf("Mail domain %s\n", mail_domains[i]);
            char *res = get_domain_mx_server(mail_domains[i]);
            struct sockaddr_in cur_domain_srv = get_domain_server_info(res);
            cur_domain_srv.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
            cur_domain_srv.sin_port = htons(25); //port 25=SMTP.

            printf("server IP:%s\n", inet_ntoa(cur_domain_srv.sin_addr));
            mail_domains_dscrptrs[ready_domains_count].domain = mail_domains[i];
            mail_domains_dscrptrs[ready_domains_count].domain_mail_server = cur_domain_srv;

            int cur_domain_socket_fd = 0;
            if ((cur_domain_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                printf("\n Error : Could not create socket \n");
                return 1;
            }

            if (connect(cur_domain_socket_fd, (struct sockaddr *)&cur_domain_srv, sizeof(cur_domain_srv)) < 0)
            {
                printf("\n Error : Connect Failed \n");
                return 1;
            }
            else
            {
                printf("\n SUCCESS : Connected \n");
                mail_domains_dscrptrs[ready_domains_count].socket_fd = cur_domain_socket_fd;
            }

            char buf[MAX_BUF_LEN];
            read_fd_line(cur_domain_socket_fd, buf, MAX_BUF_LEN);
            //checkServerReturnedCode(buf);
            printf("%s", s);
            printf("%s\n", buf);
            get_suffix(buf);

            ready_domains_count++;
        }
        //free(mail_domains);

        printf(" -------- SOCKETS --------------- \n");
        for (int i = 0; i < ready_domains_count; i++)
        {
            printf("Mail domain: %s\n", mail_domains_dscrptrs[i].domain);
            printf("Mail domain socket fd: %d\n\n", mail_domains_dscrptrs[i].socket_fd);
        }
        printf(" -------------------------------- \n");
        
        if(domains_count > 0)
            process_output_mails();
        sleep(25);
    }

    return 1;
}

// Получает массив названий почтовых доменов, которым нужно отправить письма
int get_out_mail_domains(char **domains)
{
    int domains_count = 0;

    //run through users direrctories
    struct dirent *user_dir;
    DIR *mail_dir = opendir(conf.mail_dir);
    if (mail_dir == NULL)
        printf("Could not open MAIL directory");

    while ((user_dir = readdir(mail_dir)) != NULL)
    {
        char *user_dir_name = user_dir->d_name;
        if (strcmp(user_dir_name, ".") != 0 && strcmp(user_dir_name, "..") != 0)
        {
            char *user_dir_full_path = malloc(strlen(conf.mail_dir) + 2 + strlen(user_dir_name));
            strcpy(user_dir_full_path, conf.mail_dir);
            strcat(user_dir_full_path, user_dir_name);
            strcat(user_dir_full_path, "/");

            char *user_dir_new = malloc(strlen(user_dir_full_path) + 4);
            strcpy(user_dir_new, user_dir_full_path);
            strcat(user_dir_new, "new/");

            DIR *new_dir = opendir(user_dir_new);
            if (new_dir == NULL)
                printf("Could not open NEW directory");

            int mails_count = count_dir_entries(user_dir_new);
            if (mails_count > 0)
            {
                printf("directory is NOT EMPTY\n");
                struct dirent *new_entry;
                while ((new_entry = readdir(new_dir)) != NULL)
                {
                    if (strcmp(new_entry->d_name, ".") != 0 && strcmp(new_entry->d_name, "..") != 0)
                    {
                        char **tokens;
                        tokens = str_split(new_entry->d_name, '.');

                        char *first_part = tokens[2];
                        char *second_part = tokens[3];

                        char *tmp_cur_mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
                        strcpy(tmp_cur_mail_domain, first_part);
                        strcat(tmp_cur_mail_domain, ".");
                        strcat(tmp_cur_mail_domain, second_part);
                        free(tokens);

                        tokens = str_split(tmp_cur_mail_domain, ',');
                        //Проверяем, есть ли текущий домен массиве доменов
                        int found_domain_num = -1;
                        for (int i = 0; i < domains_count; i++)
                        {
                            if (strcmp(tokens[0], domains[i]) == 0)
                                found_domain_num = i;
                        }
                        //Домен не найден - добавляем в массив
                        if (found_domain_num < 0)
                        {
                            printf("ADDING  %s\n\n", tokens[0]);
                            domains[domains_count] = tokens[0];
                            domains_count++;
                        }
                        free(tokens);
                    }
                }
            }
            else
                printf("directory is EMPTY\n");

            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
}

//получает разницу между уже готовыми доменами и доменами, по которым нужно отправить почту
int get_domains_diff(int new_domains_count, char **new_mail_domains, char **dif)
{
    //printf("new domains count %d\n", new_domains_count);
    //printf("ready domains count %d\n", ready_domains_count);
    int diff_count = 0;
    for (int j = 0; j < new_domains_count; j++)
    {
        bool is_found = false;
        for (int i = 0; i < ready_domains_count; i++)
        {
            if (strcmp(new_mail_domains[j], mail_domains_dscrptrs[i].domain) == 0)
                is_found = true;
        }

        if (!is_found)
        {
            dif[diff_count] = new_mail_domains[j];
            diff_count++;
        }
    }
    return diff_count;
}

int process_output_mails()
{
    int domains_count = 0;

    //run through users direrctories
    struct dirent *user_dir;
    DIR *mail_dir = opendir(conf.mail_dir);
    if (mail_dir == NULL)
        printf("Could not open MAIL directory");

    while ((user_dir = readdir(mail_dir)) != NULL)
    {
        char *user_dir_name = user_dir->d_name;
        if (strcmp(user_dir_name, ".") != 0 && strcmp(user_dir_name, "..") != 0)
        {
            char *user_dir_full_path = malloc(strlen(conf.mail_dir) + 2 + strlen(user_dir_name));
            strcpy(user_dir_full_path, conf.mail_dir);
            strcat(user_dir_full_path, user_dir_name);
            strcat(user_dir_full_path, "/");

            char *user_dir_new = malloc(strlen(user_dir_full_path) + 4);
            strcpy(user_dir_new, user_dir_full_path);
            strcat(user_dir_new, "new/");

            DIR *new_dir = opendir(user_dir_new);
            if (new_dir == NULL)
                printf("Could not open NEW directory");

            int mails_count = count_dir_entries(user_dir_new);
            if (mails_count > 0)
            {
                char *email_full_names[mails_count];    
                char *email_short_names[mails_count];   
                int mail_names_count = 0;

                printf("directory is NOT EMPTY\n");
                struct dirent *new_entry;
                //reading emails full and short names
                while ((new_entry = readdir(new_dir)) != NULL)
                {
                    if (strcmp(new_entry->d_name, ".") != 0 && strcmp(new_entry->d_name, "..") != 0)
                    {
                        char *email_full_name = malloc(strlen(user_dir_new) + strlen(new_entry->d_name));
                        strcpy(email_full_name, user_dir_new);
                        strcat(email_full_name, new_entry->d_name);

                        email_full_names[mail_names_count] = email_full_name;
                        email_short_names[mail_names_count] = new_entry->d_name;
                        mail_names_count++;

                        //free(email_full_name);                    
                    }
                }

                for (int j = 0; j < mails_count; j++)
                {
                    //printf("EMAIL FULL NAME %s\n", email_full_names[j]);
                    //printf("EMAIL SHORT NAME %s\n", email_short_names[j]);

                    char *email_name = malloc(strlen(email_short_names[j]) + 1);
                    strcpy(email_name, email_short_names[j]);

                    char **tokens;
                    tokens = str_split(email_name, '.');

                    char *first_part = tokens[2];
                    char *second_part = tokens[3];

                    char *tmp_cur_mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
                    strcpy(tmp_cur_mail_domain, first_part);
                    strcat(tmp_cur_mail_domain, ".");
                    strcat(tmp_cur_mail_domain, second_part);
                    free(tokens);

                    tokens = str_split(tmp_cur_mail_domain, ',');
                    char *cur_email_domain = tokens[0];
                    free(tokens);

                    for (int i = 0; i < ready_domains_count; i++)
                    {
                        if (strcmp(mail_domains_dscrptrs[i].domain, cur_email_domain) == 0)
                        {
                            //read file to buf
                            //send_to_server(mail_domains_dscrptrs[i].socket_fd);
                            printf("EMAIL DOMAIN %s\n", cur_email_domain);
                            printf("EMAIL NAME %s\n", email_short_names[j]);
                            printf("SENDING DATA . . . \n");

                            //printf("NEW ENTRY %s\n", email_full_names[j]);
                            char *email_new_name = str_replace(email_full_names[j], "new", "cur");
                            //printf("ENTRY NEW NAME %s\n", email_new_name);

                            int ret;
                            ret = rename(email_full_names[j], email_new_name);
                            if (ret == 0)
                                printf("File renamed successfully\n");
                            else
                                printf("Error: unable to rename the file\n");

                            break;
                        }
                    }
                }
            }
            else
                printf("directory is EMPTY\n");

            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
}

struct sockaddr_in get_domain_server_info(char *domain_name)
{
    struct sockaddr_in mail_server; //this struct contains ip address and a port of the server.
    struct hostent *mail_info;      //this struct contains all the info of a host name in the Internet.

    char *mail_ip;
    mail_info = gethostbyname(domain_name);
    if (mail_info == NULL)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }

    mail_ip = (char *)malloc(INET_ADDRSTRLEN + 1);
    inet_ntop(AF_INET, mail_info->h_addr_list[0], mail_ip, INET_ADDRSTRLEN);
    free(mail_ip);

    bzero(&mail_server, sizeof(mail_server));
    mail_server.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
    mail_server.sin_port = htons(25); //port 25=SMTP.
    bcopy((char *)mail_info->h_addr_list[0], (char *)&mail_server.sin_addr.s_addr, mail_info->h_length);
    return mail_server;
}

char *get_domain_mx_server(char *domain_name)
{
    static char mx_server[4096];
    u_char nsbuf[N];
    ns_msg msg;
    ns_rr rr;
    int r;

    // MX RECORD
    r = res_query(domain_name, ns_c_any, ns_t_mx, nsbuf, sizeof(nsbuf));
    if (r < 0)
        perror(domain_name);
    else
    {
        ns_initparse(nsbuf, r, &msg);
        ns_parserr(&msg, ns_s_an, 0, &rr);

        const size_t size = NS_MAXDNAME;
        unsigned char name[size];
        int t = ns_rr_type(rr);

        const u_char *data = ns_rr_rdata(rr);
        if (t == T_MX)
        {
            ns_name_unpack(nsbuf, nsbuf + r, data + sizeof(u_int16_t), name, size);
            ns_name_ntop(name, mx_server, 4096);
        }
    }

    return mx_server;
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
    return i;
}

//this method gets the suffix of the mail server.
void get_suffix(char *buf)
{
    char *token;
    char *tmp;
    const char space[3] = " ";

    //--------getting the suffix------//
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

void send_to_server(int socket_fd)
{
    char buf[MAX_BUF_LEN];
    char *token;
    const char line[3] = "\n";

    //--------sending HELO----------------//
    bzero(buf, MAX_BUF_LEN);
    gethostname(myHostName, MAX_BUF_LEN);
    strcpy(buf, "HELO ");
    strcat(buf, myHostName);
    strcat(buf, "\n");
    send_data(buf, 0, socket_fd);
    bzero(buf, MAX_BUF_LEN);
    read_fd_line(socket_fd, buf, MAX_BUF_LEN);
    //checkServerReturnedCode(buf);
    printf("%s", s);
    printf("%s\n", buf);

    //--------sending MAIL FROM----------//
    bzero(buf, MAX_BUF_LEN);
    sprintf(buf, "MAIL FROM:");
    token = strtok(msg, line);
    strcat(buf, token);
    strcat(buf, "\n");
    send_data(buf, 1, socket_fd);

    //--------sending RCPT TO------------//
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "RCPT TO:");
    token = strtok(NULL, line);
    strcat(buf, token);
    strcat(buf, "\n");
    send_data(buf, 1, socket_fd);

    //--------sending DATA------//
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "DATA\n");
    send_data(buf, 1, socket_fd);

    //--------sending the body--------------//
    token = strtok(NULL, line);
    //sending the headers
    while (strlen(token) != 1)
    { //this condition means iterate on the token until you get to the black line(strlen=1)
        //which separates the message headers from the message body
        bzero(buf, MAX_BUF_LEN);
        strcpy(buf, token);
        strcat(buf, "\n");
        send_data(buf, 0, socket_fd);
        token = strtok(NULL, line);
    }
    //sending the msg body
    while (token != NULL)
    {
        send_data(token, 0, socket_fd);
        printf("\n");
        token = strtok(NULL, line);
    }

    //---sending point to end body--------//
    send_data("\r\n.\r\n", 1, socket_fd);

    //--------sending quit to end connection---------//
    bzero(buf, MAX_BUF_LEN);
    strcpy(buf, "QUIT\n");
    send_data(buf, 1, socket_fd);
}

//this method writes and read data to/from the socket.
void send_data(char *data, int toRead, int socket_fd)
{
    //-------writing the data to the server and printing it to the screen----------//
    int n = 0;
    n = write(socket_fd, data, strlen(data));
    if (n < 0)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }
    if (strcmp(data, "\r\n.\r\n") == 0)
    {
        printf("%s", c);
        printf("%s\n\n", ".");
    }
    else
    {
        printf("%s", c);
        printf("%s\n", data);
    }

    //---reading messages from the server dynamically-----------//

    if (toRead == 1)
    {                           //this means we need to read as well from the socket
        char tmp[INITIAL_SIZE]; //tmp string to store parts of the message
        int tmp_length = INITIAL_SIZE;
        int bytes = 0;
        //-----initializing buffer------------//
        buffer = (char *)malloc(INITIAL_SIZE);
        length = INITIAL_SIZE;
        bzero(buffer, length);
        bzero(tmp, tmp_length);
        while (1)
        {
            //--------reading data------------------//
            bzero(tmp, tmp_length);
            bytes += read(socket_fd, tmp, tmp_length - 1); //it reads length-1 to save space for '\0'.
            if (bytes < 0)
            {
                printf("%s\n", strerror(errno));
                exit(0);
            }
            //---checking if the buffer has enough length to contain the part of the message--------------//
            if (bytes < length)
            {
                strcat(buffer, tmp);
            }
            //----checking if the buffer does not has enough length to contain the part of the message
            //therefore it need to be realloced------------------//
            if (bytes >= length)
            {
                length = bytes;
                buffer = (char *)realloc(buffer, length + 1);
                strcat(buffer, tmp);
            }
            //---checking if we reached the suffix of the server------------//
            if (strstr(buffer, suffix) != NULL)
            {
                break;
            }
        }
        //---checking if code is valid -----------/
        //if (strcmp(data, "DATA\n") != 0)
        //  checkServerReturnedCode(buffer);
        //---printing the message and freeing the buffer ---------/
        printf("%s", s);
        printf("%s\n", buffer);
        free(buffer);
    }
}