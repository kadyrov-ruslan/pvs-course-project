AutoGen Definitions fsm;

state = parse_cmd, process_quit, process_verify, process_rset,
	seq_err, st_err, trans_begin, rcpt_begin, rcpt_middle,
	data_wait, parse_data, store_mail;
event = HELO, EHLO, VRFY, RSET, QUIT, MAIL, RCPT, DATA, DATA_RCV, DATA_END, OK, ERR;
type = reent;
method = call;
prefix = smtp;
cookie = "void *data";

transition = { tst = "*"; tev = VRFY; next = process_verify;};
transition = { tst = process_verify; tev = OK; next = "*";};
transition = { tst = "*"; tev = RSET; next = process_rset;};
transition = { tst = process_rset; tev = OK; next = "*";};
transition = { tst = "*"; tev = QUIT; next = process_quit;};
transition = { tst = process_quit; tev = OK; next = done;};
transition = { tst = seq_err; tev = OK; next = "*";};
transition = { tst = st_err; tev = OK; next = "*";};

transition = { tst = init; tev = HELO, EHLO; next = parse_cmd;};
transition = { tst = init; tev = MAIL, RCPT, DATA, DATA_RCV; next = seq_err;};

transition = { tst = parse_cmd; tev = HELO, EHLO; next = trans_begin;};
transition = { tst = parse_cmd; tev = MAIL; next = rcpt_begin;};
transition = { tst = parse_cmd; tev = RCPT; next = rcpt_middle;};
transition = { tst = parse_cmd; tev = ERR; next = st_err;};
transition = { tst = parse_cmd; tev = DATA; next = data_wait;};

transition = { tst = trans_begin; tev = MAIL; next = parse_cmd;};
transition = { tst = trans_begin; tev = HELO, EHLO, RCPT, DATA, DATA_RCV; next = seq_err;};

transition = { tst = rcpt_begin; tev = RCPT; next = parse_cmd;};
transition = { tst = rcpt_begin; tev = HELO, EHLO, MAIL, DATA, DATA_RCV; next = seq_err;};
transition = { tst = rcpt_middle; tev = RCPT, DATA; next = parse_cmd;};
transition = { tst = rcpt_middle; tev = HELO, EHLO, MAIL, DATA_RCV; next = seq_err;};

transition = { tst = data_wait; tev = DATA_RCV; next = parse_data;};
transition = { tst = data_wait; tev = ERR; next = st_err;};
transition = { tst = data_wait; tev = HELO, EHLO, MAIL, RCPT, DATA, DATA_END; next = seq_err;};
transition = { tst = parse_data; tev = DATA_RCV; next = data_wait;};
transition = { tst = parse_data; tev = DATA_END; next = store_mail;};
transition = { tst = parse_data; tev = HELO, EHLO, MAIL, RCPT, DATA; next = seq_err;};

transition = { tst = store_mail; tev = OK; next = init;};
transition = { tst = store_mail; tev = ERR; next = st_err;};
