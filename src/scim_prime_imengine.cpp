/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 - 2005 Takuro Ashie <ashie@homa.ne.jp>
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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_prime_imengine_factory.h"
#include "scim_prime_imengine.h"
#include "scim_prime_prefs.h"
#include "prime_connection.h"
#include "prime_session.h"
#include "intl.h"

PrimeConnection PrimeInstance::m_prime = PrimeConnection();

PrimeInstance::PrimeInstance (PrimeFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_session (NULL),
      m_factory (factory),
      m_prev_key (0,0),
      m_converting (false),
      m_modifying (false),
      m_registering (false),
      m_registering_cursor (0)
{
    SCIM_DEBUG_IMENGINE(1) << "Create PRIME Instance : ";

    m_prime.open_connection (m_factory->m_command.c_str(),
                             m_factory->m_typing_method.c_str());
}

PrimeInstance::~PrimeInstance ()
{
    if (m_session) {
        m_prime.session_end (m_session);
        delete m_session;
        m_session = NULL;
    }
}

bool
PrimeInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    // ignore key release.
    if (key.is_key_release ()) {
        if (m_factory->m_predict_on_preedition)
            set_prediction ();
        return true;
    }

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    if (get_session ()) {
        // lookup user defined key binds
        if (process_key_event_lookup_keybind (key))
            return true;

        return process_remaining_key_event (key);
    } else {
        return false;
    }
}

bool
PrimeInstance::process_key_event_lookup_keybind (const KeyEvent& key)
{
    std::vector<PrimeAction>::iterator it = m_factory->m_actions.begin();
    for (; it != m_factory->m_actions.end(); it++) {
        if ((*it).perform (this, key))
            return true;
    }

    return false;
}

bool
PrimeInstance::process_remaining_key_event (const KeyEvent &key)
{
    if (key.mask & SCIM_KEY_ControlMask ||
        key.mask & SCIM_KEY_Mod1Mask ||
        key.mask & SCIM_KEY_Mod2Mask ||
        key.mask & SCIM_KEY_Mod3Mask ||
        key.mask & SCIM_KEY_Mod4Mask ||
        key.mask & SCIM_KEY_Mod5Mask)
    {
        return false;
    }

    // FIXME!: is it correct?
    if (!is_registering () && isspace (key.get_ascii_code ()))
        return false;

    if (get_session () && isprint (key.get_ascii_code ())) {
        if (is_converting () ||
            (isupper (key.get_ascii_code ()) && m_factory->m_commit_on_upper))
        {
            action_commit (true);
        }

        char buf[2];
        buf[0] = key.get_ascii_code ();
        buf[1] = '\0';

        get_session()->edit_insert (buf);
        set_preedition ();

        return true;
    }

    return false;
}

void
PrimeInstance::move_preedit_caret (unsigned int pos)
{
}

void
PrimeInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    select_candidate_no_direct (item);

    if (m_factory->m_close_cand_win_on_select) {
        unsigned int pos = m_lookup_table.get_cursor_pos ();
        update_preedit_caret (m_candidates[pos].m_conversion.length ());
        hide_lookup_table ();
    }
}

void
PrimeInstance::select_candidate_no_direct (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate_no_direct.\n";

    if (!get_session ())
        return;

    show_lookup_table ();

    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);

    WideString selected_string;
    if (is_modifying ())
        get_session()->segment_select (m_lookup_table.get_cursor_pos ());
    else
        get_session()->conv_select (selected_string,
                                    m_lookup_table.get_cursor_pos ());

    set_preedition ();
}

void
PrimeInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
PrimeInstance::lookup_table_page_up ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();
    update_lookup_table (m_lookup_table);
}

void
PrimeInstance::lookup_table_page_down ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();
    update_lookup_table (m_lookup_table);
}

