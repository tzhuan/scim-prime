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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "prime_connection.h"
#include "prime_session.h"
#include "prime_commands.h"
#include "intl.h"

static const int PIPE_FAILD          = 1;
static const int FORK_FAILD          = 2;
static const int CHILD_DUP2_FAILD    = 3;
static const int CHILD_EXEC_FAILD    = 4;
static const int COMMUNICATION_FAILD = 5;

static std::vector<PrimeConnection *> connection_list;

static int  sane_dup2      (int fd1,
                            int fd2);
static void handle_sigpipe (int signo);


PrimeCandidate::PrimeCandidate ()
{
}

PrimeCandidate::~PrimeCandidate ()
{
}

PrimeConnection::PrimeConnection ()
    : m_connection_type (PRIME_CONNECTION_PIPE),
      m_pid         (0),
      m_in_fd       (-1),
      m_out_fd      (-1),
      m_err_fd      (-1),
      m_exit_status (0)
{
    connection_list.push_back (this);

    if (!m_iconv.set_encoding ("EUC-JP"))
        return;
}

PrimeConnection::~PrimeConnection ()
{
    close_connection ();

    std::vector<PrimeConnection *>::iterator it;
    for (it = connection_list.begin (); it != connection_list.end (); it++) {
        if ((*it) == this)
	{
            connection_list.erase (it);
	    break;
	}
    }
}

bool
PrimeConnection::open_connection (const char *command,
                                  const char *typing_method,
                                  bool save)
{
    pid_t pid;
    int in_fd[2], out_fd[2], err_fd[2], report_fd[2];

    m_command = command ? command : "";
    m_typing_method = typing_method ? typing_method : "";
    m_exit_status = 0;
    m_err_msg = WideString ();

    if (m_pid > 0)
        return true;

    if (pipe (out_fd) < 0) {
        set_error_message (PIPE_FAILD, errno);
        goto ERROR0;
    }
    if (pipe (err_fd) < 0) {
        set_error_message (PIPE_FAILD, errno);
        goto ERROR1;
    }
    if (pipe (in_fd)  < 0) {
        set_error_message (PIPE_FAILD, errno);
        goto ERROR2;
    }
    if (pipe (report_fd) < 0) {
        set_error_message (PIPE_FAILD, errno);
        goto ERROR3;
    }

    pid = fork ();

    if (pid < 0) {
        set_error_message (FORK_FAILD, errno);
        goto ERROR4;
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

        int rv;
        close (report_fd[1]);
        rv = check_child_err (report_fd[0]);
        if (!rv)
            clean_child ();
        close (report_fd[0]);

        return rv;

    } else if (pid == 0) {
        /* child process */      

        char *argv[4];
        String method = "--typing-method=";

        argv[0] = (char *) command;
        if (typing_method && *typing_method) {
            method += typing_method;
            argv[1] = (char *) method.c_str();
        } else {
            argv[1] = NULL;
        }

        if (save) {
            argv[2] = NULL;
        } else {
            if (argv[1]) {
                argv[2] = "--no-save";
            } else {
                argv[1] = "--no-save";
                argv[2] = NULL;
            }
        }

        argv[3] = NULL;

        /* set pipe */
        close (out_fd[0]);
        close (err_fd[0]);
        close (in_fd[1]);
        close (report_fd[0]);
        fcntl (report_fd[1], F_SETFD, FD_CLOEXEC);

        int rv;

        rv = sane_dup2 (out_fd[1], STDOUT_FILENO);
        if (rv < 0)
            write_err_and_exit (report_fd[1], CHILD_DUP2_FAILD);

        rv = sane_dup2 (err_fd[1], STDERR_FILENO);
        if (rv < 0)
            write_err_and_exit (report_fd[1], CHILD_DUP2_FAILD);

        rv = sane_dup2 (in_fd[0],  STDIN_FILENO);
        if (rv < 0)
            write_err_and_exit (report_fd[1], CHILD_DUP2_FAILD);

        // exec
        execvp (argv[0], argv);

        // error
        write_err_and_exit (report_fd[1], CHILD_EXEC_FAILD);

        return false;
    }

ERROR4:
    close (report_fd[0]);
    close (report_fd[1]);
ERROR3:
    close (in_fd[0]);
    close (in_fd[1]);
ERROR2:
    close (err_fd[0]);
    close (err_fd[1]);
ERROR1:
    close (out_fd[0]);
    close (out_fd[1]);
ERROR0:
    return false;
}

