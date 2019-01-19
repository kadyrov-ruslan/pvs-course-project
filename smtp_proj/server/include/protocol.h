#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "conn.h"
#include "pattern.h"

#define CONN_TIMEOUT 30 * 1000

extern conn_t *connections[1024];

patterns_t patterns;
te_server_event event_map[PT_END];
int (* process_bind[SERVER_STATE_CT])(conn_t*, const char*);

#define response_220 "220 example.com\r\n"
#define response_221 "221 Closing connection\r\n"
#define response_250 "250 Ok\r\n"
#define response_252 "252 Cannot VRFY user, but will accept message and attempt delivery\r\n"
#define response_354 "354 Start mail input; end with <CRLF>.<CRLF>\r\n"
#define response_421 "421 Server is not available\r\n"
#define response_451 "451 Requested action aborted: local error in processing\r\n"
#define response_500 "500 Invalid command\r\n"
#define response_501 "501 Syntax error in parameters or arguments\r\n"
#define response_502 "502 Command not implemented\r\n"
#define response_554 "554 Transaction failed\r\n"

int protocol_init();

int protocol_update();

int process_helo(conn_t *conn, const char* data);

int process_ehlo(conn_t *conn, const char* data);

int process_mail(conn_t *conn, const char* data);

int process_rcpt(conn_t *conn, const char* data);

int process_data(conn_t *conn, const char* data);

int process_data_recv(conn_t *conn, const char* data);

int process_data_end(conn_t *conn, const char* data);

int process_rset(conn_t *conn, const char* data);

int process_vrfy(conn_t *conn, const char* data);

int process_quit(conn_t *conn, const char* data);

#endif // _PROTOCOL_H_