void
PrimeInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";

    m_registering        = false;
    m_query_string       = String ();
    m_registering_key    = WideString ();
    m_registering_value  = WideString ();
    m_registering_cursor = 0;

    m_candidates.clear();
    m_converting = false;
    m_modifying = false;

    if (get_session())
        get_session()->edit_erase();
    m_lookup_table.clear ();
    update_preedit_caret (0);
    update_preedit_string (utf8_mbstowcs (""));
    hide_lookup_table ();
    hide_preedit_string ();
}

void
PrimeInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    register_properties (m_properties);
}

void
PrimeInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
PrimeInstance::trigger_property (const String &property)
{
    String prime_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << prime_prop << "\n";
}

PrimeSession *
PrimeInstance::get_session (void)
{
    if (!m_session)
        m_session = m_prime.session_start ();

    // FIXME! check the connection

    if (!m_session) {
        reset ();
        // FIXME! set the flag
        // FIXME! show error message
    }

    return m_session;
}

void
PrimeInstance::set_preedition (void)
{
    // FIXME! check the session

    if (is_registering ()) {
        set_preedition_on_register ();

    } else if (is_modifying ()) {
        WideString left, cursor, right;
        get_session()->modify_get_conversion (left, cursor, right);

        AttributeList attr_list;
        Attribute attr (left.length (), cursor.length (), SCIM_ATTR_DECORATE);
        attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        attr_list.push_back (attr);

        update_preedit_string (left + cursor + right, attr_list);
        update_preedit_caret (left.length ());
        show_preedit_string ();

    } else if (is_converting ()) {
        int pos = m_lookup_table.get_cursor_pos ();
        PrimeCandidate &cand = m_candidates[pos];

        AttributeList attr_list;
        Attribute attr (0, cand.m_conversion.length(), SCIM_ATTR_DECORATE);
        attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        attr_list.push_back (attr);

        update_preedit_string (cand.m_conversion, attr_list);
        update_preedit_caret (0);
        show_preedit_string ();

    } else if (get_session ()) {
        WideString left, cursor, right;
        get_session()->edit_get_preedition (left, cursor, right);

        update_preedit_string (left + cursor + right);
        update_preedit_caret (left.length ());

        if (left.length () + cursor.length () + right.length () > 0)
            show_preedit_string ();
        else
            hide_preedit_string ();

    } else {
        update_preedit_string (WideString());
        update_preedit_caret (0);
        hide_preedit_string ();
    }
}

#define ADD_SEPARATOR_ATTR()                    \
{                                               \
    attr.set_start (str.length ());             \
    attr.set_length (tmp.length ());            \
    attr.set_type (SCIM_ATTR_FOREGROUND);       \
    attr.set_value (SCIM_RGB_COLOR(255, 0, 0)); \
    str += tmp;                                 \
    attr_list.push_back (attr);                 \
}

