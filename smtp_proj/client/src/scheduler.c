#include "../include/scheduler.h"

int ready_domains_count = 0; // юзается в чайлд процессах

int run_client(int proc_num, int total_send_time, int retry_time)
{
    //create_child_proc(0, proc_num);
    struct mail_process_dscrptr mail_procs[proc_num];
    mail_procs[0].pid = fork();
    if (mail_procs[0].pid == -1)
    {
        log_e("Unable to fork() new process from %d", getpid());
        abort();
    }
    else if (mail_procs[0].pid == 0)
        child_process_worker_start(6, total_send_time, retry_time);
    else
    {
        mail_procs[1].pid = fork();
        if (mail_procs[1].pid == -1)
        {
            log_e("Unable to fork() new process from %d", getpid());
            abort();
        }
        else if (mail_procs[1].pid == 0)
            child_process_worker_start(7, total_send_time, retry_time);
        else
            master_process_worker_start(mail_procs, proc_num);
    }

    return 1;
}

void create_child_proc(int idx, int proc_num, int total_send_time, int retry_time)
{
    int fork_res = fork();
    if (fork_res == -1)
    {
        log_e("Unable to fork() new process from %d", getpid());
        abort();
    }
    else if (fork_res == 0)
        child_process_worker_start(idx, total_send_time, retry_time);
    else
    {
        if (idx < proc_num - 1)
        {
            create_child_proc(idx + 1, proc_num, total_send_time, retry_time);
        }
        else
        {
            log_e("Unable to fork() new process from %d", getpid());
            struct mail_process_dscrptr mail_procs[proc_num];
            master_process_worker_start(mail_procs, proc_num);
        }
    }
}

// Содержит бизнес логику, обрабатываемую главным процессом
int master_process_worker_start(struct mail_process_dscrptr *mail_procs, int proc_num)
{
    log_i("%s", "Worker for master proc successfully started");
    struct domain_mails domains_mails[MAX_MAIL_DOMAIN_NUM * 2];
    int domains_count = 0;

    for (int i = 0; i < proc_num; i++)
    {
        key_t key = ftok("/tmp", 6 + i);
        mail_procs[i].msg_queue_id = msgget(key, 0666 | IPC_CREAT);
        mail_procs[i].domains_count = 0;
    }

    while (1)
    {
        domains_count = get_domains_mails(domains_mails, domains_count);
        for (int i = 0; i < domains_count; i++)
        {
            int domain_proc_idx = get_mail_proc_idx(domains_mails[i].domain, domains_count, mail_procs);
            for (int j = 0; j < domains_mails[i].mails_count; j++)
            {
                struct queue_msg new_msg;
                new_msg.mtype = 1;
                strcpy(new_msg.mtext, domains_mails[i].mails_paths[j]);
                free(domains_mails[i].mails_paths[j]);
                msgsnd(mail_procs[domain_proc_idx].msg_queue_id, &new_msg, sizeof(new_msg), IPC_NOWAIT);
            }

            domains_mails[i].mails_count = 0;
            //free(domains_mails[i].mails_paths);
            memset(&domains_mails[i].mails_paths[0], 0, sizeof(domains_mails[i].mails_paths));
        }

        wait_for(RETRY_DIR_READ_TIME);
    }
    return 1;
}

