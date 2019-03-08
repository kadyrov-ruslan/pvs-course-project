#include "../include/msg.h"

char *read_msg_file(char *email_path)
{
    FILE *fp;
    long file_size;
    char *buffer;

    fp = fopen(email_path, "r");
    if (fp == NULL)
    {
        //printf("%s\n", strerror(errno));
        //exit(0);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, file_size + 1);
    if (!buffer)
        fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, file_size, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    fclose(fp);
    return buffer;
}