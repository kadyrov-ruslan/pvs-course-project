#include "scheduler.h"
#include "client_types.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_FD_CNT 1024
#define MAX_Q_LEN 10
#define MAX_BUF_LEN 1024

//static char ip_def[32] = "127.0.0.1";

int run_client()
{
    /*char *ip_addr = ip_def;
    struct sockaddr_out addr;
    uint16_t port = (uint16_t)conf.port;

    //Start process for logging
    int log_pid = start_logger(conf.log_file);
    if (log_pid == 0)
    {
        log_e("%s", "unable to start process for logging");
        return -1;
    }
    else
        log_i("%s", "loging process: started");

    memset(&addr, 0, sizeof(addr));
    addr.sout_family = AF_INET;
    addr.sout_addr.s_addr = inet_addr(ip_addr);
    addr.sout_port = htons(port);*/

    //Check mail domains' folder for new messages
    check_output_mails();
    return 1;
}

void check_output_mails()
{
    const char *output_mails_dir = "/out";
    char *full_out_maildir;
    full_out_maildir = malloc(strlen(conf.mail_dir) + 1 + 4);
    strcpy(full_out_maildir, conf.mail_dir);
    strcat(full_out_maildir, output_mails_dir);

    // Pointer for directory entry
    struct dirent *de;

    DIR *dir = opendir(full_out_maildir);
    if (dir == NULL)
    {
        printf("Could not open current directory");
    }

    while ((de = readdir(dir)) != NULL)
    {
        char *cur_domain_dir = de->d_name;
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0)
        {
            char *domain_dir = malloc(5 + strlen(cur_domain_dir));
            strcpy(domain_dir, "/");
            strcat(domain_dir, cur_domain_dir);

            char *full_domain_dir;
            char *s = full_out_maildir;
            full_domain_dir = malloc(strlen(s) + strlen(domain_dir));
            strcpy(full_domain_dir, s);
            strcat(full_domain_dir, domain_dir);
            printf("full domain path %s\n", full_domain_dir);
            /*
        if (isDirectory(cur_domain_dir))
        {
            printf("IS directory");
            if(countEntriesInDir(cur_domain_dir) > 0)
            {
                printf("IS directory is NOT EMPTY");
                //if(каталог не пустой) создаем элемент словаря
            }
            else
                printf("directory is EMPTY");
        }
*/
            //printf("IS directory");
            if (countEntriesInDir(full_domain_dir) > 0)
            {

                printf("directory is NOT EMPTY\n");
                //if(каталог не пустой) создаем элемент словаря
            }
            else
                printf("directory is EMPTY\n");
        }
    }

    closedir(dir);
}

//todo вынести в другой файл по работе с каталогами
/*static int isDirectory(const char *path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}*/

//todo вынести в другой файл по работе с каталогами//
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
