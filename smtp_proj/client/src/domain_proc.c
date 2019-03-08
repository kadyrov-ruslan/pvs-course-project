#include "../include/domain_proc.h"

extern int maxfd;

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
            char *user_dir_new = get_user_new_dir_full_path(conf.mail_dir, user_dir->d_name);
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

                        char *tmp_cur_mail_domain = get_domain_name_from_email_full_path(new_entry->d_name);
                        char **tokens = str_split(tmp_cur_mail_domain, ',');
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

            free(user_dir_new);
            closedir(new_dir);
        }
    }
    closedir(mail_dir);
    return domains_count;
}

// Регистрирует новое письмо в массиве дескрипторов mail_domains_dscrptrs для последующей обработки
int register_new_email(char *email_path, struct mail_domain_dscrptr *mail_domains_dscrptrs,
                       fd_set *read_fds, fd_set *write_fds, fd_set *except_fds, int total_send_time, int retry_time, int ready_domains_count)
{
    log_i("Registering new email %s", email_path);
    char *saved_email_path = malloc(strlen(email_path));
    strcpy(saved_email_path, email_path);

    char *tmp_cur_mail_domain = get_domain_name_from_email_full_path(email_path);
    char **tokens = str_split(tmp_cur_mail_domain, ',');
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

    int cur_domain_idx = 0;
    //Домен не найден - добавляем в массив и биндим новый сокет
    if (found_domain_num < 0)
    {
        cur_domain_idx = ready_domains_count;
        log_i("Email domain %s is not registered \n", cur_email_domain);
        mail_domains_dscrptrs[cur_domain_idx].domain = malloc(strlen(cur_email_domain));
        strcpy(mail_domains_dscrptrs[cur_domain_idx].domain, cur_email_domain);

        char *res = get_domain_mx_server_name(cur_email_domain);
        struct sockaddr_in cur_domain_srv = get_domain_server_info(res);

        mail_domains_dscrptrs[cur_domain_idx].domain_mail_server = cur_domain_srv;
        mail_domains_dscrptrs[cur_domain_idx].total_send_time = total_send_time;
        mail_domains_dscrptrs[cur_domain_idx].retry_time = retry_time;

        init_mail_domain_conn_settings(mail_domains_dscrptrs, cur_domain_idx, cur_email_domain, read_fds, except_fds);

        if (mail_domains_dscrptrs[cur_domain_idx].socket_fd > maxfd)
            maxfd = mail_domains_dscrptrs[cur_domain_idx].socket_fd;

        mail_domains_dscrptrs[cur_domain_idx].mails_list = malloc(sizeof(node_t));
        mail_domains_dscrptrs[cur_domain_idx].mails_list->next = NULL;
        ready_domains_count++;
    }
    // Домен уже зарегистрирован
    else
    {
        cur_domain_idx = found_domain_num;
        log_i("Email domain %s is already registered", cur_email_domain);
        if (count(mail_domains_dscrptrs[cur_domain_idx].mails_list) == 0 || mail_domains_dscrptrs[cur_domain_idx].state == CLIENT_FSM_ST_INIT)
            init_mail_domain_conn_settings(mail_domains_dscrptrs, cur_domain_idx, cur_email_domain, read_fds, except_fds);
    }

    if (mail_domains_dscrptrs[cur_domain_idx].state == CLIENT_FSM_ST_DONE)
    {
        FD_CLR(mail_domains_dscrptrs[cur_domain_idx].socket_fd, read_fds);
        FD_CLR(mail_domains_dscrptrs[cur_domain_idx].socket_fd, except_fds);
        mail_domains_dscrptrs[cur_domain_idx].socket_fd = -1;
        mail_domains_dscrptrs[cur_domain_idx].number_of_attempts = 0;
        mail_domains_dscrptrs[cur_domain_idx].last_attempt_time = 0;
        mail_domains_dscrptrs[cur_domain_idx].curr_rcpts_index = -1;
    }
    else
    {
        add_first(&mail_domains_dscrptrs[cur_domain_idx].mails_list, saved_email_path);
        log_i("Mail %s for %s domain successfully added to process queue", saved_email_path, cur_email_domain);
        log_i("%s domain mails count %d \n", cur_email_domain, count(mail_domains_dscrptrs[cur_domain_idx].mails_list));
    }

    free(cur_email_domain);
    free(saved_email_path);
    return ready_domains_count;
}

// Инициализирует сетевые настройки домена - сокет, число подключений и проч
void init_mail_domain_conn_settings(struct mail_domain_dscrptr *mail_domains_dscrptrs, int cur_domain_idx,
                                    char *cur_email_domain, fd_set *read_fds, fd_set *except_fds)
{
    mail_domains_dscrptrs[cur_domain_idx].state = client_fsm_step(mail_domains_dscrptrs[cur_domain_idx].state, CLIENT_FSM_EV_OK, NULL);
    int cur_domain_socket_fd = connect_to_mail_server(0, mail_domains_dscrptrs[cur_domain_idx].domain_mail_server, cur_email_domain);
    if (cur_domain_socket_fd > -1)
    {
        log_i("Successfully connected to %s , socket fd %d", cur_email_domain, cur_domain_socket_fd);
        mail_domains_dscrptrs[cur_domain_idx].socket_fd = cur_domain_socket_fd;
        mail_domains_dscrptrs[cur_domain_idx].number_of_attempts = 0;
        mail_domains_dscrptrs[cur_domain_idx].last_attempt_time = 0;
        mail_domains_dscrptrs[cur_domain_idx].curr_rcpts_index = -1;
        FD_SET(cur_domain_socket_fd, read_fds);
        FD_SET(cur_domain_socket_fd, except_fds);
    }
    else
        mail_domains_dscrptrs[cur_domain_idx].state = client_fsm_step(mail_domains_dscrptrs[cur_domain_idx].state, CLIENT_FSM_EV_ERROR, NULL);
}

// Обрабатывает почтовый домен в случае, когда его сокет находится в write_fds
void handle_write_socket(struct mail_domain_dscrptr *cur_mail_domain, fd_set *read_fds, fd_set *write_fds)
{
    int code = send_msg_to_server(cur_mail_domain);
    if (code == -1)
    {
        log_e("Could not send data to server %s", cur_mail_domain->domain);
        close(cur_mail_domain->socket_fd);
    }
    else if (code == -2)
    {
        log_i("Timeout -2. Failed to send message to server %s", cur_mail_domain->domain);
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