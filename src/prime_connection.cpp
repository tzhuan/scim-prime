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
PrimeConnection::open_connection ()
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

        char *argv[2];
        argv[0] = "prime";
        argv[1] = NULL;

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
            ssize_t rv = write (m_in_fd, command + (len - remaining), remaining);
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

PrimeSession *
PrimeConnection::session_start (void)
{
    bool success = send_command (PRIME_SESSION_START, NULL);
    if (success) {
        m_sessions.push_back (PrimeSession(this, m_last_reply.c_str ()));
        return &m_sessions[m_sessions.size () - 1];
    }

    return NULL;
}

void
PrimeConnection::session_end (PrimeSession *session)
{
    if (!session)
        return;

#if 0
    bool success = send_command (PRIME_SESSION_END,
                                 session->get_id_str().c_str(),
                                 NULL);
    if (success) {
        for (std::vector<PrimeSession>::iterator it = m_sessions.begin ();
             it != m_sessions.end ();
             it++)
        {
            if ((*it).get_id_str() == session->get_id_str())
                m_sessions.erase(it);
        }
    }
#endif
}

bool
PrimeConnection::lookup (const char *sequence, PrimeCandidate &candidate)
{
    bool success = send_command (PRIME_LOOKUP, sequence, NULL);
    if (success) {
        std::vector<String> cols;
        split_string (m_last_reply, cols, "\t");

        if (cols.size () >= 2) {
            m_iconv.convert (candidate.m_preedition, cols[0]);
            m_iconv.convert (candidate.m_conversion, cols[1]);
        }
    } else {
        // error
    }

    return false;
}

bool
PrimeConnection::lookup_all (const char *sequence,
                             std::vector<PrimeCandidate> &candidates)
{
    candidates.clear ();

    bool success = send_command (PRIME_LOOKUP_ALL, sequence, NULL);
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
        }
    } else {
        // error
    }

    return false;
}

// FIXME
#include <stdarg.h>
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
        ssize_t rv = write (m_in_fd, str.c_str () + (len - remaining), remaining);
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
    m_iconv.convert(reply, m_last_reply);
}

void
PrimeConnection::get_reply (std::vector<String> &str_list, char *delim)
{
    split_string (m_last_reply, str_list, delim);
}


void
PrimeConnection::split_string (String &str, std::vector<String> &str_list,
                               char *delim)
{
    String::size_type start = 0, end;

    do {
        end = str.find(delim, start);
        if (end == String::npos)
            end = str.length ();

        str_list.push_back (str.substr(start, end - start));
        start = end + strlen(delim);
    } while (start < str.length ());
}
