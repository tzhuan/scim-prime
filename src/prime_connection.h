/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifndef __SCIM_PRIME_CONNECTION_H__
#define __SCIM_PRIME_CONNECTION_H__

#define Uses_SCIM_ICONV
#include <sys/types.h>
#include <unistd.h>
#include <scim.h>
#include "prime_session.h"
#include "prime_commands.h"

using namespace scim;


typedef enum {
    PRIME_CONNECTION_PIPE,
    PRIME_CONNECTION_UNIX_SOCKET,
    PRIME_CONNECTION_TCP_IP,
} PrimeConnectionType;


class PrimeCandidate
{
public:
    PrimeCandidate ();
    virtual ~PrimeCandidate ();

public:
    WideString m_preedition;
    WideString m_conversion;
    int        m_priority;
    WideString m_part;
    WideString m_base;
    WideString m_basekey;
    WideString m_suffix;
    WideString m_conjugation;
};


typedef std::vector<PrimeCandidate> PrimeCandidates;


class PrimeConnection
{
public:
    PrimeConnection                         (void);
    virtual ~PrimeConnection                ();

    // connection
    void                open_connection     (void);
    void                close_connection    (void);

    // comunication
    // Arguments must be terminated by NULL pointer.
    bool                send_command        (const char          *command,
                                             ...);

    // getting reply string
    void                get_reply           (String              &reply);
    void                get_reply           (WideString          &reply);
    void                get_reply           (std::vector<String> &str_list,
                                             char                *delim);

    // get prime version
    void                version             (String              &version);

    // refresh prime
    void                refresh             (void);

    // session
    PrimeSession       *session_start       (void);
    void                session_end         (PrimeSession        *session);

    // context
    void                set_context         (WideString          &context);
    void                reset_context       (void);

    // lookup
    bool                lookup              (const char          *sequence,
                                             PrimeCandidates     &candidates,
                                             const char          *command = PRIME_LOOKUP);

private:
    void                split_string        (String              &str,
                                             std::vector<String> &str_list,
                                             char                *delim);


public:
    IConvert         m_iconv;

private:
    pid_t            m_pid;
    int              m_in_fd;
    int              m_out_fd;
    int              m_err_fd;

    String           m_last_reply; // EUC-JP
};

#endif /* __SCIM_PRIME_CONNECTION_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
