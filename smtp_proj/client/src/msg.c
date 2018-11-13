#include "../include/msg.h"

char *read_msg_file(char *email_path)
{
    FILE *fp;
    fp = fopen(email_path, "r");
    if (fp == NULL)
    {
        printf("%s\n", strerror(errno));
        exit(0);
    }

    //initializing variables
    char *msg = (char *)malloc(INITIAL_SIZE);
    char tmp[INITIAL_SIZE];
    int msg_length = INITIAL_SIZE;
    int tmp_length = INITIAL_SIZE;
    bzero(msg, msg_length);
    bzero(tmp, tmp_length);
    int bytes = 0;
    int n = 0;
    msg[0] = '\0';
    while (1)
    {
        //reading data dynamically
        n = fread(tmp, 1, tmp_length - 1, fp);
        if (n < 0)
        {
            printf("%s\n", strerror(errno));
            exit(0);
        }
        if (n == 0)
            break;

        bytes += n;
        //checking if the buffer has enough length to contain the part of the message
        if (bytes < msg_length)
            strcat(msg, tmp);

        //checking if the buffer does not has enough length to contain the part of the message
        //therefore it need to be realloced
        if (bytes >= msg_length)
        {
            msg_length = bytes;
            msg = (char *)realloc(msg, msg_length + 1);
            strcat(msg, tmp);
        }
    }

    fclose(fp);
                                //printf("EMAIL FILE MSG %s\n", msg);
    return msg;
}