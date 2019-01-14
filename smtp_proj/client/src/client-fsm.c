/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (client-fsm.c)
 *
 *  It has been AutoGen-ed
 *  From the definitions    client.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (C) 1992-2014 Bruce Korb - all rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Bruce Korb'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM
#include "client-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */
/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

/**
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
    CLIENT_FSM_TR_CONNECT_ERROR,
    CLIENT_FSM_TR_CONNECT_TIMEOUT,
    CLIENT_FSM_TR_INIT_ERROR,
    CLIENT_FSM_TR_INIT_OK,
    CLIENT_FSM_TR_INIT_TIMEOUT,
    CLIENT_FSM_TR_INVALID,
    CLIENT_FSM_TR_SEND_BODY_ERROR,
    CLIENT_FSM_TR_SEND_BODY_TIMEOUT,
    CLIENT_FSM_TR_SEND_DATA_ERROR,
    CLIENT_FSM_TR_SEND_DATA_OK,
    CLIENT_FSM_TR_SEND_DATA_TIMEOUT,
    CLIENT_FSM_TR_SEND_HELO_ERROR,
    CLIENT_FSM_TR_SEND_HELO_OK,
    CLIENT_FSM_TR_SEND_HELO_RTIME_EXPIRED,
    CLIENT_FSM_TR_SEND_HELO_TIMEOUT,
    CLIENT_FSM_TR_SEND_MAIL_FROM_ERROR,
    CLIENT_FSM_TR_SEND_MAIL_FROM_OK,
    CLIENT_FSM_TR_SEND_MAIL_FROM_TIMEOUT,
    CLIENT_FSM_TR_SEND_QUIT_ERROR,
    CLIENT_FSM_TR_SEND_QUIT_OK,
    CLIENT_FSM_TR_SEND_QUIT_TIMEOUT,
    CLIENT_FSM_TR_SEND_RCPT_TO_ERROR,
    CLIENT_FSM_TR_SEND_RCPT_TO_MULTIPLE_RCPTS,
    CLIENT_FSM_TR_SEND_RCPT_TO_OK,
    CLIENT_FSM_TR_SEND_RCPT_TO_TIMEOUT
} te_client_fsm_trans;
#define CLIENT_FSM_TRANSITION_CT  25

/**
 *  State transition handling map.  Map the state enumeration and the event
 *  enumeration to the new state and the transition enumeration code (in that
 *  order).  It is indexed by first the current state and then the event code.
 */
