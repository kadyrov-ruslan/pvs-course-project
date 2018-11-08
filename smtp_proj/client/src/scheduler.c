#include "../include/client_types.h"
#include "../include/scheduler.h"
#include "../../common/include/map.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h> //Internet address family
#include <netdb.h>      //definitions for network database operations
#include <arpa/inet.h>  //definitions for internet operations
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>

#define MAX_FD_CNT 1024
#define MAX_Q_LEN 10
#define MAX_BUF_LEN 1024

//static char ip_def[32] = "127.0.0.1";

int run_client()
{
    //Check mail domains' folder for new messages
    struct MapItem mail_domains_msgs[10];
    int mail_domains_count = get_output_mails(mail_domains_msgs);
    for (int i = 0; i < mail_domains_count; i++)
    {
        printf("Mail domain %s\n", mail_domains_msgs[i].key);
        printf("Mails count %d\n", mail_domains_msgs[i].values_count);
    }

    get_server_info("smtp.gmail.com");
    return 1;
}

int get_output_mails(struct MapItem *items)
{
    //static struct MapItem items[10];
    int items_count = 0;

    const char *output_mails_dir = "/out";
    char *full_out_maildir = malloc(strlen(conf.mail_dir) + 1 + 4);
    strcpy(full_out_maildir, conf.mail_dir);
    strcat(full_out_maildir, output_mails_dir);

    // Pointer for directory entry
    struct dirent *de;
    DIR *dir = opendir(full_out_maildir);
    if (dir == NULL)
        printf("Could not open current directory");

    while ((de = readdir(dir)) != NULL)
    {
        char *cur_domain_dir = de->d_name;
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            char *full_domain_dir = malloc(strlen(full_out_maildir) + 5 + strlen(cur_domain_dir));
            strcpy(full_domain_dir, full_out_maildir);
            strcat(full_domain_dir, "/");
            strcat(full_domain_dir, cur_domain_dir);
            //printf("full domain path %s\n", full_domain_dir);

            int mails_count = countEntriesInDir(full_domain_dir);
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
                        {
                            printf("error %s\n", strerror(errno));
                        }
                        else
                        {
                            //printf("fd %d\n", fd);
                            mails_fd[cur_domain_mails_count] = fd;
                            cur_domain_mails_count = cur_domain_mails_count + 1;
                        }
                    }
                }
                closedir(dir);

                items[items_count].key = full_domain_dir;
                items[items_count].values = mails_fd;
                items[items_count].values_count = cur_domain_mails_count;
                items_count = items_count + 1;
            }
            else
                printf("directory is EMPTY\n");
        }
    }

    closedir(dir);
    return items_count;
}

//todo вынести в другой файл по работе с каталогами
int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

//todo вынести в другой файл по работе с каталогами
int countEntriesInDir(const char *dirname)
{
    printf("opening%s\n", dirname);
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

void get_server_info(char *SMTP_Server_Host_Name)
{
    struct sockaddr_in gmail_server; //this struct contains ip address and a port of the server.
    struct hostent *gmail_info;      //this struct contains all the info of a host name in the Internet.

    //getting all the information about gmail
    char *gmail_ip;
    //gethostbyname() returns strcut hostent contains all the info about the host name argument
    gmail_info = gethostbyname(SMTP_Server_Host_Name);
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
