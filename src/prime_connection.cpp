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

#include <stdarg.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "prime_connection.h"
#include "prime_commands.h"


PrimeCandidate::PrimeCandidate ()
{
}

PrimeCandidate::~PrimeCandidate ()
{
}

PrimeConnection::PrimeConnection ()
    : m_pid (0),
      m_in_fd (0),
      m_out_fd (0),
      m_err_fd (0)
{
    if (!m_iconv.set_encoding ("EUC-JP"))
        return;
}

PrimeConnection::~PrimeConnection ()
{
    close_connection ();
}

void
PrimeConnection::open_connection (const char *command,
                                  const char *typing_method)
{
    pid_t pid;
    int in_fd[2], out_fd[2], err_fd[2];

    if (m_pid)
        return;

    if (pipe (out_fd) < 0) goto ERROR0;
    if (pipe (err_fd) < 0) goto ERROR1;
    if (pipe (in_fd)  < 0) goto ERROR2;

    pid = fork ();

    if (pid < 0) {
        /* error */
        return;
    }

    if (pid > 0) {
        /* parent process */
        m_typing_method = typing_method ? typing_method : "";

        m_pid = pid;

        m_in_fd = in_fd[1];
        close (in_fd[0]);

        m_out_fd = out_fd[0];
        close (out_fd[1]);

        m_err_fd = err_fd[0];
        close (err_fd[1]);
        return;
    } else if (pid == 0) {
        /* child process */      

        char *argv[3];
        String method = "--typing-method=";

        argv[0] = (char *) command;
        if (typing_method && *typing_method) {
            method += typing_method;
            argv[1] = (char *) method.c_str();
        } else {
            argv[1] = NULL;
        }
        argv[2] = NULL;

        /* set pipe */
        close (out_fd[0]);
        close (err_fd[0]);
        close (in_fd[1]);
        dup2 (out_fd[1], STDOUT_FILENO);
        dup2 (err_fd[1], STDERR_FILENO);
        dup2 (in_fd[0],  STDIN_FILENO);

        execvp (argv[0], argv);

        /* error */
        close (out_fd[1]);
        close (err_fd[1]);
        close (in_fd[0]);

        switch (errno) {
        default:
            break;
        }

        _exit (255);
   }

    close (in_fd[0]);
    close (in_fd[1]);

ERROR2:
    close (err_fd[0]);
    close (err_fd[1]);
ERROR1:
    close (out_fd[0]);
    close (out_fd[1]);
ERROR0:
    return;
}

void
PrimeConnection::close_connection ()
{
    if (m_pid) {
        const char *command = PRIME_CLOSE "\n";
        size_t len, remaining;
        len = remaining = strlen (command);

        do {
            ssize_t rv = write (m_in_fd,
                                command + (len - remaining),
                                remaining);
            switch (errno) {
            case EBADF:
            case EINVAL:
            case EPIPE:
            case EIO:
                m_pid = 0;
                remaining = 0;
                break;
            default:
                remaining -= rv;
                break;
            }
        } while (remaining > 0);

        close (m_in_fd);
        close (m_out_fd);
        close (m_err_fd);

        m_pid    = 0;
        m_in_fd  = 0;
        m_out_fd = 0;
        m_err_fd = 0;
    }
}

void
PrimeConnection::version (String &version)
{
    bool success = send_command (PRIME_VERSION, NULL);
    if (success) {
        get_reply (version);
    } else {
        // error
    }
}

void
PrimeConnection::refresh (void)
{
    send_command (PRIME_REFRESH, NULL);
}

PrimeSession *
PrimeConnection::session_start (const char *language)
{
    bool success = send_command (PRIME_SESSION_START, language, NULL);
    if (success) {
        PrimeSession *session = new PrimeSession(this, m_last_reply.c_str(), language);
        return session;
    } else {
        // error
    }

    return NULL;
}

void
PrimeConnection::session_end (PrimeSession *session)
{
    if (!session)
        return;

    bool success = send_command (PRIME_SESSION_END,
                                 session->get_id_str().c_str(),
                                 NULL);
    if (success) {
    } else {
        // error
    }
}

void
PrimeConnection::set_context (WideString &context)
{
    String str;
    m_iconv.convert (str, context);
    send_command (PRIME_SET_CONTEXT, str.c_str(), NULL);
}

void
PrimeConnection::reset_context (void)
{
    send_command (PRIME_RESET_CONTEXT, NULL);
}

