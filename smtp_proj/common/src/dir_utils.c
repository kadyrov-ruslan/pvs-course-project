#include "dir_utils.h"

int count_dir_entries(const char *dirname)
{
    //printf("opening %s\n", dirname);
    int n = 0;
    struct dirent *d;
    DIR *dir = opendir(dirname);
    if (dir == NULL)
    {
        //printf("NULL dir\n");
        return 0;
    }

    while ((d = readdir(dir)) != NULL)
    {
        if (strcmp(d->d_name, ".") != 0 && strcmp(d->d_name, "..") != 0)
            n++;
    }
    closedir(dir);
    return n;
}

char *get_user_new_dir_full_path(const char *mail_dir, char *user_dir_name)
{
    char *user_dir_full_path = malloc(strlen(mail_dir) + 2 + strlen(user_dir_name));
    strcpy(user_dir_full_path, mail_dir);
    strcat(user_dir_full_path, user_dir_name);
    strcat(user_dir_full_path, "/");

    char *user_dir_new = malloc(strlen(user_dir_full_path) + 4);
    strcpy(user_dir_new, user_dir_full_path);
    strcat(user_dir_new, "new/");

    free(user_dir_full_path);
    return user_dir_new;
}

char *get_domain_name_from_email_full_path(char *email_path)
{
    char **tokens = str_split(email_path, '.');
    char *first_part = tokens[2];
    char *second_part = tokens[3];

    char *mail_domain = malloc(strlen(first_part) + strlen(second_part) + 1);
    strcpy(mail_domain, first_part);
    strcat(mail_domain, ".");
    strcat(mail_domain, second_part);

    free(tokens[0]);
    free(tokens[1]);
    free(first_part);
    free(second_part);
    free(tokens[4]);
    free(tokens);

    return mail_domain;
}