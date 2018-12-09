#include "../include/scheduler.h"

#define PROC_NUM 2

struct mail_domain_dscrptr mail_domains_dscrptrs[60];
struct domain_mails domains_mails[60];
int domains_count = 0;
int ready_domains_count = 0;
struct mail_process_dscrptr mail_procs[PROC_NUM];

//Названия доменов, по которым есть новая почта

// int run_client_async()
// {
//     fd_set read_fds;
//     fd_set write_fds;
//     fd_set except_fds;

//     struct timeval tv;
//     int retval;
//     /* Watch stdin (fd 0) to see when it has input. */

//     FD_ZERO(&read_fds);
//     //FD_SET(STDIN_FILENO, read_fds);
//     //FD_SET(server->socket, read_fds);

//     FD_ZERO(&write_fds);
//     // there is smth to send, set up write_fd for server socket
//     // if (server->send_buffer.current > 0)
//     //     FD_SET(server->socket, write_fds);

//     FD_ZERO(&except_fds);
//     //FD_SET(STDIN_FILENO, except_fds);
//     //FD_SET(server->socket, except_fds);

//     /* Wait up to five seconds. */
//     tv.tv_sec = 5;
//     tv.tv_usec = 0;

//     int maxfd = 50;
//     while (1)
//     {
//         // Select() updates fd_set's, so we need to build fd_set's before each select()call.
//         //build_fd_sets(&server, &read_fds, &write_fds, &except_fds);

//         int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
//         switch (activity)
//         {
//         case -1:
//             perror("select()");
//             shutdown_properly(EXIT_FAILURE);

//         case 0:
//             // you should never get here
//             printf("select() returns 0.\n");
//             shutdown_properly(EXIT_FAILURE);

//         default:
//             /* All fd_set's should be checked. */
//             // if (FD_ISSET(STDIN_FILENO, &read_fds))
//             // {
//             //     if (handle_read_from_stdin(&server, client_name) != 0)
//             //         shutdown_properly(EXIT_FAILURE);
//             // }

//             // if (FD_ISSET(STDIN_FILENO, &except_fds))
//             // {
//             //     printf("except_fds for stdin.\n");
//             //     shutdown_properly(EXIT_FAILURE);
//             // }

//             // if (FD_ISSET(server.socket, &read_fds))
//             // {
//             //     if (receive_from_peer(&server, &handle_received_message) != 0)
//             //         shutdown_properly(EXIT_FAILURE);
//             // }

//             // if (FD_ISSET(server.socket, &write_fds))
//             // {
//             //     if (send_to_peer(&server) != 0)
//             //         shutdown_properly(EXIT_FAILURE);
//             // }

//             // if (FD_ISSET(server.socket, &except_fds))
//             // {
//             //     printf("except_fds for server.\n");
//             //     shutdown_properly(EXIT_FAILURE);
//             // }
//         }
//     }
// }

int run_client()
{
    //1. создаем два дочерних процесса
    //2. в каждом процессе свой while с select
    //3. чекаем каталоги и спихиваем по round robin в процессы
    //4. каждый процесс обновляет свои массивы дескрипторов и работает

    //в родительском процессе необходимо иметь дескрипторы дочерних процессов,
    //которые будут содержать число почтовых доменов и переданных писем в каждый дочерний процесс

    //todo реализовать функцию по отслеживанию, в какой процесс лучше отправить почтовый домен
    //autofsm для smtp протокола

    mail_procs[0].pid = fork();
    if (mail_procs[0].pid == 0)
        process_worker_start(1);
    else
    {
        mail_procs[1].pid = fork();
        if (mail_procs[1].pid == 0)
            process_worker_start(2);
        else
        {
            /* Parent Code */
            key_t key = ftok("/tmp", 1);
            mail_procs[0].msg_queue_id = msgget(key, 0666 | IPC_CREAT);

            key = ftok("/tmp", 2);
            mail_procs[1].msg_queue_id = msgget(key, 0666 | IPC_CREAT);
            while (1)
            {
                domains_count = get_domains_mails(domains_mails);
                printf("WHOLE domains count %d\n", domains_count);
                for (int i = 0; i < domains_count; i++)
                {
                    printf("DOMAIN  %s\n\n", domains_mails[i].domain);
                    for (int j = 0; j < domains_mails[i].mails_count; j++)
                        printf("mail  %s\n", domains_mails[i].mails_paths[j]);
                }

                sleep(15);
            }
        }
    }

    return 1;
}

// Обновляет массив с описаниями зарегистрированных почтовых доменов
// Каждый элемент содержит название домена, число новых писем для него и пути к письмам
int get_domains_mails(struct domain_mails *domains_mails)
{
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

                            printf("NEW domain %s\n", domains_mails[domains_count].domain);

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
            else
                printf("directory is EMPTY\n");

            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
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

// Содержит бизнес логику, обрабатываемую отдельным процессом
int process_worker_start(int proc_idx)
{
    // printf("[son] pid %d from [parent] pid %d\n", getpid(), getppid());
    key_t key = ftok("/tmp", proc_idx);
    int cur_proc_mq_id = msgget(key, 0666 | IPC_CREAT);
    // printf("SON A MSQ queue %d\n\n", cur_proc_mq_id);
    (void)cur_proc_mq_id;
    return 1;
}

// int send_domain_to_process(char **mail_domains)
// {
// }