#include "../include/scheduler.h"
#include <time.h>

#define PROC_NUM 2

//в родительском процессе необходимо иметь дескрипторы дочерних процессов,
//которые будут содержать число почтовых доменов и переданных писем в каждый дочерний процесс

//todo реализовать функцию по отслеживанию, в какой процесс лучше отправить почтовый домен
//autofsm для smtp протокола

struct mail_domain_dscrptr mail_domains_dscrptrs[60];
struct domain_mails domains_mails[60];
int domains_count = 0;
int ready_domains_count = 0;
struct mail_process_dscrptr mail_procs[PROC_NUM];

int run_client()
{
    mail_procs[0].pid = fork();
    if (mail_procs[0].pid == 0)
        child_process_worker_start(6);
    else
    {
        mail_procs[1].pid = fork();
        if (mail_procs[1].pid == 0)
            child_process_worker_start(7);
        else
            master_process_worker_start();
    }

    return 1;
}

// Содержит бизнес логику, обрабатываемую главным процессом
int master_process_worker_start()
{
    key_t key = ftok("/tmp", 6);
    mail_procs[0].msg_queue_id = msgget(key, 0666 | IPC_CREAT);

    key = ftok("/tmp", 7);
    mail_procs[1].msg_queue_id = msgget(key, 0666 | IPC_CREAT);

    while (1)
    {
        domains_count = get_domains_mails(domains_mails);
        for (int i = 0; i < domains_count; i++)
        {
            for (int j = 0; j < domains_mails[i].mails_count; j++)
            {
                struct queue_msg new_msg;
                new_msg.mtype = 1;
                strcpy(new_msg.mtext, domains_mails[i].mails_paths[j]);
                msgsnd(mail_procs[1].msg_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);

                //printf("SENDING %s\n", domains_mails[i].mails_paths[j]);
                //fflush(stdout);
            }

            domains_mails[i].mails_count = 0;
            memset(&domains_mails[i].mails_paths[0], 0, sizeof(domains_mails[i].mails_paths));
        }

        waitFor(10);
    }
    return 1;
}

