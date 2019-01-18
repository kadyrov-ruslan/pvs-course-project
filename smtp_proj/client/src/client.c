#include "../include/client_types.h"
#include "../include/scheduler.h"
#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#define CLIENT_USAGE "Usage: <client> <config_file>"
#define PROC_CNT_DEFAULT 6
#define RETRY_TIME_DEFAULT 10
#define TOTAL_SEND_TIME_DEFAULT 120

static config_t client_conf;
static char hostname_sys[NAME_MAX];
struct client_conf conf;

__attribute__((constructor)) static void client_init(void)
{
    config_init(&client_conf);
}

__attribute__((destructor)) static void client_deinit(void)
{
    config_destroy(&client_conf);
}

static int client_parse_config(void)
{
    struct stat mail_dir_st;
    int log_lvl;
    const char *user_group;
    struct passwd *pwd;
    struct group *gr;
    config_setting_t *system = config_lookup(&client_conf, "system");

    if (system == NULL)
    {
        log_e("%s", "not `system' parametr");
        return -1;
    }

    if (config_lookup_int(&client_conf, "proc_cnt", &conf.proc_cnt) != CONFIG_TRUE)
        conf.proc_cnt = PROC_CNT_DEFAULT;

    if (config_lookup_int(&client_conf, "retry_time", &conf.retry_time) != CONFIG_TRUE)
        conf.retry_time = RETRY_TIME_DEFAULT;

    if (config_lookup_int(&client_conf, "total_send_time", &conf.total_send_time) != CONFIG_TRUE)
        conf.total_send_time = TOTAL_SEND_TIME_DEFAULT;

    if (config_lookup_string(&client_conf, "hostname", &conf.hostname) != CONFIG_TRUE)
    {
        if (gethostname(hostname_sys, NAME_MAX) != 0)
        {
            log_e("gethostname() failed: %s", strerror(errno));
            abort();
        }

        conf.hostname = hostname_sys;
    }
    log_i("`hostname' %s'", conf.hostname);
    if (config_setting_lookup_int(system, "log_level", &log_lvl) != CONFIG_TRUE ||
        log_lvl < 0 || log_lvl >= LOG_LVL_LAST)
    {
        log_e("%s", "incorrect `log_level' parametr in config");
        return -1;
    }
    conf.log_lvl = log_lvl;
    cur_lvl = log_lvl;

    if (config_lookup_string(&client_conf, "mail_dir", &conf.mail_dir) != CONFIG_TRUE)
    {
        log_e("%s", "incorrect `mail_dir'");
        return -1;
    }

    if (stat(conf.mail_dir, &mail_dir_st) != 0)
    {
        log_e("incorrect mail dir: %s", strerror(errno));
        return -1;
    }

    if (config_setting_lookup_string(system, "user", &user_group) != CONFIG_TRUE)
    {
        log_e("%s", "No `user' parametr in config");
        return -1;
    }

    if ((pwd = getpwnam(user_group)) == NULL)
    {
        log_e("user %s doesn't exist or error occured", user_group);
        return -1;
    }

    if (config_setting_lookup_string(system, "group", &user_group) != CONFIG_TRUE)
    {
        log_e("%s", "No `group' parametr in config");
        return -1;
    }

    if ((gr = getgrnam(user_group)) == NULL)
    {
        log_e("group %s doesn't exist or error occured", user_group);
        return -1;
    }

    if (setgid(gr->gr_gid) != 0 ||
        setuid(pwd->pw_uid) != 0)
    {
        log_e("unable to change to user and group from config: %s", strerror(errno));
        return -1;
    }

    if (mail_dir_st.st_uid != pwd->pw_uid || mail_dir_st.st_gid != gr->gr_gid)
    {
        log_e("access denied to %s", conf.mail_dir);
        return -1;
    }

    return 0;
}

static int read_config(char *argv[])
{
    FILE *conf_f = fopen(argv[1], "r");
    if (conf_f == NULL)
    {
        perror("Unable to open config file:");
        return -1;
    }

    if (config_read(&client_conf, conf_f) != CONFIG_TRUE)
    {
        log_e("Error while config parsing: %s\n", config_error_text(&client_conf));
        return -1;
    }

    if (client_parse_config() != 0)
    {
        log_e("%s", "Unable to start client: incorrect config file");
        return -1;
    }

    fflush(stdout);
    log_i("config `%s' is correct. Ready to start client", argv[1]);
    return 0;
}

int main(int argc, char *argv[])
{
    //"/home/dev/pvs-course-project/smtp_proj/client/logs/"
    if (fork() == 0)
        start_logger(argv[2]);
    else
    {
        if (argc == 1)
        {
            log_e("%s", CLIENT_USAGE);
        }
        else
        {
            read_config(argv);
            run_client(conf.proc_cnt, conf.total_send_time, conf.retry_time);
        }
    }
    return 0;
}
