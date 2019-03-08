#include "../include/mx_utils.h"

struct sockaddr_in get_domain_server_info(char *domain_name)
{
    struct sockaddr_in mail_server; //this struct contains ip address and a port of the server.
    struct hostent *mail_info = gethostbyname(domain_name);
    if (mail_info == NULL)
    {
        bzero(&mail_server, sizeof(mail_server));
        mail_server.sin_port = htons(0); //port 25=SMTP.
    }
    else
    {
        char *mail_ip = (char *)malloc(INET_ADDRSTRLEN + 1);
        inet_ntop(AF_INET, mail_info->h_addr_list[0], mail_ip, INET_ADDRSTRLEN);
        free(mail_ip);

        bzero(&mail_server, sizeof(mail_server));
        mail_server.sin_family = AF_INET; //AF_INIT means Internet doamin socket.
        mail_server.sin_port = htons(25); //port 25=SMTP.
        bcopy((char *)mail_info->h_addr_list[0], (char *)&mail_server.sin_addr.s_addr, mail_info->h_length);
    }
    return mail_server;
}

char *get_domain_mx_server_name(char *domain_name)
{
    static char mx_server[4096];
    u_char nsbuf[N];
    ns_msg msg;
    ns_rr rr;
    int r;

    if (strcmp(domain_name, "localhost.com") == 0)
        return "localhost";

    // MX RECORD
    r = res_query(domain_name, ns_c_any, ns_t_mx, nsbuf, sizeof(nsbuf));
    if (r < 0)
    {
        //perror(domain_name);
        return NULL;
    }
    else
    {
        ns_initparse(nsbuf, r, &msg);
        ns_parserr(&msg, ns_s_an, 0, &rr);

        const size_t size = NS_MAXDNAME;
        unsigned char name[size];
        int t = ns_rr_type(rr);

        const u_char *data = ns_rr_rdata(rr);
        if (t == T_MX)
        {
            ns_name_unpack(nsbuf, nsbuf + r, data + sizeof(u_int16_t), name, size);
            ns_name_ntop(name, mx_server, 4096);
        }
    }
    return mx_server;
}