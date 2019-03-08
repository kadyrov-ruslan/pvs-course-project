AutoGen Definitions fsm;

state = connected, disconnected, ready,
        process_helo, process_ehlo, process_quit, process_vrfy, process_rset, process_timeout,
        expect_mail, process_mail,
        expect_rcpt, process_rcpt,
        expect_data, process_data,
        expect_data_recv, process_data_recv, process_data_end;
event = CONNECT, TIMEOUT, OK, ERR,
        QUIT, RSET, VRFY, HELO, EHLO, MAIL, RCPT, DATA, DATA_RECV, DATA_END;
type = reent;
method = call;
prefix = server;
cookie = "void *data";

// CONNECTION
transition = { tst = init; tev = CONNECT; next = connected;};
transition = { tst = connected; tev = OK; next = ready;};
transition = { tst = disconnected; tev = OK; next = done;};

// QUIT
transition = { tst = connected; tev = QUIT; next = process_quit;};
transition = { tst = ready; tev = QUIT; next = process_quit;};
transition = { tst = expect_mail; tev = QUIT; next = process_quit;};
transition = { tst = expect_rcpt; tev = QUIT; next = process_quit;};
transition = { tst = expect_data; tev = QUIT; next = process_quit;};
transition = { tst = process_quit; tev = OK; next = disconnected;};

// TIMEOUT
transition = { tst = ready; tev = TIMEOUT; next = process_timeout;};
transition = { tst = expect_mail; tev = TIMEOUT; next = process_timeout;};
transition = { tst = expect_rcpt; tev = TIMEOUT; next = process_timeout;};
transition = { tst = expect_data; tev = TIMEOUT; next = process_timeout;};
transition = { tst = process_timeout; tev = OK; next = disconnected;};

// RSET
transition = { tst = ready; tev = RSET; next = process_rset;};
transition = { tst = expect_mail; tev = RSET; next = process_rset;};
transition = { tst = expect_rcpt; tev = RSET; next = process_rset;};
transition = { tst = expect_data; tev = RSET; next = process_rset;};
transition = { tst = process_rset; tev = OK; next = ready;};

// VRFY
transition = { tst = ready; tev = VRFY; next = process_vrfy;};
transition = { tst = expect_mail; tev = VRFY; next = process_vrfy;};
transition = { tst = expect_rcpt; tev = VRFY; next = process_vrfy;};
transition = { tst = expect_data; tev = VRFY; next = process_vrfy;};
//transition = { tst = process_vrfy; tev = OK; next = "*";};

// HELO
transition = { tst = ready; tev = HELO; next = process_helo;};
transition = { tst = process_helo; tev = ERR; next = ready;};
transition = { tst = process_helo; tev = OK; next = expect_mail;};

// EHLO
transition = { tst = ready; tev = EHLO; next = process_ehlo;};
transition = { tst = process_ehlo; tev = ERR; next = ready;};
transition = { tst = process_ehlo; tev = OK; next = expect_mail;};

// MAIL
transition = { tst = expect_mail; tev = MAIL; next = process_mail;};
transition = { tst = process_mail; tev = ERR; next = expect_mail;};
transition = { tst = process_mail; tev = OK; next = expect_rcpt;};

// RCPT
transition = { tst = expect_rcpt; tev = RCPT; next = process_rcpt;};
transition = { tst = process_rcpt; tev = ERR; next = expect_rcpt;};
transition = { tst = process_rcpt; tev = OK; next = expect_data;};

// DATA
transition = { tst = expect_data; tev = DATA; next = process_data;};
transition = { tst = process_data; tev = ERR; next = expect_data;};
transition = { tst = process_data; tev = OK; next = expect_data_recv;};

// DATA RECV
transition = { tst = expect_data_recv; tev = DATA_RECV; next = process_data_recv;};
transition = { tst = expect_data_recv; tev = DATA_END; next = process_data_end;};
transition = { tst = process_data_recv; tev = ERR; next = expect_data_recv;};
transition = { tst = process_data_recv; tev = OK; next = expect_data_recv;};

// DATA END
transition = { tst = process_data_end; tev = OK; next = expect_mail;};