#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "conn.h"

extern conn_opts_t *connections[1024];

int protocol_init();

int protocol_update();

#endif // _PROTOCOL_H_
