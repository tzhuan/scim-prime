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


#ifndef __PRIME_SESSION_H__
#define __PRIME_SESSION_H__

#define Uses_SCIM_ICONV
#include <sys/types.h>
#include <unistd.h>
#include <scim.h>

using namespace scim;

typedef enum {
    PRIME_PREEDITION_DEFAULT,
    PRIME_PREEDITION_KATAKANA,
    PRIME_PREEDITION_HALF_KATAKANA,
    PRIME_PREEDITION_WIDE_ASCII,
    PRIME_PREEDITION_RAW,
} PrimePreeditionMode;

class PrimeConnection;

class PrimeSession
{
public:
    PrimeSession                  (PrimeConnection *connection,
                                   String           id_str,
                                   const char      *language);
    virtual ~PrimeSession         ();

public: // prime commands
    void        conv_predict             (PrimeCandidates &candidates,
                                          String method = String ());
    void        conv_convert             (PrimeCandidates &candidates,
                                          String method = String ());
    void        conv_select              (WideString &selected_string,
                                          int index);
    void        conv_commit              (WideString &commited_string);

    void        edit_backspace           (void);
    void        edit_commit              (void);
    void        edit_cursor_left         (void);
    void        edit_cursor_left_edge    (void);
    void        edit_cursor_right        (void);
    void        edit_cursor_right_edge   (void);
    void        edit_delete              (void);
    void        edit_erase               (void);
    void        edit_get_preedition      (WideString &left,
                                          WideString &cursor,
                                          WideString &right);
    void        edit_get_query_string    (String     &string);
    void        edit_insert              (const char *str);
    void        edit_set_mode            (PrimePreeditionMode mode);
    void        edit_undo                (void);

    void        modify_cursor_expand     (void);
    void        modify_cursor_left       (void);
    void        modify_cusror_left_edge  (void);
    void        modify_cursor_right      (void);
    void        modify_cursor_right_edge (void);
    void        modify_cursor_shrink     (void);
    void        modify_get_candidates    (void);
    void        modify_start             (void);

    void        segment_commit           (void);
    void        segment_reconvert        (void);
    void        segment_select           (int index);

    void        context_set_previous_word(WideString &word);
    void        context_reset            (void);

    void        get_env                  (const String        &key,
                                          String              &type,
                                          std::vector<String> &values);

public: // other functions
    bool        has_preedition           (void);
    bool        send_command             (const char *command,
                                          const char *arg2 = NULL);
    String     &get_id_str               (void);

private:
    void        get_candidates           (PrimeCandidates &candidates);

private:
    PrimeConnection *m_connection;
    String           m_id_str;
    String           m_language;
};

#endif /* __PRIME_SESSION_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
