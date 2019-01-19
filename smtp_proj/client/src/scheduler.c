#include "../include/scheduler.h"

int maxfd = 0;

int run_client(int proc_num, int total_send_time, int retry_time)
{
    //int fork_res = -1;
    int child_id = -1;
    struct mail_process_dscrptr mail_procs[proc_num];
    for (pid_t i = 0; i < proc_num; i++)
    {
        mail_procs[i].pid = fork();
        if (mail_procs[i].pid == 0)
        {
            child_id = i;
            break;
        }
        else if (mail_procs[i].pid == -1)
            log_e("%s", "Can't fork worker process\n");
    }

    if (child_id != -1)
        child_process_worker_start(child_id, total_send_time, retry_time);
    else
        master_process_worker_start(proc_num, mail_procs);

    return 1;
}

// Содержит бизнес логику, обрабатываемую главным процессом
int master_process_worker_start(int proc_num, struct mail_process_dscrptr *mail_procs)
{
    log_i("%s", "Worker for master proc successfully started");
    //struct mail_process_dscrptr mail_procs[proc_num];
    for (int i = 0; i < proc_num; i++)
    {
        key_t key = ftok("/tmp", i);
        mail_procs[i].msg_queue_id = msgget(key, 0666 | IPC_CREAT);
        mail_procs[i].domains_count = 0;
    }

    struct domain_mails domains_mails[MAX_MAIL_DOMAIN_NUM * 2];
    int domains_count = 0;
    while (1)
    {
        domains_count = get_domains_mails(domains_mails, domains_count);
        for (int i = 0; i < domains_count; i++)
        {
            int domain_proc_idx = get_mail_proc_idx(domains_mails[i].domain, domains_count, mail_procs, proc_num);
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

// Возвращает индекс процесса, в который стоит отправить новое письмо на обработку
int get_mail_proc_idx(char *domain_name, int domains_count, struct mail_process_dscrptr *mail_procs, int proc_num)
{
    for (int j = 0; j < proc_num; j++)
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
    int min_proc_idx = 0;
    for (int j = 1; j < proc_num; j++)
    {
        if (mail_procs[min_proc_idx].domains_count > mail_procs[j].domains_count)
            min_proc_idx = j;
    }

    mail_procs[min_proc_idx].domains[mail_procs[min_proc_idx].domains_count] = malloc(strlen(domain_name));
    strcpy(mail_procs[min_proc_idx].domains[mail_procs[min_proc_idx].domains_count], domain_name);
    mail_procs[min_proc_idx].domains_count++;
    log_i("Process %d handles %s domain. Domains count %d", min_proc_idx, domain_name, mail_procs[1].domains_count);
    return min_proc_idx;
}

// Содержит бизнес логику, обрабатываемую дочерним процессом
int child_process_worker_start(int proc_idx, int total_send_time, int retry_time)
{
    log_i("Worker for child proc `%d' successfully started.", getpid());
    struct mail_domain_dscrptr mail_domains_dscrptrs[MAX_MAIL_DOMAIN_NUM];
    int ready_domains_count = 0;

    fd_set read_fds;
    fd_set write_fds;
    fd_set except_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);

    key_t key = ftok("/tmp", proc_idx);
    int cur_proc_mq_id = msgget(key, 0644);
    struct queue_msg cur_msg;

    struct timeval tv;
    while (1)
    {
        if (msgrcv(cur_proc_mq_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
        {
            if (strlen(cur_msg.mtext) != 0)
                ready_domains_count = register_new_email(cur_msg.mtext, mail_domains_dscrptrs,
                                                         &read_fds, &write_fds, &except_fds, total_send_time, retry_time, ready_domains_count);
        }

        for (int i = 0; i < ready_domains_count; i++)
        {
            struct mail_domain_dscrptr *cur_domain = &mail_domains_dscrptrs[i];
            if (cur_domain->socket_fd > maxfd)
                maxfd = cur_domain->socket_fd;
        }

        tv.tv_sec = 15;
        tv.tv_usec = 0;
        int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, &tv);
        if (activity <= 0)
        {
            //log_e(" error %d", errno);
            //shutdown_properly(EXIT_FAILURE);
        }

        for (int i = 0; i < maxfd + 1; i++)
        {
            struct mail_domain_dscrptr *cur_mail_domain;
            for (int j = 0; j < ready_domains_count; j++)
            {
                if (mail_domains_dscrptrs[j].socket_fd == i)
                {
                    cur_mail_domain = &mail_domains_dscrptrs[j];
                    break;
                }
            }

            if (FD_ISSET(i, &read_fds))
                handle_read_socket(cur_mail_domain, &read_fds, &write_fds);

            if (FD_ISSET(i, &write_fds))
                handle_write_socket(cur_mail_domain, &read_fds, &write_fds);

            if (FD_ISSET(i, &except_fds))
            {
                log_i("Socket %d of %s domain is in except_fds", cur_mail_domain->socket_fd, cur_mail_domain->domain);
                //shutdown_properly(EXIT_FAILURE);
            }
        }
    }

    return 1;
}

// Ожидает заданное число секунд
void wait_for(unsigned int secs)
{
    unsigned int retTime = time(0) + secs;
    while (time(0) < retTime)
        ; // Loop until it arrives.
}

void shutdown_master_properly(int proc_num, struct mail_process_dscrptr *mail_procs)
{
    for (int i = 0; i < proc_num; i++)
        kill(mail_procs[i].pid, SIGINT);

    printf("Shutdown client properly.\n");
    exit(0);
}

void shutdown_child_properly(int signal, int maxfd)
{
    switch (signal)
    {
    case SIGINT:
        close_all_conns(maxfd);
        exit(0);
    }
}