bool
PrimeConnection::lookup (const String &sequence,
                         PrimeCandidates &candidates,
                         const char *command)
{
    candidates.clear ();

    if (!command)
        command = PRIME_LOOKUP;

    bool success = send_command (command, sequence.c_str (), NULL);
    if (success) {
        std::vector<String> rows;
        split_string (m_last_reply, rows, "\n");

        for (unsigned int i = 0; i < rows.size (); i++) {
            candidates.push_back (PrimeCandidate ());

            std::vector<String> cols;
            split_string (rows[i], cols, "\t");

            if (cols.size () >= 2) {
                m_iconv.convert (candidates[i].m_preedition, cols[0]);
                m_iconv.convert (candidates[i].m_conversion, cols[1]);
            }

            for (unsigned int j = 2; j < cols.size (); j++) {
                std::vector<String> pair;
                split_string (cols[j], pair, "=", 2);

                if (pair[0] == "priority")
                    candidates[i].m_priority = pair[1];
                else if (pair[0] == "part")
                    m_iconv.convert (candidates[i].m_part, pair[1]);
                else if (pair[0] == "base")
                    m_iconv.convert (candidates[i].m_base, pair[1]);
                else if (pair[0] == "basekey")
                    m_iconv.convert (candidates[i].m_basekey, pair[1]);
                else if (pair[0] == "suffix")
                    m_iconv.convert (candidates[i].m_suffix, pair[1]);
                else if (pair[0] == "rest")
                    m_iconv.convert (candidates[i].m_rest, pair[1]);
                else if (pair[0] == "conjugation")
                    m_iconv.convert (candidates[i].m_conjugation, pair[1]);
                else if (pair[0] == "usage")
                    m_iconv.convert (candidates[i].m_usage, pair[1]);
                else if (pair[0] == "annotation")
                    m_iconv.convert (candidates[i].m_annotation, pair[1]);
            }
        }
    } else {
        // error
    }

    return false;
}

void
PrimeConnection::learn_word (WideString wkey, WideString wvalue,
                             WideString wpart, WideString wcontext,
                             WideString wsuffix, WideString wrest)
{
    String key, value, part, context, suffix, rest;

    m_iconv.convert (key, wkey);
    m_iconv.convert (value, wvalue);
    m_iconv.convert (part, wpart);
    m_iconv.convert (context, wcontext);
    m_iconv.convert (suffix, wsuffix);
    m_iconv.convert (rest, wrest);

    send_command (PRIME_LEARN_WORD,
                  key.c_str(), value.c_str(),
                  part.c_str(), context.c_str(),
                  suffix.c_str(), rest.c_str(),
                  NULL);
}

bool
PrimeConnection::send_command (const char *command,
                               ...)
{
    if (!command || !*command)
        return false;

    if (!m_pid || m_in_fd <= 0 || m_out_fd <= 0)
        return false;


    //
    // create command string
    //
    String str = command;

    va_list args;
    const char *arg;

    va_start (args, command);
    for (arg = va_arg(args, const char *);
         arg;
         arg = va_arg(args, const char *))
    {
        str += "\t";
        str += arg;
    }
    va_end (args);
    str += "\n";


    //
    // write the command
    //
    ssize_t rv;
    size_t len, remaining;
    len = remaining = str.length ();

    do {
        ssize_t rv = write (m_in_fd,
                            str.c_str() + (len - remaining),
                            remaining);
        switch (errno) {
        case EBADF:
        case EINVAL:
        case EPIPE:
        case EIO:
            m_pid = 0;
            close (m_in_fd);  m_in_fd  = 0;
            close (m_out_fd); m_out_fd = 0;
            close (m_err_fd); m_err_fd = 0;
            return false;
            break;
        default:
            remaining -= rv;
            break;
        }
    } while (remaining > 0);


    //
    // read the reply
    //
    m_last_reply = String ();

    char buf[4096];
    while (true) {
        rv = read (m_out_fd, buf, 4095);
        if (rv <= 0) {
            switch (errno) {
            case EBADF:
            case EINVAL:
            case EPIPE:
            case EIO:
                m_pid = 0;
                close (m_in_fd);  m_in_fd  = 0;
                close (m_out_fd); m_out_fd = 0;
                close (m_err_fd); m_err_fd = 0;
                break;
            default:
                break;
            }
            break;
        } else {
            buf[rv < 4095 ? rv : 4095] = '\0';
            m_last_reply += buf;
            if (m_last_reply.length () > 2 &&
                m_last_reply.substr (m_last_reply.length () - 2, 2) == "\n\n")
            {
                m_last_reply.erase (m_last_reply.length () - 2, 2);
                break;
            }
        }
    }

    if (m_last_reply.length () > 3 && m_last_reply.substr (0, 3) == "ok\n") {
        m_last_reply.erase (0, 3);
        return true;
    }

    if (m_last_reply.length () > 6 && m_last_reply.substr (0, 6) == "error\n") {
        m_last_reply.erase (0, 6);
        return false;
    }

    return false;
}

void
PrimeConnection::get_reply (String &reply)
{
    reply = m_last_reply;
}

void
PrimeConnection::get_reply (WideString &reply)
{
    m_iconv.convert (reply, m_last_reply);
}

void
PrimeConnection::get_reply (std::vector<String> &str_list, char *delim)
{
    split_string (m_last_reply, str_list, delim);
}


void
PrimeConnection::split_string (String &str, std::vector<String> &str_list,
                               char *delim, int num)
{
    String::size_type start = 0, end;

    for (int i = 0; (num > 0 && i < num) || start < str.length (); i++) {
        end = str.find (delim, start);
        if ((num > 0 && i == num - 1) || (end == String::npos))
            end = str.length ();

        if (start < str.length ()) {
            str_list.push_back (str.substr (start, end - start));
            start = end + strlen (delim);
        } else {
            str_list.push_back (String ());
        }
    }
}