//todo доделать для аргумента proc_num
// Возвращает индекс процесса, в который стоит отправить новое письмо на обработку
int get_mail_proc_idx(char *domain_name, int domains_count, struct mail_process_dscrptr *mail_procs)
{
    for (int j = 0; j < 2; j++)
    {
        for (int i = 0; i < mail_procs[j].domains_count; i++)
        {
            // Если один из процессов уже занимается обработкой конкр.домена, скидываем письмо в него
            if (strcmp(domain_name, mail_procs[j].domains[i]) == 0)
            {
                log_i("Process %d already handles %s domain. Domains count %d", j, domain_name, mail_procs[j].domains_count);
                return j;
            }
        }
    }

    // Домен не найден ни в одном из процессов. Скидываем в процесс с меньшим числом доменов
    if (mail_procs[0].domains_count > mail_procs[1].domains_count)
    {
        mail_procs[1].domains[mail_procs[1].domains_count] = malloc(strlen(domain_name));
        strcpy(mail_procs[1].domains[mail_procs[1].domains_count], domain_name);
        mail_procs[1].domains_count++;
        log_i("Process 1 handles %s domain. Domains count %d", domain_name, mail_procs[1].domains_count);
        return 1;
    }
    else
    {
        mail_procs[0].domains[mail_procs[0].domains_count] = malloc(strlen(domain_name));
        strcpy(mail_procs[0].domains[mail_procs[0].domains_count], domain_name);
        mail_procs[0].domains_count++;
        log_i("Process 0 handles %s domain. Domains count %d", domain_name, mail_procs[1].domains_count);
        return 0;
    }
}

