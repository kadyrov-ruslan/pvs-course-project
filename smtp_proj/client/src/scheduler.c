#include "../include/scheduler.h"

int maxfd = 0;

int run_client(int proc_num, int total_send_time, int retry_time)
{
    //create_child_proc(proc_num, total_send_time, retry_time);
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

void create_child_proc(int proc_num, int total_send_time, int retry_time)
{
    printf("%d\n", proc_num);
    int fork_res = -1;
    int child_id = 0;
    for (int i = 0; i < proc_num; ++i)
    {
        fork_res = fork();
        if (fork_res == 0)
        {
            child_id = i;
            break;
        }           
        else if (fork_res == -1)
        {
            fprintf(stderr, "Can't fork worker process\n");
        }
    }

    if (child_id != 0)
        child_process_worker_start(child_id, total_send_time, retry_time);
    else
    {
        struct mail_process_dscrptr mail_procs[proc_num];
        master_process_worker_start(mail_procs, proc_num);
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
        key_t key = ftok("/tmp", i);
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
    for (int j = 0; j < 6; j++)
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

    while (1)
    {
        if (msgrcv(cur_proc_mq_id, &cur_msg, sizeof(cur_msg), 1, IPC_NOWAIT) != -1)
        {
            if (strlen(cur_msg.mtext) != 0)
                ready_domains_count = register_new_email(cur_msg.mtext, mail_domains_dscrptrs,
                                                         &read_fds, &write_fds, &except_fds, total_send_time, retry_time, ready_domains_count);
        }

        int activity = select(maxfd + 1, &read_fds, &write_fds, &except_fds, NULL);
        if (activity <= 0)
        {
            log_e("%s", "select() returned -1");
            shutdown_properly(EXIT_FAILURE);
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
                shutdown_properly(EXIT_FAILURE);
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

void shutdown_properly(int code)
{
    //   delete_peer(&server);
    printf("Shutdown client properly.\n");
    exit(code);
}
