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
#include <map>
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
    std::map<String, WideString> m_values;
};


typedef std::vector<String> Strings;
typedef std::vector<PrimeCandidate> PrimeCandidates;


class PrimeSession;


class PrimeConnection
{
public:
    PrimeConnection                         (void);
    virtual ~PrimeConnection                ();

    // connection
    void                open_connection     (const char *command,
                                             const char *typing_method = NULL,
                                             bool save = true);
    void                close_connection    (void);
    void                close_connection_with_error (void);
    bool                is_connected        (void);
    PrimeConnectionType get_connection_type (void) { return m_connection_type; }
    pid_t               get_child_pid       (void) { return m_pid; }

    // comunication
    // Arguments must be terminated by NULL pointer.
    bool                send_command        (const char      *command,
                                             ...);

    // getting reply string
    void                get_reply           (String          &reply);
    void                get_reply           (WideString      &reply);
    void                get_reply           (Strings         &str_list,
                                             char            *delim,
                                             int              num = -1);

    // get prime version
    void                version             (String          &version);
    int                 major_version       (void);
    int                 minor_version       (void);
    int                 micro_version       (void);

    // get variables
    void                get_env             (const String    &key,
                                             String          &type,
                                             Strings         &values);

    // refresh prime
    void                refresh             (void);

    // session
    PrimeSession       *session_start       (const char      *language = NULL);
    void                session_end         (PrimeSession    *session);

    // context
    void                set_context         (WideString      &context);
    void                reset_context       (void);

    // preedition
    void                preedit_convert_input (const String  &pattern,
                                               WideString    &preedition,
                                               WideString    &pending);

    // lookup
    bool                lookup              (const String    &sequence,
                                             PrimeCandidates &candidates,
                                             const char      *command = PRIME_LOOKUP);

    // learn
    void                learn_word          (WideString       key,
                                             WideString       value,
                                             WideString       part,
                                             WideString       context,
                                             WideString       suffix,
                                             WideString       rest);

private:
    int                 get_version_int     (int idx); // major: idx=0
                                                       // minor: idx=1
                                                       // micro: idx=2
    void                clean_child         (void);

public:
    IConvert            m_iconv;

private:
    PrimeConnectionType m_connection_type;

    pid_t               m_pid;
    int                 m_in_fd;
    int                 m_out_fd;
    int                 m_err_fd;

    String              m_typing_method;

    String              m_last_reply; // EUC-JP
};


void scim_prime_util_split_string (String  &str,
                                   Strings &str_list,
                                   char    *delim,
                                   int      num = -1);

#endif /* __SCIM_PRIME_CONNECTION_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