void
PrimeInstance::set_preedition_on_register (void)
{
    AttributeList attr_list;
    int pos;

    WideString str = utf8_mbstowcs (_("Register a word:"));
    Attribute attr (0, str.length (), SCIM_ATTR_DECORATE);
    attr.set_value (SCIM_ATTR_DECORATE_HIGHLIGHT);
    attr_list.push_back (attr);

    WideString tmp = utf8_mbstowcs (_("["));
    ADD_SEPARATOR_ATTR();

    str += m_registering_key;

    tmp = utf8_mbstowcs (_("|"));
    ADD_SEPARATOR_ATTR();

    str += m_registering_value.substr (0, m_registering_cursor);
    pos = str.length ();

    if (is_modifying ()) {
        WideString left, cursor, right;
        get_session()->modify_get_conversion (left, cursor, right);

        attr.set_type (SCIM_ATTR_DECORATE);
        attr.set_value (SCIM_ATTR_DECORATE_HIGHLIGHT);
        attr.set_start (pos);
        attr.set_length (left.length ());
        attr_list.push_back (attr);

        attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        attr.set_start (pos + left.length ());
        attr.set_length (cursor.length ());
        attr_list.push_back (attr);

        attr.set_value (SCIM_ATTR_DECORATE_HIGHLIGHT);
        attr.set_start (pos + left.length () + cursor.length ());
        attr.set_length (right.length ());
        attr_list.push_back (attr);

        pos += left.length ();
        str += left + cursor + right;

    } else if (is_converting ()) {
        int candpos = m_lookup_table.get_cursor_pos ();
        str += m_candidates[candpos].m_conversion;

        attr.set_type (SCIM_ATTR_DECORATE);
        attr.set_value (SCIM_ATTR_DECORATE_REVERSE);
        attr.set_start (pos);
        attr.set_length (m_candidates[candpos].m_conversion.length ());
        attr_list.push_back (attr);

    } else {
        WideString left, cursor, right, all;
        get_session()->edit_get_preedition (left, cursor, right);

        attr.set_type (SCIM_ATTR_DECORATE);
        attr.set_value (SCIM_ATTR_DECORATE_HIGHLIGHT);
        attr.set_start (pos);
        attr.set_length (left.length () + cursor.length () + right.length ());
        attr_list.push_back (attr);

        pos += left.length ();
        str += left + cursor + right;
    }

    str += m_registering_value.substr (
        m_registering_cursor,
        m_registering_value.length () - m_registering_cursor);

    tmp = utf8_mbstowcs (_("]"));
    ADD_SEPARATOR_ATTR();

    update_preedit_string (str, attr_list);
    update_preedit_caret (pos);
    show_preedit_string ();
}

#undef ADD_SEPARATOR_ATTR

void
PrimeInstance::set_prediction (void)
{
    if (!m_factory->m_predict_on_preedition)
        return;

    if (is_converting () || is_modifying ())
        return;

    if (!get_session ())
        return;

    m_lookup_table.clear ();

    PrimeCandidates candidates;
    get_session()->conv_predict (candidates);

    if (is_preediting () &&
        candidates.size () > 0 &&
        candidates[0].m_conversion.length () > 0)
    {
        for (unsigned int i = 0; i < candidates.size (); i++) {
            m_lookup_table.append_candidate (candidates[i].m_conversion);
        }
        m_lookup_table.show_cursor (false);
        update_lookup_table (m_lookup_table);
        show_lookup_table ();

    } else {
        hide_lookup_table ();
        update_lookup_table (m_lookup_table);
    }
}

bool
PrimeInstance::is_preediting (void)
{
    return get_session() && get_session()->has_preedition ();
}

bool
PrimeInstance::is_converting (void)
{
    return m_converting;
}

bool
PrimeInstance::is_modifying (void)
{
    return m_modifying;
}

bool
PrimeInstance::is_registering (void)
{
    return m_registering;
}

bool
PrimeInstance::action_commit (bool learn)
{
    if (is_registering ()) {
        return action_commit_on_register (learn);

    } else if (is_modifying ()) {
        WideString left, cursor, right;
        get_session()->modify_get_conversion (left, cursor, right);
#warning FIXME! commit segment
        commit_string (left + cursor + right);

        reset ();

    } else if (is_converting ()) {
        WideString cand, sel;
        get_session()->conv_select (sel, m_lookup_table.get_cursor_pos ());
        if (learn)
            get_session()->conv_commit (cand);
        commit_string (cand);

        reset ();

    } else if (is_preediting ()) {
        WideString left, cursor, right;
        get_session()->edit_get_preedition (left, cursor, right);
        commit_string (left + cursor + right);

        reset ();

    } else {
        return false;
    }

    return true;
}

