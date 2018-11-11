#include "../include/scheduler.h"

#define N 4096
#define MAX_FD_CNT 1024
#define MAX_Q_LEN 10
#define MAX_BUF_LEN 1024

int run_client()
{
    char *mail_domains[15];
    int domains_count = get_out_mail_domains(mail_domains);
    printf("domains count %d\n", domains_count);

    for (int i = 0; i < domains_count; i++)
    {
        printf(" ------------------------------- \n");
        printf("Mail domain %s\n", mail_domains[i]);
        char *res = get_domain_mx_server(mail_domains[i]);
        struct sockaddr_in cur_domain_socket_info = get_domain_server_info(res);

        printf("server IP:%s\n",   inet_ntoa(cur_domain_socket_info.sin_addr));
        printf("server port:%d\n", cur_domain_socket_info.sin_port);

        //bind socket for cur server
        //save fd of sockets and run process
    }

    // while (1)
    // {
    //     printf(" ########### NEW PERIOD ############## \n");
    //     //Check mail domains' folder for new messages
    //     struct MapItem mail_domains_msgs[3];
    //     int mail_domains_count = get_output_mails(mail_domains_msgs);

    //     printf("domains Mails count %d\n", mail_domains_count);

    //     for (int i = 0; i < mail_domains_count; i++)
    //     {
    //         printf(" ------------------------------- \n");
    //         printf("Mail domain %s\n", mail_domains_msgs[i].domain);
    //         //bind socket for cur server
    //         //save fd of sockets and run process
    //     }
    //     sleep(5);
    // }
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
                            printf("ADDING  %s\n", tokens[0]);
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

int get_output_mails(struct MapItem *items)
{
    int items_count = 0;

    const char *output_mails_dir = "/new";
    char *full_out_maildir = malloc(strlen(conf.mail_dir) + 1 + 4);
    strcpy(full_out_maildir, conf.mail_dir);
    strcat(full_out_maildir, output_mails_dir);

    // Pointer for dir entry
    struct dirent *de;
    DIR *dir = opendir(full_out_maildir);
    if (dir == NULL)
        printf("Could not open current directory");

    while ((de = readdir(dir)) != NULL)
    {
        char *d_name = de->d_name;
        char *cur_domain_dir = malloc(strlen(d_name));
        strcpy(cur_domain_dir, d_name);

        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            char *domain_dir = malloc(1 + strlen(cur_domain_dir));
            strcpy(domain_dir, "/");
            strcat(domain_dir, cur_domain_dir);

            char *full_domain_dir = malloc(strlen(full_out_maildir) + 5 + strlen(domain_dir));
            strcpy(full_domain_dir, full_out_maildir);
            strcat(full_domain_dir, "/");
            strcat(full_domain_dir, cur_domain_dir);

            int mails_count = count_dir_entries(full_domain_dir);
            if (mails_count > 0)
            {
                printf("directory is NOT EMPTY\n");
                int mails_fd[mails_count];
                int cur_domain_mails_count = 0;

                //получаем fd писем
                struct dirent *d;
                DIR *dir = opendir(full_domain_dir);
                while ((d = readdir(dir)) != NULL)
                {
                    if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
                    {
                        char *file_full_name = malloc(5 + strlen(full_domain_dir));
                        strcpy(file_full_name, full_domain_dir);
                        strcat(file_full_name, "/");
                        strcat(file_full_name, d->d_name);

                        int fd;
                        if ((fd = open(file_full_name, O_RDONLY)) == -1)
                            printf("error %s\n", strerror(errno));
                        else
                        {
                            //printf("fd %d\n", fd);
                            mails_fd[cur_domain_mails_count] = fd;
                            cur_domain_mails_count = cur_domain_mails_count + 1;
                        }
                    }
                }
                closedir(dir);

                items[items_count].full_domain_dir = full_domain_dir;
                items[items_count].domain = cur_domain_dir;
                items[items_count].values = mails_fd;
                items[items_count].values_count = cur_domain_mails_count;
                items_count = items_count + 1;
            }
            else
                printf("directory is EMPTY\n");
        }
    }

    closedir(dir);

    /*for (int i = 0; i < items_count; i++)
    {
        printf(" ----$#############################---------- \n");
        printf("Mail domain %s\n", items[i].domain);
        printf("Full Mail domain dir %s\n", items[i].full_domain_dir);
        printf("Mails count %d\n", items[i].values_count);

        get_domain_server_info(items[i].domain);
        //bind socket for cur server
        //save fd of sockets and run process
    }*/

    return items_count;
}

struct sockaddr_in get_domain_server_info(char *domain_name)
{
    struct sockaddr_in mail_server; //this struct contains ip address and a port of the server.
    struct hostent *mail_info;      //this struct contains all the info of a host name in the Internet.

    //getting all the information about gmail
    char *mail_ip;
    //gethostbyname() returns strcut hostent contains all the info about the host name argument
    mail_info = gethostbyname(domain_name);
    if (mail_info == NULL)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }

    mail_ip = (char *)malloc(INET_ADDRSTRLEN + 1);
    inet_ntop(AF_INET, mail_info->h_addr_list[0], mail_ip, INET_ADDRSTRLEN);
    //printf("server IP:%s\n", mail_ip);
    free(mail_ip);
    //printf("server Name:%s\n", (char *)mail_info->h_name);

    //filling struct sockaddr_in gmail_server in the ip and the port of the server (from struct hostent* gmail_info)
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

/* после ОТПРАВКИ rename()
                        char *mail_old_full_name = malloc(strlen(user_dir_new) + strlen(new_entry->d_name));
                        strcpy(mail_old_full_name, user_dir_new);
                        strcat(mail_old_full_name, new_entry->d_name);

                        printf("NEW ENTRY %s\n", mail_old_full_name);

                        char *mail_new_name = str_replace(mail_old_full_name, "new", "cur");
                        printf("ENTRY NEW NAME%s\n", mail_new_name);

                        int ret;
                        ret = rename(mail_old_full_name, mail_new_name);
                        if (ret == 0)
                            printf("File renamed successfully\n");
                        else
                            printf("Error: unable to rename the file\n");
                            */