// Содержит бизнес логику, обрабатываемую дочерним процессом
int child_process_worker_start(int proc_idx, int total_send_time, int retry_time)
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
                register_new_email(cur_msg.mtext, mail_domains_dscrptrs, &read_fds, &write_fds, &except_fds, total_send_time, retry_time);
        }

        int maxfd = 0;
        for (int i = 0; i < ready_domains_count; i++)
        {
            struct mail_domain_dscrptr *cur_domain = &mail_domains_dscrptrs[i];
            if (cur_domain->socket_fd > maxfd)
                maxfd = cur_domain->socket_fd;
        }

        // Проверяем, есть ли в каждом из доменов необработанные письма
        for (int i = 0; i < ready_domains_count; i++)
        {
            struct mail_domain_dscrptr *cur_domain = &mail_domains_dscrptrs[i];
            if (count(cur_domain->mails_list) > 0 || cur_domain->state > CLIENT_FSM_ST_INIT)
                process_mail_domain(maxfd, cur_domain, &read_fds, &write_fds, &except_fds);
        }
        // задержка 50 мс - при задержкке цикла < 50мс обработка зависает после BODY_MSG WRITE_FDS
        usleep(100000);
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

                        char **tokens = str_split(new_entry->d_name, '.');
                        char *first_part = tokens[2];
                        char *second_part = tokens[3];

                        char *tmp_cur_mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
                        strcpy(tmp_cur_mail_domain, first_part);
                        strcat(tmp_cur_mail_domain, ".");
                        strcat(tmp_cur_mail_domain, second_part);

                        free(tokens[0]);
                        free(tokens[1]);
                        free(first_part);
                        free(second_part);
                        free(tokens[4]);
                        free(tokens);

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
                        free(tokens[0]);
                        free(tokens[1]);
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
                       fd_set *read_fds, fd_set *write_fds, fd_set *except_fds, int total_send_time, int retry_time)
{
    log_i("Registering new email %s", email_path);
    char *saved_email_path = malloc(strlen(email_path));
    strcpy(saved_email_path, email_path);

    char **tokens = str_split(email_path, '.');
    char *first_part = tokens[2];
    char *second_part = tokens[3];

    char *tmp_cur_mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
    strcpy(tmp_cur_mail_domain, first_part);
    strcat(tmp_cur_mail_domain, ".");
    strcat(tmp_cur_mail_domain, second_part);

    free(tokens[0]);
    free(tokens[1]);
    //free(first_part);
    //free(second_part);
    //free(tokens[4]);
    free(tokens);

    tokens = str_split(tmp_cur_mail_domain, ',');
    free(tmp_cur_mail_domain);
    char *cur_email_domain = tokens[0];
    //free(tokens[0]);
    //free(tokens[1]);
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
        log_i("Email domain %s is not registered \n", cur_email_domain);
        mail_domains_dscrptrs[ready_domains_count].domain = malloc(strlen(cur_email_domain));
        strcpy(mail_domains_dscrptrs[ready_domains_count].domain, cur_email_domain);

        char *res = get_domain_mx_server_name(cur_email_domain);
        struct sockaddr_in cur_domain_srv = get_domain_server_info(res);
        cur_domain_srv.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
        cur_domain_srv.sin_port = htons(25); //port 25=SMTP.

        mail_domains_dscrptrs[ready_domains_count].domain_mail_server = cur_domain_srv;
        mail_domains_dscrptrs[ready_domains_count].total_send_time = total_send_time;
        mail_domains_dscrptrs[ready_domains_count].number_of_attempts = 0;
        mail_domains_dscrptrs[ready_domains_count].last_attempt_time = 0;
        mail_domains_dscrptrs[ready_domains_count].curr_rcpts_index = -1;

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
            mail_domains_dscrptrs[ready_domains_count].state = CLIENT_FSM_ST_CONNECT;

            FD_SET(cur_domain_socket_fd, read_fds);
            FD_SET(cur_domain_socket_fd, except_fds);
        }

        mail_domains_dscrptrs[ready_domains_count].mails_list = malloc(sizeof(node_t));
        mail_domains_dscrptrs[ready_domains_count].mails_list->next = NULL;

        add_first(&mail_domains_dscrptrs[ready_domains_count].mails_list, saved_email_path);
        log_i("Mail %s for %s domain successfully added to process queue", saved_email_path, cur_email_domain);
        log_i("%s domain mails count %d \n", cur_email_domain, count(mail_domains_dscrptrs[ready_domains_count].mails_list));
        free(cur_email_domain);
        free(saved_email_path);
        ready_domains_count++;
        return cur_domain_socket_fd;
    }
    // Домен уже зарегистрирован
    else
    {
        log_i("Email domain %s is already registered", cur_email_domain);
        if (count(mail_domains_dscrptrs[found_domain_num].mails_list) == 0 || mail_domains_dscrptrs[found_domain_num].state == CLIENT_FSM_ST_INIT)
        {
            mail_domains_dscrptrs[found_domain_num].state = client_fsm_step(mail_domains_dscrptrs[found_domain_num].state, CLIENT_FSM_EV_OK, NULL);
            int cur_domain_socket_fd = 0;
            if ((cur_domain_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                mail_domains_dscrptrs[ready_domains_count].state = client_fsm_step(mail_domains_dscrptrs[ready_domains_count].state, CLIENT_FSM_EV_ERROR, NULL);
                log_e("Could not create socket to %s", cur_email_domain);
                return -1;
            }

            if (connect(cur_domain_socket_fd,
                        (struct sockaddr *)&mail_domains_dscrptrs[found_domain_num].domain_mail_server, sizeof(mail_domains_dscrptrs[found_domain_num].domain_mail_server)) < 0)
            {
                mail_domains_dscrptrs[ready_domains_count].state = client_fsm_step(mail_domains_dscrptrs[ready_domains_count].state, CLIENT_FSM_EV_ERROR, NULL);
                log_e("Connection to %s Failed ", cur_email_domain);
                return -1;
            }
            else
            {
                mail_domains_dscrptrs[found_domain_num].socket_fd = cur_domain_socket_fd;
                mail_domains_dscrptrs[ready_domains_count].number_of_attempts = 0;
                mail_domains_dscrptrs[ready_domains_count].last_attempt_time = 0;
                mail_domains_dscrptrs[ready_domains_count].curr_rcpts_index = -1;
                fcntl(cur_domain_socket_fd, F_SETFL, O_NONBLOCK);
                log_i("Successfully connected to %s ", cur_email_domain);

                FD_SET(cur_domain_socket_fd, read_fds);
                FD_SET(cur_domain_socket_fd, except_fds);
            }
        }

        add_first(&mail_domains_dscrptrs[found_domain_num].mails_list, saved_email_path);
        log_i("Mail %s for %s domain successfully added to process queue", saved_email_path, cur_email_domain);
        log_i("%s domain mails count %d \n", cur_email_domain, count(mail_domains_dscrptrs[found_domain_num].mails_list));
        free(cur_email_domain);
        free(saved_email_path);
        return mail_domains_dscrptrs[ready_domains_count - 1].socket_fd;
    }
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
            handle_read_socket(cur_mail_domain, read_fds, write_fds);

        if (FD_ISSET(cur_mail_domain->socket_fd, write_fds))
            handle_write_socket(cur_mail_domain, read_fds, write_fds);

        if (FD_ISSET(cur_mail_domain->socket_fd, except_fds))
        {
            log_i("Socket %d of %s domain is in except_fds", cur_mail_domain->socket_fd, cur_mail_domain->domain);
            shutdown_properly(EXIT_FAILURE);
        }
    }
}