// Содержит бизнес логику, обрабатываемую отдельным процессом
int child_process_worker_start(int proc_idx)
{
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    key_t key = ftok("/tmp", proc_idx);
    int cur_proc_mq_id = msgget(key, 0644);
    struct queue_msg cur_msg;
    while (1)
    {
        if (msgrcv(cur_proc_mq_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
        {
            if (strlen(cur_msg.mtext) != 0)
            {
                process_email(cur_msg.mtext);
                // int maxfd = 100;
                // int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
                // switch (activity)
                // {
                // case -1:
                //     perror("select()");
                //     shutdown_properly(EXIT_FAILURE);

                // case 0:
                //     printf("select() returns 0.\n");
                //     shutdown_properly(EXIT_FAILURE);

                // default:
                //     /* All fd_set's should be checked. */
                //     // if (FD_ISSET(STDIN_FILENO, &read_fds))
                //     // {
                //     //     if (handle_read_from_stdin(&server, client_name) != 0)
                //     //         shutdown_properly(EXIT_FAILURE);
                //     // }

                //     if (FD_ISSET(STDIN_FILENO, &except_fds))
                //     {
                //         printf("except_fds for stdin.\n");
                //         shutdown_properly(EXIT_FAILURE);
                //     }

                //     // if (FD_ISSET(server.socket, &read_fds))
                //     // {
                //     //     if (receive_from_peer(&server, &handle_received_message) != 0)
                //     //         shutdown_properly(EXIT_FAILURE);
                //     // }

                //     // if (FD_ISSET(server.socket, &write_fds))
                //     // {
                //     //     if (send_to_peer(&server) != 0)
                //     //         shutdown_properly(EXIT_FAILURE);
                //     // }

                //     // if (FD_ISSET(server.socket, &except_fds))
                //     // {
                //     //     printf("except_fds for server.\n");
                //     //     shutdown_properly(EXIT_FAILURE);
                //     // }
                // }
            }
        }
    }

    return 1;
}

// Обновляет массив с описаниями зарегистрированных почтовых доменов
// Каждый элемент содержит название домена, число новых писем для него и пути к письмам
int get_domains_mails(struct domain_mails *domains_mails)
{
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
                //printf("directory is NOT EMPTY\n");
                struct dirent *new_entry;
                while ((new_entry = readdir(new_dir)) != NULL)
                {
                    if (strcmp(new_entry->d_name, ".") != 0 && strcmp(new_entry->d_name, "..") != 0)
                    {
                        char *email_full_name = malloc(strlen(user_dir_new) + strlen(new_entry->d_name));
                        strcpy(email_full_name, user_dir_new);
                        strcat(email_full_name, new_entry->d_name);

                        char **tokens;
                        tokens = str_split(new_entry->d_name, '.');

                        char *first_part = tokens[2];
                        char *second_part = tokens[3];

                        char *tmp_cur_mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
                        strcpy(tmp_cur_mail_domain, first_part);
                        strcat(tmp_cur_mail_domain, ".");
                        strcat(tmp_cur_mail_domain, second_part);

                        tokens = str_split(tmp_cur_mail_domain, ',');
                        //Проверяем, есть ли текущий домен массиве доменов
                        int found_domain_num = -1;
                        for (int i = 0; i < domains_count; i++)
                        {
                            if (strcmp(tokens[0], domains_mails[i].domain) == 0)
                                found_domain_num = i;
                        }

                        //Домен не найден - добавляем в массив
                        if (found_domain_num < 0)
                        {
                            domains_mails[domains_count].domain = malloc(strlen(tokens[0]));
                            strcpy(domains_mails[domains_count].domain, tokens[0]);

                            domains_mails[found_domain_num].mails_count = 0;
                            domains_mails[domains_count].mails_paths[0] = malloc(strlen(email_full_name));
                            strcpy(domains_mails[domains_count].mails_paths[0], email_full_name);

                            domains_mails[domains_count].mails_count++;
                            domains_count++;
                        }
                        //Домен найден - обновляем его состояние
                        else
                        {
                            //Проверяем, есть ли текущее письмо в очереди обработки
                            int found_mail_num = -1;
                            for (int k = 0; k < domains_mails[found_domain_num].mails_count; k++)
                            {
                                if (strcmp(email_full_name, domains_mails[found_domain_num].mails_paths[k]) == 0)
                                    found_mail_num = k;
                            }

                            if (found_mail_num < 0)
                            {
                                int last_mail_num = domains_mails[found_domain_num].mails_count;
                                domains_mails[found_domain_num].mails_paths[last_mail_num] = malloc(strlen(email_full_name));
                                strcpy(domains_mails[found_domain_num].mails_paths[last_mail_num], email_full_name);

                                domains_mails[found_domain_num].mails_count++;
                            }
                        }
                        free(tokens);
                    }
                }
            }
            //else
            //printf("directory is EMPTY\n");

            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
}

int process_email(char *email_path)
{
    printf("EMAIL %s\n", email_path);
    char *saved_email_path = malloc(strlen(email_path));
    strcpy(saved_email_path, email_path);

    char **tokens;
    tokens = str_split(email_path, '.');

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

    //Проверяем, является ли домен письма новым
    //Если да - создаем элемент массива дескриптора биндим сокет
    //Если нет, считываем файл
    int found_domain_num = -1;
    for (int i = 0; i < ready_domains_count; i++)
    {
        if (strcmp(cur_email_domain, mail_domains_dscrptrs[i].domain) == 0)
            found_domain_num = i;
    }

    //Домен не найден - добавляем в массив и биндим новый сокет
    if (found_domain_num < 0)
    {
        printf("EMAIL NOT EXIST %s\n", cur_email_domain);
        mail_domains_dscrptrs[ready_domains_count].domain = malloc(strlen(cur_email_domain));
        strcpy(mail_domains_dscrptrs[ready_domains_count].domain, cur_email_domain);

        char *res = get_domain_mx_server_name(cur_email_domain);
        struct sockaddr_in cur_domain_srv = get_domain_server_info(res);
        cur_domain_srv.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
        cur_domain_srv.sin_port = htons(25); //port 25=SMTP.

        mail_domains_dscrptrs[ready_domains_count].domain_mail_server = cur_domain_srv;
        int cur_domain_socket_fd = 0;
        if ((cur_domain_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            //printf("\n Error : Could not create socket \n");
            return -1;
        }

        if (connect(cur_domain_socket_fd, (struct sockaddr *)&cur_domain_srv, sizeof(cur_domain_srv)) < 0)
        {
            //printf("\n Error : Connect Failed \n");
            return -1;
        }
        else
        {
            printf("\n SUCCESS : Connected \n");
            //fflush(stdout);
            mail_domains_dscrptrs[ready_domains_count].socket_fd = cur_domain_socket_fd;
        }

        ready_domains_count++;
    }
    else
    {
        printf("EMAIL EXISTs %s\n", cur_email_domain);
    }

    // for (int i = 0; i < ready_domains_count; i++)
    // {
    //     if (strcmp(mail_domains_dscrptrs[i].domain, cur_email_domain) == 0)
    //     {
    //         printf("EMAIL DOMAIN %s\n", cur_email_domain);
    //         fflush(stdout);
    //         //printf("SENDING DATA . . . \n");
    //         //send_msg_to_server(mail_domains_dscrptrs[i].socket_fd, email_msg);
    //         char *email_new_name = str_replace(saved_email_path, "new", "cur");
    //         printf("ENTRY NEW NAME %s\n", email_new_name);
    //         fflush(stdout);

    //         // int ret;
    //         // ret = rename(email_full_names[j], email_new_name);
    //         // if (ret == 0)
    //         //     printf("File renamed successfully\n");
    //         // else
    //         //     printf("Error: unable to rename the file\n");

    //         break;
    //     }
    // }

    return 1;
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

                //Обрабатываем каждое письмо - чтение в память, отправка и rename()
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
                            printf("EMAIL DOMAIN %s\n", cur_email_domain);
                            printf("EMAIL NAME %s\n", email_short_names[j]);
                            char *email_msg = read_msg_file(email_full_names[j]);
                            printf("EMAIL FILE MSG %s\n", email_msg);

                            printf("SENDING DATA . . . \n");
                            send_msg_to_server(mail_domains_dscrptrs[i].socket_fd, email_msg);

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

void waitFor(unsigned int secs)
{
    unsigned int retTime = time(0) + secs; // Get finishing time.
    while (time(0) < retTime)
        ; // Loop until it arrives.
}

void shutdown_properly(int code)
{
    //   delete_peer(&server);
    printf("Shutdown client properly.\n");
    exit(code);
}

//printf("MQ id : %d \n", cur_proc_mq_id);
//fflush(stdout);

//printf("SON MSQ queue %d\n\n", cur_proc_mq_id);
//fflush(stdout);

//printf("MASTER MQ id : %d \n", mail_procs[1].msg_queue_id);
//fflush(stdout);

//printf("MASTER MQ id : %d \n", domains_mails[i].mails_count);
//printf("[son] pid %d from [parent] pid %d\n", getpid(), getppid());

//     printf("Data Received is : %s \n", cur_msg.mtext);
// fflush(stdout);