void
PrimeConnection::close_connection (void)
{
    if (m_pid) {
        const char *command = PRIME_CLOSE "\n";
        size_t len, remaining;
        len = remaining = strlen (command);

        sig_t prev_handler = signal (SIGPIPE, handle_sigpipe);

#if 0
        bool rv;
        rv = write_all (m_in_fd, command, len);
        if (!rv) {
            if (m_err_msg.empty ())
                set_error_message (COMMUNICATION_FAILD, errno);
        }
#else
        while (remaining > 0) {
            ssize_t rv = write (m_in_fd,
                                command + (len - remaining),
                                remaining);
            if (rv < 0) {
                switch (errno) {
                case EBADF:
                case EINVAL:
                case EPIPE:
                    remaining = 0;
                    if (m_err_msg.empty ())
                        set_error_message (COMMUNICATION_FAILD, errno);
                    break;
                default:
                    break;
                }
            } else {
                remaining -= rv;
            }
        }
#endif

        if (prev_handler == SIG_ERR)
            signal (SIGPIPE, SIG_DFL);
        else
            signal (SIGPIPE, prev_handler);

        clean_child ();
    }
}

void
PrimeConnection::close_connection_with_error (void)
{
    clean_child ();
    set_error_message (COMMUNICATION_FAILD, errno);
}

void
PrimeConnection::clean_child (void)
{
    pid_t pid;
    do {
        pid = waitpid (-1, &m_exit_status, WNOHANG);
    } while (pid > 0);

    if (m_in_fd)
        close (m_in_fd);
    if (m_out_fd)
        close (m_out_fd);
    if (m_err_fd)
        close (m_err_fd);

    m_pid    = 0;
    m_in_fd  = -1;
    m_out_fd = -1;
    m_err_fd = -1;
}