// Обрабатывает почтовый домен в случае, когда его сокет находится в write_fds
void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds)
{
    int code = send_msg_to_server(cur_mail_domain);
    if (code == -1)
    {
        log_e("Could not send data to server %s", cur_mail_domain->domain);
        //close_socket(sockets[i].fd);
        //replace_all_files_by_mx_record(sockets[i], "cur", "new");
        //sockets = delete_struct_socket_info_by_index(sockets, &count, i);
    }
    else if (code == -2)
    {
        FD_CLR(cur_mail_domain->socket_fd, write_fds);
        FD_SET(cur_mail_domain->socket_fd, write_fds);
    }
    else if (code == -3)
    {
        log_i("Timeout. Failed to send message to server %s", cur_mail_domain->domain);
        te_client_fsm_state new_state = client_fsm_step(cur_mail_domain->state, CLIENT_FSM_EV_RTIME_EXPIRED, NULL);

        if (new_state != CLIENT_FSM_ST_DONE)
            log_e("%s", "FSM Wrong transition state");

        FD_CLR(cur_mail_domain->socket_fd, write_fds);
        //replace_file(sockets[i].messages[0].address, "cur", "error");
        //sockets[i].messages = delete_item_by_index(sockets[i].messages, &sockets[i].msg_count, 0);

        // if (count(cur_mail_domain->mails_list) > 0)
        //     sockets = delete_struct_socket_info_by_index(sockets, &count, i);
    }
    else
    {
        FD_CLR(cur_mail_domain->socket_fd, write_fds);
        FD_SET(cur_mail_domain->socket_fd, read_fds);
    }
}

// Обрабатывает почтовый домен в случае, когда его сокет находится в read_fds
void handle_read_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds)
{
    char *server_response = read_data_from_server(cur_mail_domain->socket_fd);
    if (server_response == NULL)
    {
        log_e("%s", "Get NULL data from server");
        //close_socket(sockets[i].fd);
    }

    log_i("Socket %d of %s domain is in %d READ_FDS", cur_mail_domain->socket_fd, cur_mail_domain->domain, cur_mail_domain->state);
    te_client_fsm_event event = check_server_code(server_response);
    if (cur_mail_domain->state == CLIENT_FSM_ST_SEND_BODY && event != CLIENT_FSM_EV_ERROR)
    {
        remove_first(&cur_mail_domain->mails_list);
        free(cur_mail_domain->buffer);
        if (count(cur_mail_domain->mails_list) > 0)
            cur_mail_domain->state = client_fsm_step(cur_mail_domain->state, CLIENT_FSM_EV_MULTIPLE_EMAILS, NULL);
        else
            cur_mail_domain->state = client_fsm_step(cur_mail_domain->state, event, NULL);
    }
    else
        cur_mail_domain->state = client_fsm_step(cur_mail_domain->state, event, NULL);

    if (cur_mail_domain->state == CLIENT_FSM_ST_INIT)
    {
        close(cur_mail_domain->socket_fd);
        FD_CLR(cur_mail_domain->socket_fd, write_fds);
    }
    else
        FD_SET(cur_mail_domain->socket_fd, write_fds);

    FD_CLR(cur_mail_domain->socket_fd, read_fds);
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