bool
PrimeInstance::action_commit_on_register (bool learn)
{
    if (!is_registering ())
        return false;

    if (is_modifying ()) {
        WideString left, cursor, right, all;
        get_session()->modify_get_conversion (left, cursor, right);
        all = left + cursor + right;
        m_registering_value.insert (m_registering_cursor, all);
        m_registering_cursor += all.length ();

        action_finish_selecting_candidates ();
        m_modifying = false;
        get_session()->edit_erase();
        set_preedition ();

    } else if (is_converting ()) {
        WideString cand, sel;
        int pos = m_lookup_table.get_cursor_pos ();

        get_session()->conv_select (sel, pos);
        if (learn)
            get_session()->conv_commit (cand);

        m_registering_value.insert (m_registering_cursor, cand);
        m_registering_cursor += cand.length ();

        action_finish_selecting_candidates ();
        get_session()->edit_erase();
        set_preedition ();

    } else if (is_preediting ()) {
        WideString left, cursor, right, all;
        get_session()->edit_get_preedition (left, cursor, right);
        all = left + cursor + right;
        m_registering_value.insert (m_registering_cursor, all);
        m_registering_cursor += all.length ();

        action_finish_selecting_candidates ();
        get_session()->edit_erase();
        set_preedition ();

    } else {
        if (m_registering_key.length () > 0 &&
            m_registering_value.length () > 0)
        {
            m_prime.learn_word (m_registering_key, m_registering_value,
                                WideString (), WideString (),
                                WideString (), WideString ());
        }

        commit_string (m_registering_value);

        reset ();
    }

    return true;
}

bool
PrimeInstance::action_commit_with_learn (void)
{
    return action_commit (true);
}

bool
PrimeInstance::action_commit_without_learn (void)
{
    return action_commit (false);
}

bool
PrimeInstance::action_convert (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    m_converting = true;

    m_lookup_table.clear ();

    int idx = 0;

    if (is_modifying ())
        get_session()->segment_reconvert (m_candidates);
    else
        get_session()->conv_convert (m_candidates);

    for (unsigned int i = 0; i < m_candidates.size (); i++) {
        WideString label;
        get_candidate_label (label, m_candidates[i]);
        m_lookup_table.append_candidate (label);
    }

    if (m_candidates.size () > 0) {
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
        select_candidate_no_direct (idx);
    } else {
        m_converting = false;
        hide_lookup_table ();
    }

    set_preedition ();

    return true;
}

bool
PrimeInstance::action_revert (void)
{
    if (!is_preediting () && !is_registering ())
        return false;

    if (is_registering ()) {
        if (is_converting ()) {
            action_finish_selecting_candidates ();

        } else if (is_modifying ()) {
            m_modifying = false;
            set_preedition ();

        } else if (is_preediting ()) {
            get_session()->edit_erase ();
            m_lookup_table.clear ();
            hide_lookup_table ();
            set_preedition ();
        } else {
            String query_string = m_query_string;
            reset (); 
            get_session()->edit_insert (query_string.c_str());
            set_preedition ();
        }

    } else if (is_converting ()) {
        action_finish_selecting_candidates ();

    } else if (is_modifying ()) {
        m_modifying = false;
        set_preedition ();

    } else {
        reset ();
    }

    return true;
}

