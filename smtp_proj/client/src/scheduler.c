#include "../include/scheduler.h"

#define MAX_FD_CNT 1024
#define MAX_Q_LEN 10
#define MAX_BUF_LEN 1024

int run_client()
{
    while (1)
    {
        printf(" ########### NEW PERIOD ############## \n");
        //Check mail domains' folder for new messages
        struct MapItem mail_domains_msgs[3];
        int mail_domains_count = get_output_mails(mail_domains_msgs);

        printf("domains Mails count %d\n", mail_domains_count);

        for (int i = 0; i < mail_domains_count; i++)
        {
            printf(" ------------------------------- \n");
            printf("Mail domain %s\n", mail_domains_msgs[i].domain);
            printf("Mails count %d\n", mail_domains_msgs[i].values_count);

            get_domain_server_info(mail_domains_msgs[i].domain);
            //bind socket for cur server
            //save fd of sockets and run process
        }
        sleep(5);
    }
    return 1;
}

int get_output_mails(struct MapItem *items)
{
    int items_count = 0;

    const char *output_mails_dir = "/out";
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

//todo вынести в другой файл по работе с каталогами
int count_dir_entries(const char *dirname)
{
    printf("opening %s\n", dirname);
    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(dirname);
    if (dir == NULL)
    {
        printf("NULL dir\n");
        return 0;
    }

    while ((d = readdir(dir)) != NULL)
    {
        if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
            n++;
    }
    closedir(dir);
    return n;
}

void get_domain_server_info(char *domain_name)
{
    //printf("domain name  %s\n", domain_name);

    int host_num = 0;
    for (int i = 0; i < 3; i++)
    {
        if (strcmp(conf.mail_domains[i].domain, domain_name) == 0)
        {
            host_num = i;
            printf("SMTP_Server_Host_Name %s\n", conf.mail_domains[i].server_host_name);
            break;
        }
    }

    struct sockaddr_in gmail_server; //this struct contains ip address and a port of the server.
    struct hostent *gmail_info;      //this struct contains all the info of a host name in the Internet.

    //getting all the information about gmail
    char *gmail_ip;
    //gethostbyname() returns strcut hostent contains all the info about the host name argument
    gmail_info = gethostbyname(conf.mail_domains[host_num].server_host_name);
    if (gmail_info == NULL)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }

    gmail_ip = (char *)malloc(INET_ADDRSTRLEN + 1);
    inet_ntop(AF_INET, gmail_info->h_addr_list[0], gmail_ip, INET_ADDRSTRLEN);
    printf("server IP:%s\n", gmail_ip);
    free(gmail_ip);
    printf("server Name:%s\n", (char *)gmail_info->h_name);

    //filling struct sockaddr_in gmail_server in the ip and the port of the server (from struct hostent* gmail_info)
    bzero(&gmail_server, sizeof(gmail_server));
    gmail_server.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
    gmail_server.sin_port = htons(25); //port 25=SMTP.
    bcopy((char *)gmail_info->h_addr_list[0], (char *)&gmail_server.sin_addr.s_addr, gmail_info->h_length);
}
