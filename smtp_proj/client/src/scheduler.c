#include "../include/scheduler.h"

struct mail_domain_dscrptr mail_domains_dscrptrs[60];
int ready_domains_count = 0;

//Названия доменов, по которым есть новая почта
char *new_mails_domains[60];

int run_client_async()
{
    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    struct timeval tv;
    int retval;
    /* Watch stdin (fd 0) to see when it has input. */

    FD_ZERO(&read_fds);
    //FD_SET(STDIN_FILENO, read_fds);
    //FD_SET(server->socket, read_fds);

    FD_ZERO(&write_fds);
    // there is smth to send, set up write_fd for server socket
    // if (server->send_buffer.current > 0)
    //     FD_SET(server->socket, write_fds);

    FD_ZERO(&except_fds);
    //FD_SET(STDIN_FILENO, except_fds);
    //FD_SET(server->socket, except_fds);

    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int maxfd = 50;
    while (1)
    {
        // Select() updates fd_set's, so we need to build fd_set's before each select()call.
        //build_fd_sets(&server, &read_fds, &write_fds, &except_fds);

        int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
        switch (activity)
        {
        case -1:
            perror("select()");
            shutdown_properly(EXIT_FAILURE);

        case 0:
            // you should never get here
            printf("select() returns 0.\n");
            shutdown_properly(EXIT_FAILURE);

        default:
            /* All fd_set's should be checked. */
            // if (FD_ISSET(STDIN_FILENO, &read_fds))
            // {
            //     if (handle_read_from_stdin(&server, client_name) != 0)
            //         shutdown_properly(EXIT_FAILURE);
            // }

            // if (FD_ISSET(STDIN_FILENO, &except_fds))
            // {
            //     printf("except_fds for stdin.\n");
            //     shutdown_properly(EXIT_FAILURE);
            // }

            // if (FD_ISSET(server.socket, &read_fds))
            // {
            //     if (receive_from_peer(&server, &handle_received_message) != 0)
            //         shutdown_properly(EXIT_FAILURE);
            // }

            // if (FD_ISSET(server.socket, &write_fds))
            // {
            //     if (send_to_peer(&server) != 0)
            //         shutdown_properly(EXIT_FAILURE);
            // }

            // if (FD_ISSET(server.socket, &except_fds))
            // {
            //     printf("except_fds for server.\n");
            //     shutdown_properly(EXIT_FAILURE);
            // }
        }
    }
}

int run_client()
{
    //1. создаем два дочерних процесса
    //2. в каждом процессе свой while с select
    //3. чекаем каталоги и спихиваем по round robin в процессы
    //4. каждый процесс обновляет свои массивы дескрипторов и работает

    //в родительском процессе необходимо иметь дескрипторы дочерних процессов,
    //которые будут содержать число почтовых доменов и переданных писем в каждый дочерний процесс

    //todo реализовать функцию по отслеживанию, в какой процесс лучше отправить почтовый домен

    pid_t child_a, child_b;
    void* shmem_child_a = create_shared_memory(128);
    void* shmem_child_b = create_shared_memory(128);

    child_a = fork();
    if (child_a == 0)
    {
        /* Child A code */       
        // Принимаем из род.процесса дескрипторы почтовых доменов
        // При необходимости создаем сокет
        // Шарим по каталогам и читаем письма
        // Отправляем письма

        char* mail_dom = shmem_child_b;
    }
    else
    {
        child_b = fork();
        if (child_b == 0)
        {
            /* Child B code */
            // Принимаем из род.процесса дескрипторы почтовых доменов
            // При необходимости создаем сокет
            // Шарим по каталогам и читаем письма
            // Отправляем письма

            char* mail_dom = shmem_child_b;
            
        }
        else
        {
            /* Parent Code */
            int new_mails_domains_count = 10;
            for (int i = 0; i < new_mails_domains_count; i++)
            {
                memcpy(shmem_child_a, new_mails_domains[i], sizeof(new_mails_domains[i]));
            }

            while (1)
            {
                printf("\n########### NEW PERIOD ############## \n");
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
                    char *res = get_domain_mx_server_name(mail_domains[i]);
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

                        read_fd_line(cur_domain_socket_fd, buf, MAX_BUF_LEN);
                        check_server_response_code(buf);
                        printf("%s\n", buf);
                        get_suffix(buf);
                    }
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

                if (domains_count > 0)
                    process_output_mails();
                sleep(120);
            }
        }
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
                        //free(tokens);

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