bool
PrimeInstance::action_finish_selecting_candidates (void)
{
    if (!is_converting ())
        return false;

    m_lookup_table.clear ();
    hide_lookup_table ();
    m_candidates.clear();
    m_converting = false;
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_left (void)
{
    if (is_registering () && !is_preediting ()) {
        if (m_registering_cursor > 0) {
            m_registering_cursor--;
            set_preedition ();
        }
        return true;
    }

    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;
    if (is_modifying ())
        return false;

    get_session()->edit_cursor_left ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_right (void)
{
    if (is_registering () && !is_preediting ()) {
        if (m_registering_cursor < m_registering_value.length ()) {
            m_registering_cursor++;
            set_preedition ();
        }
        return true;
    }

    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;
    if (is_modifying ())
        return false;

    get_session()->edit_cursor_right ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_left_edge (void)
{
    if (is_registering () && !is_preediting ()) {
        m_registering_cursor = 0;
        set_preedition ();
        return true;
    }

    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;
    if (is_modifying ())
        return false;

    get_session()->edit_cursor_left_edge ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_right_edge (void)
{
    if (is_registering () && !is_preediting ()) {
        m_registering_cursor = m_registering_value.length ();
        set_preedition ();
        return true;
    }

    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;
    if (is_modifying ())
        return false;

    get_session()->edit_cursor_right_edge ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_edit_backspace (void)
{
    if (is_registering () && !is_preediting ()) {
        if (m_registering_cursor > 0) {
            m_registering_value.erase (m_registering_cursor - 1, 1);
            m_registering_cursor--;
            set_preedition ();
            return true;
        }
    }

    if (!is_preediting ())
        return false;

    if (is_converting ()) {
        action_revert ();
        return true;
    }

    get_session()->edit_backspace ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_edit_delete (void)
{
    if (is_registering () && !is_preediting ()) {
        if (m_registering_cursor < m_registering_value.length ()) {
            m_registering_value.erase (m_registering_cursor, 1);
            set_preedition ();
            return true;
        }
    }

    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    get_session()->edit_delete ();
    set_preedition();

    return true;
}

bool
PrimeInstance::action_insert_space (void)
{
    if (is_registering ())
        return false;

    if (is_preediting ())
        return false;

    commit_string (utf8_mbstowcs (m_factory->m_space_char));

    return true;
}

bool
PrimeInstance::action_insert_alternative_space (void)
{
    if (is_registering ())
        return false;

    if (is_preediting ())
        return false;

    commit_string (utf8_mbstowcs (m_factory->m_alt_space_char.c_str()));

    return true;
}

bool
PrimeInstance::action_conv_next_candidate (void)
{
    if (is_registering () && !is_preediting ()) {
        action_revert ();
        return true;
    }

    if (!is_converting ())
        return false;

    int last_candidate = m_lookup_table.number_of_candidates () - 1;

    if (m_lookup_table.get_cursor_pos () == last_candidate) {
        if (m_factory->m_auto_register && !is_registering ())
            return action_register_a_word ();
        else
            m_lookup_table.set_cursor_pos (0);
    } else {
        m_lookup_table.cursor_down ();
    }

    select_candidate_no_direct (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
PrimeInstance::action_conv_prev_candidate (void)
{
    if (is_registering () && !is_preediting ()) {
        action_revert ();
        return true;
    }

    if (!is_converting ())
        return false;

    if (m_lookup_table.get_cursor_pos () == 0)
        m_lookup_table.set_cursor_pos (m_lookup_table.number_of_candidates () - 1);
    else
        m_lookup_table.cursor_up ();

    select_candidate_no_direct (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
PrimeInstance::action_conv_next_page (void)
{
    if (!is_converting ())
        return false;

    m_lookup_table.page_down ();
    select_candidate_no_direct (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
PrimeInstance::action_conv_prev_page (void)
{
    if (!is_converting ())
        return false;

    m_lookup_table.page_up ();
    select_candidate_no_direct (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
PrimeInstance::action_select_candidate (unsigned int i)
{
    if (!is_converting ())
        return false;

    select_candidate (i);

    return true;
}

bool
PrimeInstance::action_select_candidate_1 (void)
{
    return action_select_candidate (0);
}

bool
PrimeInstance::action_select_candidate_2 (void)
{
    return action_select_candidate (1);
}

bool
PrimeInstance::action_select_candidate_3 (void)
{
    return action_select_candidate (2);
}

bool
PrimeInstance::action_select_candidate_4 (void)
{
    return action_select_candidate (3);
}

bool
PrimeInstance::action_select_candidate_5 (void)
{
    return action_select_candidate (4);
}

bool
PrimeInstance::action_select_candidate_6 (void)
{
    return action_select_candidate (5);
}

bool
PrimeInstance::action_select_candidate_7 (void)
{
    return action_select_candidate (6);
}


bool
PrimeInstance::action_select_candidate_8 (void)
{
    return action_select_candidate (7);
}

bool
PrimeInstance::action_select_candidate_9 (void)
{
    return action_select_candidate (8);
}

bool
PrimeInstance::action_select_candidate_10 (void)
{
    return action_select_candidate (9);
}

bool
PrimeInstance::action_modify_start (void)
{
    if (!is_converting () && !is_modifying ())
        return false;

    if (!is_modifying ()) {
        get_session()->modify_start ();
        m_modifying = true;
    }

    return true;
}

bool
PrimeInstance::action_select_prev_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_left ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_select_next_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_right ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_select_first_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_left_edge ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_select_last_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_right_edge ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_shrink_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_shrink ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_expand_segment (void)
{
    if (!action_modify_start ())
        return false;

    get_session()->modify_cursor_expand ();
    if (!action_finish_selecting_candidates ())
        set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_default (void)
{
    if (is_converting ())
        action_revert ();

    get_session()->edit_set_mode (PRIME_PREEDITION_DEFAULT);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_katakana (void)
{
    if (is_converting ())
        action_revert ();

    get_session()->edit_set_mode (PRIME_PREEDITION_KATAKANA);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_half_katakana (void)
{
    if (is_converting ())
        action_revert ();

    get_session()->edit_set_mode (PRIME_PREEDITION_HALF_KATAKANA);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_raw (void)
{
    if (is_converting ())
        action_revert ();

    get_session()->edit_set_mode (PRIME_PREEDITION_RAW);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_wide_ascii (void)
{
    if (is_converting ())
        action_revert ();

    get_session()->edit_set_mode (PRIME_PREEDITION_WIDE_ASCII);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_toggle_language (void)
{
    reset ();

    if (!m_session) {
        m_session = m_prime.session_start ();
        return true;
    }

    String key = "language", type;
    std::vector<String> list;
    get_session()->get_env (key, type, list);

    // FIXME! should be stored using map container
    m_prime.session_end (m_session);
    delete m_session;
    m_session = NULL;

    if (list.size() <= 0) {
        m_session = m_prime.session_start ();
        return true;
    }

    if (list[0] == "English") {
        m_session = m_prime.session_start ("Japanese");
    } else if (list[0] == "Japanese") {
        m_session = m_prime.session_start ("English");
    } else {
        m_session = m_prime.session_start ("Japanese");
    }

    return true;
}

bool
PrimeInstance::action_register_a_word (void)
{
    // FIXME!
    if (!is_preediting ())
        return false;

    if (is_converting ())
        action_revert ();

    get_session()->edit_get_query_string (m_query_string);

    WideString left, cursor, right;
    get_session()->edit_get_preedition (left, cursor, right);
    m_registering_key = left + cursor + right;

    m_registering = true;

    get_session()->edit_erase();
    action_finish_selecting_candidates ();
    set_preedition ();

    return true;
}

bool
PrimeInstance::match_key_event (const KeyEventList &keys, const KeyEvent &key) const
{
    KeyEventList::const_iterator kit;

    for (kit = keys.begin (); kit != keys.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
             return true;
    }
    return false;
}

void
PrimeInstance::get_candidate_label (WideString &label, PrimeCandidate &cand)
{
    label = cand.m_conversion;

    if (m_factory->m_show_annotation &&
        cand.m_values["form"].length () > 0)
    {
        label += utf8_mbstowcs ("  (");
        label += cand.m_values["form"];
        label += utf8_mbstowcs (" )");
    }

    if (m_factory->m_show_usage &&
        cand.m_values["usage"].length () > 0)
    {
        label += utf8_mbstowcs ("\t\xE2\x96\xBD");
        label += cand.m_values["usage"];
    }

    if (m_factory->m_show_comment &&
        cand.m_values["comment"].length () > 0)
    {
        label += utf8_mbstowcs ("\t<");
        label += cand.m_values["comment"];
        label += utf8_mbstowcs (">");
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
