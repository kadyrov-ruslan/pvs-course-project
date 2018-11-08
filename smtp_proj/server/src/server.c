#include "conn.h"

#include <stdlib.h>
#include <stdio.h>

#define USAGE "Usage: server <config file>"
#define PROCESS_DEFAULT 4

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, USAGE);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}