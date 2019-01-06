#include "../include/scheduler.h"

int ready_domains_count = 0; // юзается в чайлд процессах

int run_client()
{
    // Дескрипторы дочерних процессов - содержат число почтовых доменов и переданных писем в каждый дочерний процесс
    struct mail_process_dscrptr mail_procs[PROC_NUM];
    mail_procs[0].pid = fork();
    if (mail_procs[0].pid == 0)
        child_process_worker_start(6);
    else
    {
        mail_procs[1].pid = fork();
        if (mail_procs[1].pid == 0)
            child_process_worker_start(7);
        else
            master_process_worker_start(mail_procs);
    }

    return 1;
}

// Содержит бизнес логику, обрабатываемую главным процессом
int master_process_worker_start(struct mail_process_dscrptr *mail_procs)
{
    log_i("%s", "Worker for master proc successfully started");
    struct domain_mails domains_mails[MAX_MAIL_DOMAIN_NUM * 2];
    int domains_count = 0;

    key_t key = ftok("/tmp", 6);
    mail_procs[0].msg_queue_id = msgget(key, 0666 | IPC_CREAT);

    key = ftok("/tmp", 7);
    mail_procs[1].msg_queue_id = msgget(key, 0666 | IPC_CREAT);
    while (1)
    {
        domains_count = get_domains_mails(domains_mails, domains_count);
        for (int i = 0; i < domains_count; i++)
        {
            // проверяем, содержится ли домен в одном из процессов
            for (int j = 0; j < domains_mails[i].mails_count; j++)
            {
                //TODO реализовать функцию по отслеживанию, в какой процесс лучше отправить почтовый домен
                //чекаем, в каком из процессов может находиться домен.
                //если нашли, то скидываем в соответствующую очередь
                //если нет, то скидываем туда, где меньше всего нагрузка
                struct queue_msg new_msg;
                new_msg.mtype = 1;
                strcpy(new_msg.mtext, domains_mails[i].mails_paths[j]);
                msgsnd(mail_procs[1].msg_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);

                //printf("SENDING %s\n", domains_mails[i].mails_paths[j]);
            }

            domains_mails[i].mails_count = 0;
            memset(&domains_mails[i].mails_paths[0], 0, sizeof(domains_mails[i].mails_paths));
        }

        wait_for(30);
    }
    return 1;
}

// Содержит бизнес логику, обрабатываемую дочерним процессом
int child_process_worker_start(int proc_idx)
{
    log_i("Worker for child proc `%d' successfully started.", getpid());
    struct mail_domain_dscrptr mail_domains_dscrptrs[MAX_MAIL_DOMAIN_NUM];

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
                register_new_email(cur_msg.mtext, mail_domains_dscrptrs, &read_fds, &write_fds, &except_fds);
        }

        int maxfd = 0;
        for (int i = 0; i < ready_domains_count; i++)
        {
            struct mail_domain_dscrptr cur_domain = mail_domains_dscrptrs[i];
            printf("DOMAIN STATUS %d \n", cur_domain.state);
            if (cur_domain.socket_fd > maxfd)
                maxfd = cur_domain.socket_fd;
        }

        // Проверяем, есть ли в каждом из доменов необработанные письма
        for (int i = 0; i < ready_domains_count; i++)
        {
            struct mail_domain_dscrptr *cur_domain = &mail_domains_dscrptrs[i];
            if (count(cur_domain->mails_list) > 0)
                process_mail_domain(maxfd, cur_domain, &read_fds, &write_fds, &except_fds);
        }

        wait_for(5);
    }

    return 1;
}

// Обновляет массив с описаниями зарегистрированных почтовых доменов
// Каждый элемент содержит название домена, число новых писем для него и пути к письмам
int get_domains_mails(struct domain_mails *domains_mails, int domains_count)
{
    struct dirent *user_dir;
    DIR *mail_dir = opendir(conf.mail_dir);
    if (mail_dir == NULL)
        log_e("%s", "Could not open MAIL directory");

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
                log_e("%s", "Could not open NEW directory");

            int mails_count = count_dir_entries(user_dir_new);
            if (mails_count > 0)
            {
                log_i("Directory %s is not empty", user_dir_new);
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

                        free(email_full_name);
                        free(tmp_cur_mail_domain);
                        free(tokens);
                    }
                }
            }
            else
                log_i("Directory %s is empty", user_dir_new);

            free(user_dir_full_path);
            free(user_dir_new);
            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
}

// Регистрирует новое письмо в массиве дескрипторов mail_domains_dscrptrs для последующей обработки
int register_new_email(char *email_path, struct mail_domain_dscrptr *mail_domains_dscrptrs,
                       fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
    log_i("Registering new email %s", email_path);
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
            log_e("Could not create socket to %s", cur_email_domain);
            return -1;
        }

        if (connect(cur_domain_socket_fd, (struct sockaddr *)&cur_domain_srv, sizeof(cur_domain_srv)) < 0)
        {
            log_e("Connection to %s Failed ", cur_email_domain);
            return -1;
        }
        else
        {
            fcntl(cur_domain_socket_fd, F_SETFL, O_NONBLOCK);
            log_i("Successfully connected to %s ", cur_email_domain);
            log_i("Socket fd : %d", cur_domain_socket_fd);
            mail_domains_dscrptrs[ready_domains_count].socket_fd = cur_domain_socket_fd;

            FD_SET(cur_domain_socket_fd, read_fds);
            FD_SET(cur_domain_socket_fd, write_fds);
            FD_SET(cur_domain_socket_fd, except_fds);
        }

        mail_domains_dscrptrs[ready_domains_count].mails_list = malloc(sizeof(node_t));
        mail_domains_dscrptrs[ready_domains_count].mails_list->next = NULL;

        add_first(&mail_domains_dscrptrs[ready_domains_count].mails_list, saved_email_path);
        mail_domains_dscrptrs[ready_domains_count].state = READY;
        log_i("Mail %s for %s domain successfully added to process queue", saved_email_path, cur_email_domain);
        log_i("List count %d \n", count(mail_domains_dscrptrs[ready_domains_count].mails_list));
        ready_domains_count++;
        return cur_domain_socket_fd;
    }
    // Домен уже зарегистрирован
    else
    {
        log_i("Socket for mail domain %s already binded", cur_email_domain);
        add_first(&mail_domains_dscrptrs[found_domain_num].mails_list, saved_email_path);
        log_i("Mail %s for %s domain successfully added to process queue", saved_email_path, cur_email_domain);
        log_i("List count %d \n", count(mail_domains_dscrptrs[found_domain_num].mails_list));
        return mail_domains_dscrptrs[ready_domains_count - 1].socket_fd;
    }

    free(saved_email_path);
    free(tmp_cur_mail_domain);
}

