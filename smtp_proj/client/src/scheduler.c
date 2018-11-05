#include "scheduler.h"
#include "client_types.h"
#include <stdio.h>
#include <dirent.h>

static char ip_def[32] = "127.0.0.1";

int run_client()
{
    char *ip_addr = ip_def;
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
    addr.sout_port = htons(port);

    //Check mail domains' folder for new messages
    check_output_mails();
}

int check_output_mails()
{
    const char *output_mails_dir = "/out";
    char *full_out_maildir;
    name_with_extension = malloc(strlen(name) + 1 + 4);
    strcpy(full_out_maildir, conf.mail_dir);
    strcat(full_out_maildir, output_mails_dir);
    printf("full path %s\n", full_out_maildir);

    // Pointer for directory entry
    struct dirent *de;

    DIR *dir = opendir(full_out_maildir);
    if (dir == NULL)
    {
        printf("Could not open current directory");
        return 0;
    }

    while ((de = readdir(dir)) != NULL)
    {
        char *cur_domain_dir = de->d_name;
        printf("%s\n", cur_domain_dir);
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
    }

    closedir(dr);
    return 0;
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
    int n = 0;
    dirent *d;
    DIR *dir = opendir(dirname);
    if (dir == NULL)
        return 0;
    while ((d = readdir(dir)) != NULL)
        n++;
    closedir(dir);
    return n;
}