typedef struct client_fsm_transition t_client_fsm_transition;
struct client_fsm_transition {
    te_client_fsm_state  next_state;
    te_client_fsm_trans  transition;
};
static const t_client_fsm_transition
client_fsm_trans_table[ CLIENT_FSM_STATE_CT ][ CLIENT_FSM_EVENT_CT ] = {

  /* STATE 0:  CLIENT_FSM_ST_INIT */
  { { CLIENT_FSM_ST_CONNECT, CLIENT_FSM_TR_INIT_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_INIT_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_INIT_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 1:  CLIENT_FSM_ST_SEND_HELO */
  { { CLIENT_FSM_ST_SEND_MAIL_FROM, CLIENT_FSM_TR_SEND_HELO_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_HELO_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_HELO_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_HELO_RTIME_EXPIRED }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 2:  CLIENT_FSM_ST_SEND_MAIL_FROM */
  { { CLIENT_FSM_ST_SEND_RCPT_TO, CLIENT_FSM_TR_SEND_MAIL_FROM_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_MAIL_FROM_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_MAIL_FROM_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 3:  CLIENT_FSM_ST_SEND_RCPT_TO */
  { { CLIENT_FSM_ST_SEND_DATA, CLIENT_FSM_TR_SEND_RCPT_TO_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_RCPT_TO_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_RCPT_TO_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_SEND_RCPT_TO, CLIENT_FSM_TR_SEND_RCPT_TO_MULTIPLE_RCPTS } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 4:  CLIENT_FSM_ST_SEND_DATA */
  { { CLIENT_FSM_ST_SEND_BODY, CLIENT_FSM_TR_SEND_DATA_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_DATA_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_DATA_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 5:  CLIENT_FSM_ST_SEND_BODY */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_BODY_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_BODY_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 6:  CLIENT_FSM_ST_SEND_QUIT */
  { { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_QUIT_OK }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_QUIT_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_SEND_QUIT_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  },


  /* STATE 7:  CLIENT_FSM_ST_CONNECT */
  { { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  OK */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_CONNECT_ERROR }, /* EVT:  ERROR */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  UNSENT_DATA */
    { CLIENT_FSM_ST_DONE, CLIENT_FSM_TR_CONNECT_TIMEOUT }, /* EVT:  TIMEOUT */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  RTIME_EXPIRED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID }, /* EVT:  COMMAND_NOT_IMPLEMENTED */
    { CLIENT_FSM_ST_INVALID, CLIENT_FSM_TR_INVALID } /* EVT:  MULTIPLE_RCPTS */
  }
};


#define Client_FsmFsmErr_off     19
#define Client_FsmEvInvalid_off  75
#define Client_FsmStInit_off     83


static char const zClient_FsmStrings[246] =
/*     0 */ "** OUT-OF-RANGE **\0"
/*    19 */ "FSM Error:  in state %d (%s), event %d (%s) is invalid\n\0"
/*    75 */ "invalid\0"
/*    83 */ "init\0"
/*    88 */ "send_helo\0"
/*    98 */ "send_mail_from\0"
/*   113 */ "send_rcpt_to\0"
/*   126 */ "send_data\0"
/*   136 */ "send_body\0"
/*   146 */ "send_quit\0"
/*   156 */ "connect\0"
/*   164 */ "ok\0"
/*   167 */ "error\0"
/*   173 */ "unsent_data\0"
/*   185 */ "timeout\0"
/*   193 */ "rtime_expired\0"
/*   207 */ "command_not_implemented\0"
/*   231 */ "multiple_rcpts";

static const size_t aszClient_FsmStates[8] = {
    83,  88,  98,  113, 126, 136, 146, 156 };

static const size_t aszClient_FsmEvents[8] = {
    164, 167, 173, 185, 193, 207, 231, 75 };


#define CLIENT_FSM_EVT_NAME(t)   ( (((unsigned)(t)) >= 8) \
    ? zClient_FsmStrings : zClient_FsmStrings + aszClient_FsmEvents[t])

#define CLIENT_FSM_STATE_NAME(s) ( (((unsigned)(s)) >= 8) \
    ? zClient_FsmStrings : zClient_FsmStrings + aszClient_FsmStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

static int client_fsm_invalid_transition( te_client_fsm_state st, te_client_fsm_event evt );

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * */
/**
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
static int
client_fsm_invalid_transition( te_client_fsm_state st, te_client_fsm_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char const * fmt = zClient_FsmStrings + Client_FsmFsmErr_off;
    fprintf( stderr, fmt, st, CLIENT_FSM_STATE_NAME(st), evt, CLIENT_FSM_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  CLIENT_FSM_ST_DONE or CLIENT_FSM_ST_INVALID, it resets to
 *  CLIENT_FSM_ST_INIT and returns CLIENT_FSM_ST_INIT.
 */
te_client_fsm_state
client_fsm_step(
    te_client_fsm_state client_fsm_state,
    te_client_fsm_event trans_evt,
    void *data )
{
    te_client_fsm_state nxtSt;
    te_client_fsm_trans trans;

    if ((unsigned)client_fsm_state >= CLIENT_FSM_ST_INVALID) {
        return CLIENT_FSM_ST_INIT;
    }

#ifndef __COVERITY__
    if (trans_evt >= CLIENT_FSM_EV_INVALID) {
        nxtSt = CLIENT_FSM_ST_INVALID;
        trans = CLIENT_FSM_TR_INVALID;
    } else
#endif /* __COVERITY__ */
    {
        const t_client_fsm_transition* pTT =
            client_fsm_trans_table[ client_fsm_state ] + trans_evt;
        nxtSt = pTT->next_state;
        trans = pTT->transition;
    }


    switch (trans) {
    case CLIENT_FSM_TR_CONNECT_ERROR:
        /* START == CONNECT_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_CONNECT_ERROR();
        /* END   == CONNECT_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_CONNECT_TIMEOUT:
        /* START == CONNECT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_CONNECT_TIMEOUT();
        /* END   == CONNECT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_INIT_ERROR:
        /* START == INIT_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_INIT_ERROR();
        /* END   == INIT_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_INIT_OK:
        /* START == INIT_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_INIT_OK();
        /* END   == INIT_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_INIT_TIMEOUT:
        /* START == INIT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_INIT_TIMEOUT();
        /* END   == INIT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_INVALID:
        /* START == INVALID == DO NOT CHANGE THIS COMMENT */
        exit(client_fsm_invalid_transition(client_fsm_state, trans_evt));
        /* END   == INVALID == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_BODY_ERROR:
        /* START == SEND_BODY_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_BODY_ERROR();
        /* END   == SEND_BODY_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_BODY_TIMEOUT:
        /* START == SEND_BODY_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_BODY_TIMEOUT();
        /* END   == SEND_BODY_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_DATA_ERROR:
        /* START == SEND_DATA_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_DATA_ERROR();
        /* END   == SEND_DATA_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_DATA_OK:
        /* START == SEND_DATA_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_DATA_OK();
        /* END   == SEND_DATA_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_DATA_TIMEOUT:
        /* START == SEND_DATA_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_DATA_TIMEOUT();
        /* END   == SEND_DATA_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_HELO_ERROR:
        /* START == SEND_HELO_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_HELO_ERROR();
        /* END   == SEND_HELO_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_HELO_OK:
        /* START == SEND_HELO_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_HELO_OK();
        /* END   == SEND_HELO_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_HELO_RTIME_EXPIRED:
        /* START == SEND_HELO_RTIME_EXPIRED == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_HELO_RTIME_EXPIRED();
        /* END   == SEND_HELO_RTIME_EXPIRED == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_HELO_TIMEOUT:
        /* START == SEND_HELO_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_HELO_TIMEOUT();
        /* END   == SEND_HELO_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_MAIL_FROM_ERROR:
        /* START == SEND_MAIL_FROM_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_MAIL_FROM_ERROR();
        /* END   == SEND_MAIL_FROM_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_MAIL_FROM_OK:
        /* START == SEND_MAIL_FROM_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_MAIL_FROM_OK();
        /* END   == SEND_MAIL_FROM_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_MAIL_FROM_TIMEOUT:
        /* START == SEND_MAIL_FROM_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_MAIL_FROM_TIMEOUT();
        /* END   == SEND_MAIL_FROM_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_QUIT_ERROR:
        /* START == SEND_QUIT_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_QUIT_ERROR();
        /* END   == SEND_QUIT_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_QUIT_OK:
        /* START == SEND_QUIT_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_QUIT_OK();
        /* END   == SEND_QUIT_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_QUIT_TIMEOUT:
        /* START == SEND_QUIT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_QUIT_TIMEOUT();
        /* END   == SEND_QUIT_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_RCPT_TO_ERROR:
        /* START == SEND_RCPT_TO_ERROR == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_RCPT_TO_ERROR();
        /* END   == SEND_RCPT_TO_ERROR == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_RCPT_TO_MULTIPLE_RCPTS:
        /* START == SEND_RCPT_TO_MULTIPLE_RCPTS == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_RCPT_TO_MULTIPLE_RCPTS();
        /* END   == SEND_RCPT_TO_MULTIPLE_RCPTS == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_RCPT_TO_OK:
        /* START == SEND_RCPT_TO_OK == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_RCPT_TO_OK();
        /* END   == SEND_RCPT_TO_OK == DO NOT CHANGE THIS COMMENT */
        break;


    case CLIENT_FSM_TR_SEND_RCPT_TO_TIMEOUT:
        /* START == SEND_RCPT_TO_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        nxtSt = HANDLE_SEND_RCPT_TO_TIMEOUT();
        /* END   == SEND_RCPT_TO_TIMEOUT == DO NOT CHANGE THIS COMMENT */
        break;


    default:
        /* START == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
        exit(client_fsm_invalid_transition(client_fsm_state, trans_evt));
        /* END   == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
    }


    /* START == FINISH STEP == DO NOT CHANGE THIS COMMENT */
    /* END   == FINISH STEP == DO NOT CHANGE THIS COMMENT */

    return nxtSt;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of client-fsm.c */