// Выполняет обработку нового письма домена
void process_mail_domain(int maxfd, struct mail_domain_dscrptr *cur_mail_domain,
                         fd_set *read_fds, fd_set *write_fds, fd_set *except_fds)
{
    int activity = select(maxfd + 1, read_fds, write_fds, except_fds, NULL);
    switch (activity)
    {
    case -1:
        log_e("%s", "select() returned -1");
        shutdown_properly(EXIT_FAILURE);

    case 0:
        log_e("%s", "select() returned 0");
        shutdown_properly(EXIT_FAILURE);

    default:
        if (FD_ISSET(cur_mail_domain->socket_fd, read_fds))
        {
            log_i("Socket %d of %s domain is in read_fds", cur_mail_domain->socket_fd, cur_mail_domain->domain);
            handle_read_socket(cur_mail_domain);
        }

        if (FD_ISSET(cur_mail_domain->socket_fd, write_fds))
        {
            log_i("Socket %d of %s domain is in write_fds", cur_mail_domain->socket_fd, cur_mail_domain->domain);
            handle_write_socket(cur_mail_domain);
        }

        if (FD_ISSET(cur_mail_domain->socket_fd, except_fds))
        {
            log_i("Socket %d of %s domain is in except_fds", cur_mail_domain->socket_fd, cur_mail_domain->domain);
            shutdown_properly(EXIT_FAILURE);
        }
    }
}

// Обрабатывает почтовый домен в случае, когда его сокет находится в write_fds
void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain)
{
    switch (cur_mail_domain->state)
    {
    case READY:
        printf("READY WRITE_FDS \n");
        //char *msg = read_msg_file((*cur_mail_domain.mails_list).val);
        //char *email_new_name = str_replace((*cur_mail_domain->mails_list).val, "new", "cur");
        //printf("ENTRY NEW NAME %s\n", email_new_name);
        cur_mail_domain->state = MAIL_FROM_MSG;

        // int ret = rename((*cur_mail_domain.mails_list).val, email_new_name);
        // if (ret == 0)
        //     printf("File renamed successfully\n");
        // else
        //     printf("Error: unable to rename the file\n");

        // remove_first(&cur_mail_domain.mails_list);
        //send_helo(cur_mail_domain.socket_fd);
        break;

    case MAIL_FROM_MSG:
        printf("MAIL_FROM_MSG WRITE_FDS \n");
        //send_mail_from(cur_mail_domain.socket_fd, cur_mail_domain.buffer);
        break;

    case RCPT_TO_MSG:
        printf("RCPT_TO_MSG WRITE_FDS \n");
        //send_rcpt_to(cur_mail_domain.socket_fd, cur_mail_domain.buffer);
        break;

    case DATA_MSG:
        printf("DATA_MSG WRITE_FDS \n");
        //send_data_msg(cur_mail_domain.socket_fd);
        break;

    case HEADERS_MSG:
        printf("HEADERS_MSG WRITE_FDS \n");
        //send_headers(cur_mail_domain.socket_fd);
        break;

    case BODY_MSG:
        printf("BODY_MSG WRITE_FDS \n");
        //send_msg_body(cur_mail_domain.socket_fd);
        break;

    default:
        printf("DEFAULT WRITE_FDS \n");
        //send_quit(cur_mail_domain.socket_fd);
        break;
    }
}

// Обрабатывает почтовый домен в случае, когда его сокет находится в read_fds
void handle_read_socket(struct mail_domain_dscrptr *cur_mail_domain)
{
    switch (cur_mail_domain->state)
    {
    case READY:
        log_i("%s", "READY READ_FDS \n");
        //read response
        cur_mail_domain->state = MAIL_FROM_MSG;
        break;

    case MAIL_FROM_MSG:
        printf("MAIL_FROM_MSG READ_FDS \n");
        break;

    case RCPT_TO_MSG:
        printf("RCPT_TO_MSG READ_FDS \n");
        break;

    case DATA_MSG:
        printf("DATA_MSG READ_FDS \n");
        break;

    case HEADERS_MSG:
        printf("HEADERS_MSG READ_FDS \n");
        break;

    case BODY_MSG:
        printf("BODY_MSG READ_FDS \n");
        break;

    default:
        printf("DEFAULT READ_FDS \n");
        break;
    }
}

// Ожидает заданное число секунд
void wait_for(unsigned int secs)
{
    unsigned int retTime = time(0) + secs;
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
// printf("Data Received is : %s \n", cur_msg.mtext);