bool
PrimeConnection::is_connected (void)
{
    if (m_pid)
        return true;
    else
        return false;
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

int
PrimeConnection::get_version_int (int idx)
{
    if (idx < 0 || idx > 2)
        return -1;

    bool success = send_command (PRIME_VERSION, NULL);
    if (success) {
        std::vector<String> versions;
        get_reply (versions, ".");
        if (versions.size() > (unsigned int) idx)
            return atoi (versions[idx].c_str());
        else
            return -1;
    } else {
        return -1;
    }
}

int
PrimeConnection::major_version (void)
{
    return get_version_int (0);
}

int
PrimeConnection::minor_version (void)
{
    return get_version_int (1);
}

int
PrimeConnection::micro_version (void)
{
    return get_version_int (2);
}

void
PrimeConnection::get_env (const String &key,
                          String &type, std::vector<String> &values)
{
    type = String ();
    values.clear ();

    bool success = send_command (PRIME_GET_ENV, key.c_str(), NULL);

    if (success) {
        get_reply (values, "\t");
        if (values.size () > 0) {
            type = values[0];
            values.erase (values.begin());
        }
    } else {
        type = "nil";
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
        PrimeSession *session = new PrimeSession(this,
                                                 m_last_reply.c_str(),
                                                 language);
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

void
PrimeConnection::preedit_convert_input (const String &pattern,
                                        WideString &preedition,
                                        WideString &pending)
{
    bool success = send_command (PRIME_PREEDIT_CONVERT_INPUT,
                                 pattern.c_str(), NULL);

    if (success) {
        std::vector<String> list;
        get_reply (list, "\t");
        if (list.size () > 0)
            m_iconv.convert (preedition, list[0]);
        if (list.size () > 1)
            m_iconv.convert (pending, list[1]);
    } else {
        // error
    }
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
        scim_prime_util_split_string (m_last_reply, rows, "\n");

        for (unsigned int i = 0; i < rows.size (); i++) {
            candidates.push_back (PrimeCandidate ());

            std::vector<String> cols;
            scim_prime_util_split_string (rows[i], cols, "\t");

            if (cols.size () >= 2) {
                m_iconv.convert (candidates[i].m_preedition, cols[0]);
                m_iconv.convert (candidates[i].m_conversion, cols[1]);
            }

            for (unsigned int j = 2; j < cols.size (); j++) {
                std::vector<String> pair;
                scim_prime_util_split_string (cols[j], pair, "=", 2);
                m_iconv.convert (candidates[i].m_values[pair[0]], pair[1]);
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

    if (!m_pid || m_in_fd < 0 || m_out_fd < 0)
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


    sig_t prev_handler = signal (SIGPIPE, handle_sigpipe);

    //
    // write the command
    //
#if 0
    bool rv;
    rv = write_all (m_in_fd, str.c_str (), str.length ());
    if (!rv) {
        clean_child ();
        if (m_err_msg.empty ())
            set_error_message (COMMUNICATION_FAILD, errno);
        goto ERROR;
    }
#else
    ssize_t rv;
    size_t len, remaining;
    len = remaining = str.length ();

    while (remaining > 0) {
        ssize_t rv = write (m_in_fd,
                            str.c_str() + (len - remaining),
                            remaining);
        if (rv < 0) {
            switch (errno) {
            case EBADF:
            case EINVAL:
            case EPIPE:
                clean_child ();
                if (m_err_msg.empty ())
                    set_error_message (COMMUNICATION_FAILD, errno);
                goto ERROR;
                break;
            default:
                break;
            }
        } else {
            remaining -= rv;
        }
    }
#endif


    //
    // read the reply
    //
    m_last_reply = String ();

    char buf[4096];
    while (true) {
        rv = read (m_out_fd, buf, 4095);
        if (rv < 0) {
            switch (errno) {
            case EBADF:
            case EINVAL:
            case EPIPE:
                clean_child ();
                if (m_err_msg.empty ())
                    set_error_message (COMMUNICATION_FAILD, errno);
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

        if (!m_pid || m_in_fd < 0 || m_out_fd < 0)
            goto ERROR;
    }

    if (m_last_reply.length () > 3 && m_last_reply.substr (0, 3) == "ok\n") {
        m_last_reply.erase (0, 3);

        if (prev_handler == SIG_ERR)
            signal (SIGPIPE, SIG_DFL);
        else
            signal (SIGPIPE, prev_handler);

        return true;
    }

    if (m_last_reply.length () > 6 && m_last_reply.substr (0, 6) == "error\n") {
        m_last_reply.erase (0, 6);
        goto ERROR;
    }

ERROR:
    if (prev_handler == SIG_ERR)
        signal (SIGPIPE, SIG_DFL);
    else
        signal (SIGPIPE, prev_handler);
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
PrimeConnection::get_reply (std::vector<String> &str_list, char *delim, int num)
{
    scim_prime_util_split_string (m_last_reply, str_list, delim, num);
}

void
PrimeConnection::get_error_message (WideString &msg)
{
    msg = m_err_msg;
}


bool
PrimeConnection::write_all (int fd, const char *buf, int size)
{
    int remaining = size;
    if (fd < 0)
        return false;

    while (remaining > 0) {
        int rv = write (fd, buf + (size - remaining), remaining);
        switch (errno) {
        case EBADF:
        case EINVAL:
        case EPIPE:
            return false;
        default:
            remaining -= rv;
            break;
        }
    }

    return true;
}

void
PrimeConnection::write_err_and_exit (int fd, int msg)
{
  int en = errno;

  write_all (fd, (const char *) &msg, sizeof(msg));
  write_all (fd, (const char *) &en,  sizeof(en));

  _exit (255);
}

bool
PrimeConnection::set_error_message (int erridx, int syserr)
{
    String msg_locale = strerror (syserr), enc;
    IConvert aiconv;
    WideString syserr_msg_wide;
    String syserr_msg;

    if (erridx == 0)
        return true;

    enc = scim_get_locale_encoding (scim_get_current_locale ());
    aiconv.set_encoding (enc);

    aiconv.convert (syserr_msg_wide, msg_locale);
    syserr_msg = utf8_wcstombs (syserr_msg_wide);

    switch (erridx) {
    case PIPE_FAILD:
    {
        String format = _("Failed to create pipe (%s)");
        char buf[format.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    case FORK_FAILD:
    {
        String format = _("Failed to create child process (%s)");
        char buf[format.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    case CHILD_DUP2_FAILD:
    {
        String format = _("Failed to redirect output or input of child process (%s)");
        char buf[format.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    case CHILD_EXEC_FAILD:
    {
        String format = _("Failed to execute child process \"%s\" (%s)");
        char buf[format.length () + m_command.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), m_command.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    case COMMUNICATION_FAILD:
    {
        String format = _("Failed to communicate with PRIME (%s)");
        char buf[format.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    default:
    {
        String format = _("An error occured (%s)");
        char buf[format.length () + syserr_msg.length () + 1];
        sprintf (buf, format.c_str (), syserr_msg.c_str ());
        m_err_msg = utf8_mbstowcs (buf);
        return false;
    }
    }

    return true;
}

bool
PrimeConnection::check_child_err (int fd)
{
    if (fd < 0)
        return false;

    int buf[2];
    int rv = true, n_bytes;

    buf[0] = 0;
    buf[1] = 0;

    n_bytes = read (fd, buf, sizeof (int) * 2);

    if (n_bytes >= (int) (sizeof (int) * 2) && buf[0] != 0)
        rv = set_error_message (buf[0], buf[1]);

    return rv;
}


static int
sane_dup2 (int fd1, int fd2)
{
    int ret;

retry:
    ret = dup2 (fd1, fd2);
    if (ret < 0 && errno == EINTR)
        goto retry;

    return ret;
}

static void
handle_sigpipe (int signo)
{
    int status;
    pid_t pid;

    while ((pid = waitpid (-1, &status, WNOHANG)) > 0) {
        std::vector<PrimeConnection *>::iterator it;
        for (it = connection_list.begin ();
             it != connection_list.end ();
             it++)
        {
            if (((*it)->get_connection_type() == PRIME_CONNECTION_PIPE) &&
                ((*it)->get_child_pid() == pid))
            {
                (*it)->close_connection_with_error ();
            }
        }
    };
}


void
scim_prime_util_split_string (String &str, std::vector<String> &str_list,
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
