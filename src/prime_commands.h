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


#ifndef __PRIME_COMMANDS_H__
#define __PRIME_COMMANDS_H__

#define PRIME_CLOSE                  "close"                  // noargs

#define PRIME_SESSION_START          "session_start"          // arg1:LANGUAGE
#define PRIME_SESSION_END            "session_end"            // arg1:SESSION

#define PRIME_LOOKUP                 "lookup"                 // arg1:PATTERN
#define PRIME_LOOKUP_ALL             "lookup_all"             // arg1:PATTERN
#define PRIME_LOOKUP_COMPACT         "lookup_compact"         // arg1:PATTERN
#define PRIME_LOOKUP_COMPACT_ALL     "lookup_compact_all"     // arg1:PATTERN
#define PRIME_LOOKUP_DIRECT          "lookup_direct"          // arg1:PATTERN
#define PRIME_LOOKUP_DIRECT_ALL      "lookup_direct_all"      // arg1:PATTERN
#define PRIME_LOOKUP_EXPANSION       "lookup_expansion"       // arg1:PATTERN
#define PRIME_LOOKUP_HYBRID          "lookup_hybrid"          // arg1:PATTERN
#define PRIME_LOOKUP_HYBRID_ALL      "lookup_hybrid_all"      // arg1:PATTERN
#define PRIME_LOOKUP_MIXED           "lookup_mixed"           // arg1:PATTERN
#define PRIME_LOOKUP_PREFIX          "lookup_prefix"          // arg1:PATTERN
#define PRIME_LOOKUP_PREFIX_EX       "lookup_prefix_ex"       // arg1:PATTERN

#define PRIME_EDIT_COMMIT            "edit_commit"            // noargs
#define PRIME_EDIT_CURSOR_LEFT       "edit_cursor_left"       // arg1:SESSION
#define PRIME_EDIT_CURSOR_LEFT_EDGE  "edit_cursor_left_edge"  // arg1:SESSION
#define PRIME_EDIT_CURSOR_RIGHT      "edit_cursor_right"      // arg1:SESSION
#define PRIME_EDIT_CURSOR_RIGHT_EDGE "edit_cursor_right_edge" // arg1:SESSION
#define PRIME_EDIT_DELETE            "edit_delete"            // noargs
#define PRIME_EDIT_ERASE             "edit_erase"             // arg1:SESSION
#define PRIME_EDIT_GET_QUERY_STRING  "edit_get_query_string"  // arg1:SESSION
#define PRIME_EDIT_GET_PREEDITION    "edit_get_preedition"    // arg1:SESSION
#define PRIME_EDIT_BACKSPACE         "edit_backspace"         // arg1:SESSION
#define PRIME_EDIT_INSERT            "edit_insert"            // arg1:SESSION
                                                              // arg2:STRING

#define PRIME_LEARN_WORD             "learn_word"
#define PRIME_REGISTER_WORD          "register_word"          // arg1:SESSION
                                                              // arg2:READING
                                                              // arg3:LITERAL
                                                              // arg4:POS

#define PRIME_SET_CONTEXT            "set_context"            // CONTEXT
#define PRIME_RESET_CONTEXT          "reset_context"          // noargs

#endif /* __PRIME_COMMANDS_H__ */
