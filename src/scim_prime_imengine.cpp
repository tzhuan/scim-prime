/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2004 Takuro Ashie <ashie@homa.ne.jp>
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
#include "scim_prime_imengine.h"
#include "scim_prime_prefs.h"

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#define scim_module_init prime_LTX_scim_module_init
#define scim_module_exit prime_LTX_scim_module_exit
#define scim_imengine_module_init prime_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory prime_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_PRIME_UUID     "/IMEngine/PRIME/UUID-"

#define SCIM_PROP_PREFIX                    "/IMEngine/PRIME"

#ifndef SCIM_PRIME_ICON_FILE
    #define SCIM_PRIME_ICON_FILE           (SCIM_ICONDIR"/scim-prime.png")
#endif

static ConfigPointer _scim_config (0);
PrimeConnection PrimeInstance::m_prime = PrimeConnection();

extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_PRIME_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE(1) << "Initialize PRIME Engine.\n";

        _scim_config = config;

        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        PrimeFactory *factory = 0;

        try {
            factory = new PrimeFactory (String ("ja_JP"),
                                        String ("53ad00ca-0eb7-4495-bec4-7cdb52f367d7"),
                                        _scim_config);
        } catch (...) {
            delete factory;
            factory = 0;
        }

        return factory;
    }
}

PrimeFactory::PrimeFactory (const String &lang,
                            const String &uuid,
                            const ConfigPointer &config)
    : m_uuid (uuid),
      m_config (config)
{
    SCIM_DEBUG_IMENGINE(1) << "Create PRIME Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);
}

PrimeFactory::~PrimeFactory ()
{
}

WideString
PrimeFactory::get_name () const
{
    return utf8_mbstowcs (String ("PRIME"));
}

WideString
PrimeFactory::get_authors () const
{
    return WideString ();
}

WideString
PrimeFactory::get_credits () const
{
    return WideString ();
}

WideString
PrimeFactory::get_help () const
{
    return WideString ();
}

String
PrimeFactory::get_uuid () const
{
    return m_uuid;
}

String
PrimeFactory::get_icon_file () const
{
    return String ();
}

IMEngineInstancePointer
PrimeFactory::create_instance (const String &encoding, int id)
{
    return new PrimeInstance (this, encoding, id);
}

PrimeInstance::PrimeInstance (PrimeFactory   *factory,
                              const String   &encoding,
                              int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_session (NULL),
      m_factory (factory),
      m_prev_key (0,0),
      m_converting (false)
{
    SCIM_DEBUG_IMENGINE(1) << "Create PRIME Instance : ";

    m_prime.open_connection ();
    m_session = m_prime.session_start ();
}

void
PrimeFactory::reload_config (const ConfigPointer &config)
{
    if (config) {
    }
}

PrimeInstance::~PrimeInstance ()
{
    m_prime.session_end (m_session);
    delete m_session;
    m_session = NULL;
}

bool
PrimeInstance::process_key_event_lookup_keybind (const KeyEvent& key)
{
    return false;
}

bool
PrimeInstance::process_key_event_without_preedit (const KeyEvent& key)
{
    return process_remaining_key_event (key);
}

bool
PrimeInstance::process_key_event_with_preedit (const KeyEvent& key)
{
    switch (key.code) {
    case SCIM_KEY_Escape:
        if (action_revert ())
            return true;
        break;

    case SCIM_KEY_Return:
        if (action_commit ())
            return true;
        break;

    case SCIM_KEY_space:
        if (action_convert ())
            return true;
        break;

    case SCIM_KEY_BackSpace:
        if (action_back ())
            return true;
        break;

    case SCIM_KEY_Delete:
        if (action_delete ())
            return true;
        break;

    case SCIM_KEY_Left:
        if (action_move_caret_backward ())
            return true;
        break;

    case SCIM_KEY_Right:
        if (action_move_caret_forward ())
            return true;
        break;

    default:
        break;
    }

    return process_remaining_key_event (key);
}

bool
PrimeInstance::process_key_event_with_candidate (const KeyEvent &key)
{
    switch (key.code) {
    case SCIM_KEY_Return:
        if (action_commit ())
            return true;
        break;

    case SCIM_KEY_Escape:
    case SCIM_KEY_BackSpace:
        if (action_revert ())
            return true;
        break;

    case SCIM_KEY_space:
    case SCIM_KEY_Down:
        if (action_select_next_candidate ())
            return true;
        break;

    case SCIM_KEY_Up:
        if (action_select_prev_candidate ())
            return true;
        break;

    default:
        break;
    }

    return process_remaining_key_event (key);
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

    if (isprint(key.get_ascii_code ())) {
        char buf[2];
        buf[0] = key.get_ascii_code ();
        buf[1] = '\0';

        if (m_session)
            m_session->edit_insert (buf);
        set_preedition();

        return true;
    }

    return false;
}

bool
PrimeInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";
    KeyEvent newkey;

    // ignore key release.
    if (key.is_key_release ()) {
        set_prediction ();
        return true;
    }

    // ignore modifier keys
    if (key.code == SCIM_KEY_Shift_L || key.code == SCIM_KEY_Shift_R ||
        key.code == SCIM_KEY_Control_L || key.code == SCIM_KEY_Control_R ||
        key.code == SCIM_KEY_Alt_L || key.code == SCIM_KEY_Alt_R)
        return false;

    // lookup user defined key binds
    if (process_key_event_lookup_keybind (key))
        return true;

    if (is_converting ())
        return process_key_event_with_candidate (key);
    else if (is_preediting ())
        return process_key_event_with_preedit (key);
    else
        return process_key_event_without_preedit (key);
}

