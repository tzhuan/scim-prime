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

#include "prime_connection.h"
#include "prime_session.h"
#include "prime_commands.h"

PrimeSession::PrimeSession (PrimeConnection *connection, String id_str)
    : m_connection (connection),
      m_id_str (id_str)
{
}

PrimeSession::~PrimeSession ()
{
}

void
PrimeSession::edit_backspace (void)
{
    send_command (PRIME_EDIT_BACKSPACE);
}

void
PrimeSession::edit_commit (void)
{
    send_command (PRIME_EDIT_COMMIT);
}

void
PrimeSession::edit_cursor_left (void)
{
    send_command (PRIME_EDIT_CURSOR_LEFT);
}

void
PrimeSession::edit_cursor_left_edge (void)
{
    send_command (PRIME_EDIT_CURSOR_LEFT_EDGE);
}

void
PrimeSession::edit_cursor_right (void)
{
    send_command (PRIME_EDIT_CURSOR_RIGHT);
}

void
PrimeSession::edit_cursor_right_edge (void)
{
    send_command (PRIME_EDIT_CURSOR_RIGHT_EDGE);
}

void
PrimeSession::edit_delete (void)
{
    send_command (PRIME_EDIT_DELETE);
}

void
PrimeSession::edit_erase (void)
{
    send_command (PRIME_EDIT_ERASE);
}

void
PrimeSession::edit_get_preedition (WideString &left,
                                   WideString &cursor,
                                   WideString &right)
{
    bool success = send_command (PRIME_EDIT_GET_PREEDITION);

    std::vector<String> list;
    String preedition;

    if (success) {
        m_connection->get_reply (list, "\t");

        if (list.size () >= 1)
            m_connection->m_iconv.convert (left, list[0]);
        if (list.size () >= 2)
            m_connection->m_iconv.convert (cursor, list[1]);
        if (list.size () >= 3)
            m_connection->m_iconv.convert (right, list[2]);
    } else {
        // error
    }
}

void
PrimeSession::edit_get_query_string (String &string)
{
    bool success = send_command (PRIME_EDIT_GET_QUERY_STRING);

    if (success) {
        m_connection->get_reply (string);
    } else {
        // error
    }
}

void
PrimeSession::edit_insert (const char *str)
{
    send_command (PRIME_EDIT_INSERT, str);
}

void
PrimeSession::edit_set_mode (PrimePreeditionMode mode)
{
    char *command = "default";

    switch (mode) {
    case PRIME_PREEDITION_KATAKANA:
        command = "katakana";
        break;
    case PRIME_PREEDITION_HALF_KATAKANA:
        command = "half_katakana";
        break;
    case PRIME_PREEDITION_WIDE_ASCII:
        command = "wide_ascii";
        break;
    case PRIME_PREEDITION_RAW:
        command = "raw";
        break;
    case PRIME_PREEDITION_DEFAULT:
    default:
        command = "default";
        break;
    }

    send_command (PRIME_EDIT_SET_MODE, command);
}

void
PrimeSession::edit_undo (void)
{
    send_command (PRIME_EDIT_UNDO);
}

void
PrimeSession::segment_commit (void)
{
    send_command (PRIME_SEGMENT_COMMIT);
}

void
PrimeSession::segment_reconvert (void)
{
    send_command (PRIME_SEGMENT_RECONVERT);
}

void
PrimeSession::segment_select (int index)
{
    char buf[32];
    sprintf(buf, "%10d\n", index);
    send_command (PRIME_SEGMENT_SELECT, buf);
}

void
PrimeSession::context_set_previous_word(WideString &word)
{
    String str;
    m_connection->m_iconv.convert(str, word);
    send_command (PRIME_CONTEXT_SET_PREVIOUS_WORD, str.c_str());
}

void
PrimeSession::context_reset (void)
{
    send_command (PRIME_CONTEXT_RESET);
}

bool
PrimeSession::has_preedition (void)
{
    WideString left, cursor, right;

    edit_get_preedition (left, cursor, right);

    if (left.length () + cursor.length () + right.length () > 0)
        return true;
    else
        return false;
}

String &
PrimeSession::get_id_str (void)
{
    return m_id_str;
}

bool
PrimeSession::send_command (const char *command, const char *arg2)
{
    if (m_connection) {
        return m_connection->send_command (command, m_id_str.c_str (), arg2,
                                           NULL);
    } else {
        return false;
    }
}