void
PrimeInstance::move_preedit_caret (unsigned int pos)
{
}

void
PrimeInstance::select_candidate_no_direct (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate_no_direct.\n";

    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);

    set_preedition ();
}

void
PrimeInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    select_candidate_no_direct (item);
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

    m_converting = false;

    if (m_session)
        m_session->edit_erase();
    m_lookup_table.clear ();
    update_preedit_caret (0);
    update_preedit_string (utf8_mbstowcs (""));
    hide_lookup_table ();
    //hide_preedit_string ();
}

void
PrimeInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();
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

void
PrimeInstance::set_preedition (void)
{
    if (is_converting ()) {
        int pos = m_lookup_table.get_cursor_pos_in_current_page ();
        WideString cand = m_lookup_table.get_candidate_in_current_page (pos);
        update_preedit_string (cand);
        update_preedit_caret (0);
    } else if (m_session) {
        WideString left, cursor, right;
        m_session->edit_get_preedition (left, cursor, right);

        update_preedit_string (left + cursor + right);
        update_preedit_caret (left.length ());
    } else {
        update_preedit_string (WideString());
        update_preedit_caret (0);
    }
}

void
PrimeInstance::set_prediction (void)
{
    // prediction
    if (!is_converting () && m_session) {
        m_lookup_table.clear ();

        String query;
        m_session->edit_get_query_string (query);

        PrimeCandidate candidate;
        m_prime.lookup (query.c_str (), candidate);

        if (is_preediting () && candidate.m_conversion.length () > 0) {
            m_lookup_table.append_candidate (candidate.m_conversion);
            m_lookup_table.show_cursor (false);
            update_lookup_table (m_lookup_table);
            show_lookup_table ();
        } else {
            hide_lookup_table ();
            update_lookup_table (m_lookup_table);
        }
    }
}

bool
PrimeInstance::is_preediting (void)
{
    return m_session && m_session->has_preedition ();
}

bool
PrimeInstance::is_converting (void)
{
    return m_converting;
}

bool
PrimeInstance::is_learning (void)
{
    return false;
}

bool
PrimeInstance::action_commit (void)
{
    if (is_converting ()) {
        int pos = m_lookup_table.get_cursor_pos_in_current_page ();
        WideString cand = m_lookup_table.get_candidate_in_current_page (pos);
        commit_string (cand);
    } else if (is_preediting ()) {
        if (m_session) {
            WideString left, cursor, right, all;
            m_session->edit_get_preedition (left, cursor, right);
            commit_string (left + cursor + right);
            all = left + cursor + right;
            m_session->set_context (all);
        }
    } else {
        return false;
    }

    reset ();

    return true;
}

bool
PrimeInstance::action_convert (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session) {
        m_converting = true;

        m_lookup_table.clear ();

        String query;
        m_session->edit_get_query_string (query);

        std::vector<PrimeCandidate> candidates;
        m_prime.lookup_all (query.c_str (), candidates);
        for (unsigned int i = 0; i < candidates.size (); i++)
            m_lookup_table.append_candidate (candidates[i].m_conversion);

        m_lookup_table.set_cursor_pos (0);
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    }

    set_preedition();

    return true;
}

bool
PrimeInstance::action_revert (void)
{
    if (!is_preediting ())
        return false;

    if (is_converting ()) {
        m_lookup_table.clear ();
        hide_lookup_table ();
        m_converting = false;
        set_preedition ();
    } else {
        reset ();
    }

    return true;
}

bool
PrimeInstance::action_move_caret_backward (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_left ();

    set_preedition();

    return true;
}

bool
PrimeInstance::action_move_caret_forward (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_right ();
    
    set_preedition();

    return true;
}

bool
PrimeInstance::action_move_caret_first (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    return false;
}

bool
PrimeInstance::action_move_caret_last (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    return false;
}

bool
PrimeInstance::action_back (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_backspace ();

    set_preedition();

    return true;
}

bool
PrimeInstance::action_delete (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_delete ();

    set_preedition();

    return true;
}

bool
PrimeInstance::action_select_next_candidate (void)
{
    if (!is_converting ())
        return false;

    if (m_lookup_table.get_cursor_pos () == (int) (m_lookup_table.number_of_candidates () - 1))
        m_lookup_table.set_cursor_pos (0);
    else
        m_lookup_table.cursor_down ();

    select_candidate_no_direct (m_lookup_table.get_cursor_pos_in_current_page ());

    return true;
}

bool
PrimeInstance::action_select_prev_candidate (void)
{
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
PrimeInstance::match_key_event (const KeyEventList &keys, const KeyEvent &key) const
{
    KeyEventList::const_iterator kit;

    for (kit = keys.begin (); kit != keys.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
             return true;
    }
    return false;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
