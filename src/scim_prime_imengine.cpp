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
      m_config (config),
      m_auto_register (SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT),
      m_commit_on_upper (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT),
      m_show_annotation (SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT),
      m_show_usage (SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT)
{
    SCIM_DEBUG_IMENGINE(1) << "Create PRIME Factory :\n";
    SCIM_DEBUG_IMENGINE(1) << "  Lang : " << lang << "\n";
    SCIM_DEBUG_IMENGINE(1) << "  UUID : " << uuid << "\n";

    if (lang.length () >= 2)
        set_languages (lang);

    // config
    reload_config (m_config);
    m_reload_signal_connection
        = m_config->signal_connect_reload (slot (this, &PrimeFactory::reload_config));
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
    return String (SCIM_PRIME_ICON_FILE);
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
      m_converting (false),
      m_registering (false)
{
    SCIM_DEBUG_IMENGINE(1) << "Create PRIME Instance : ";

    m_prime.open_connection (m_factory->m_command.c_str(),
                             m_factory->m_typing_method.c_str());
    m_session = m_prime.session_start ();
}

void
PrimeFactory::reload_config (const ConfigPointer &config)
{
    if (!config) return;

    String str;

    m_command
        = config->read (String (SCIM_PRIME_CONFIG_COMMAND),
                        String (SCIM_PRIME_CONFIG_COMMAND_DEFAULT));
    m_typing_method
        = config->read (String (SCIM_PRIME_CONFIG_TYPING_METHOD),
                        String (SCIM_PRIME_CONFIG_TYPING_METHOD_DEFAULT));
    m_auto_register
        = config->read (String (SCIM_PRIME_CONFIG_AUTO_REGISTER),
                        SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT);
    m_commit_on_upper
        = config->read (String (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER),
                        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT);
    m_show_annotation
        = config->read (String (SCIM_PRIME_CONFIG_SHOW_ANNOTATION),
                        SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT);
    m_show_usage
        = config->read (String (SCIM_PRIME_CONFIG_SHOW_USAGE),
                        SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT);

    // edit keys
    str = config->read (String (SCIM_PRIME_CONFIG_COMMIT_KEY),
                        String (SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT));
    scim_string_to_key_list (m_commit_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_CONVERT_KEY),
                        String (SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT));
    scim_string_to_key_list (m_convert_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_CANCEL_KEY),
                        String (SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT));
    scim_string_to_key_list (m_cancel_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_BACKSPACE_KEY),
                        String (SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT));
    scim_string_to_key_list (m_backspace_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_DELETE_KEY),
                        String (SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT));
    scim_string_to_key_list (m_delete_keys, str);

    // caret keys
    str = config->read (String (SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY),
                        String (SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT));
    scim_string_to_key_list (m_modify_caret_left_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY),
                        String (SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT));
    scim_string_to_key_list (m_modify_caret_right_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY),
                        String (SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT));
    scim_string_to_key_list (m_modify_caret_left_edge_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY),
                        String (SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT));
    scim_string_to_key_list (m_modify_caret_right_edge_keys, str);

    // candidate keys
    str = config->read (String (SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY),
                        String (SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT));
    scim_string_to_key_list (m_conv_next_candidate_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY),
                        String (SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT));
    scim_string_to_key_list (m_conv_prev_candidate_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY),
                        String (SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY_DEFAULT));
    scim_string_to_key_list (m_conv_next_page_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY),
                        String (SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY_DEFAULT));
    scim_string_to_key_list (m_conv_prev_page_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[0], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[1], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[2], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[3], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[4], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[5], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[6], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[7], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[8], str);

    str = config->read (String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY),
                        String (SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT));
    scim_string_to_key_list (m_select_candidate_keys[9], str);

    // mode keys
    str = config->read (String (SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY),
                        String (SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT));
    scim_string_to_key_list (m_set_mode_default_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY),
                        String (SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT));
    scim_string_to_key_list (m_set_mode_katakana_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY),
                        String (SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT));
    scim_string_to_key_list (m_set_mode_half_katakana_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY),
                        String (SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT));
    scim_string_to_key_list (m_set_mode_raw_keys, str);

    str = config->read (String (SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY),
                        String (SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT));
    scim_string_to_key_list (m_set_mode_wide_ascii_keys, str);

    // register a word
    str = config->read (String (SCIM_PRIME_CONFIG_REGISTER_WORD_KEY),
                        String (SCIM_PRIME_CONFIG_REGISTER_WORD_KEY_DEFAULT));
    scim_string_to_key_list (m_register_word_keys, str);
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
    // edit keys
    if (match_key_event (m_factory->m_commit_keys, key) &&
        action_commit ())
        return true;

    if (match_key_event (m_factory->m_convert_keys, key) &&
        action_convert ())
        return true;

    if (match_key_event (m_factory->m_cancel_keys, key) &&
        action_revert ())
        return true;

    if (match_key_event (m_factory->m_backspace_keys, key) &&
        action_edit_backspace ())
        return true;

    if (match_key_event (m_factory->m_delete_keys, key) &&
        action_edit_delete ())
        return true;

    // caret keys
    if (match_key_event (m_factory->m_modify_caret_left_keys, key) &&
        action_modify_caret_left ())
        return true;

    if (match_key_event (m_factory->m_modify_caret_right_keys, key) &&
        action_modify_caret_right ())
        return true;

    if (match_key_event (m_factory->m_modify_caret_left_edge_keys, key) &&
        action_modify_caret_left_edge ())
        return true;

    if (match_key_event (m_factory->m_modify_caret_right_edge_keys, key) &&
        action_modify_caret_right_edge ())
        return true;

    // candidate keys
    if (match_key_event (m_factory->m_conv_next_candidate_keys, key) &&
        action_conv_next_candidate ())
        return true;

    if (match_key_event (m_factory->m_conv_prev_candidate_keys, key) &&
        action_conv_prev_candidate ())
        return true;

    if (match_key_event (m_factory->m_conv_next_page_keys, key) &&
        action_conv_next_page ())
        return true;

    if (match_key_event (m_factory->m_conv_prev_page_keys, key) &&
        action_conv_prev_page ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[0], key) &&
        action_select_candidate_1 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[1], key) &&
        action_select_candidate_2 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[2], key) &&
        action_select_candidate_3 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[3], key) &&
        action_select_candidate_4 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[4], key) &&
        action_select_candidate_5 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[5], key) &&
        action_select_candidate_6 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[6], key) &&
        action_select_candidate_7 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[7], key) &&
        action_select_candidate_8 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[8], key) &&
        action_select_candidate_9 ())
        return true;

    if (match_key_event (m_factory->m_select_candidate_keys[9], key) &&
        action_select_candidate_10 ())
        return true;

    // mode keys
    if (match_key_event (m_factory->m_set_mode_default_keys, key) &&
        action_set_mode_default ())
        return true;

    if (match_key_event (m_factory->m_set_mode_katakana_keys, key) &&
        action_set_mode_katakana ())
        return true;

    if (match_key_event (m_factory->m_set_mode_half_katakana_keys, key) &&
        action_set_mode_half_katakana ())
        return true;

    if (match_key_event (m_factory->m_set_mode_raw_keys, key) &&
        action_set_mode_raw ())
        return true;

    if (match_key_event (m_factory->m_set_mode_wide_ascii_keys, key) &&
        action_set_mode_wide_ascii ())
        return true;

    // register a word
    if (match_key_event (m_factory->m_register_word_keys, key) &&
        action_register_a_word ())
        return true;

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
    return process_remaining_key_event (key);
}

bool
PrimeInstance::process_key_event_with_candidate (const KeyEvent &key)
{
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

    // FIXME!: does it correct?
    if (!is_registering () && isspace (key.get_ascii_code ()))
        return false;

    if (m_session && isprint (key.get_ascii_code ())) {
        if (is_converting () ||
            (isupper (key.get_ascii_code ()) && m_factory->m_commit_on_upper))
        {
            action_commit();
        }

        char buf[2];
        buf[0] = key.get_ascii_code ();
        buf[1] = '\0';

        m_session->edit_insert (buf);
        set_preedition ();

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

    if (m_session) {
        // lookup user defined key binds
        if (process_key_event_lookup_keybind (key))
            return true;

        if (is_converting ())
            return process_key_event_with_candidate (key);
        else if (is_preediting ())
            return process_key_event_with_preedit (key);
        else
            return process_key_event_without_preedit (key);
    } else {
        return false;
    }
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

    m_registering       = false;
    m_registering_key   = WideString ();
    m_registering_value = WideString ();

    m_candidates.clear();
    m_converting     = false;

    if (m_session)
        m_session->edit_erase();
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

#define ADD_SEPARATOR_ATTR() \
{ \
    attr.set_start (str.length ()); \
    attr.set_length (tmp.length ()); \
    attr.set_type (SCIM_ATTR_FOREGROUND); \
    attr.set_value (SCIM_RGB_COLOR(255, 0, 0)); \
    str += tmp; \
    attr_list.push_back (attr); \
}

void
PrimeInstance::set_preedition (void)
{
    if (is_registering ()) {
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

        str += m_registering_value;

        pos = str.length ();

        if (is_converting ()) {
            int candpos = m_lookup_table.get_cursor_pos ();
            str += m_candidates[candpos].m_conversion;
        } else {
            WideString left, cursor, right;
            m_session->edit_get_preedition (left, cursor, right);
            pos += left.length ();
            str += left + cursor + right;
        }

        tmp = utf8_mbstowcs (_("]"));
        ADD_SEPARATOR_ATTR()

        update_preedit_string (str, attr_list);
        update_preedit_caret (pos);
        show_preedit_string ();

    } else if (is_converting ()) {
        int pos = m_lookup_table.get_cursor_pos ();
        WideString cand = m_candidates[pos].m_conversion;
        update_preedit_string (cand);
        update_preedit_caret (0);
        show_preedit_string ();

    } else if (m_session) {
        WideString left, cursor, right;
        m_session->edit_get_preedition (left, cursor, right);

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

#undef ADD_SEPARATOR_ATTR

void
PrimeInstance::set_prediction (void)
{
    // prediction
    if (!is_converting () && m_session) {
        m_lookup_table.clear ();

        String query;
        m_session->edit_get_query_string (query);

        PrimeCandidates candidates;
        m_prime.set_context (m_context);
        m_prime.lookup (query, candidates);

        if (is_preediting () &&
            candidates.size () > 0 &&
            candidates[0].m_conversion.length () > 0)
        {
            for (unsigned int i = 0; i < candidates.size (); i++) {
                WideString label;
                get_candidate_label (label, candidates[i]);
                m_lookup_table.append_candidate (label);
            }
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
PrimeInstance::is_registering (void)
{
    return m_registering;
}

bool
PrimeInstance::action_commit (void)
{
    if (is_registering ()) {
        if (is_converting ()) {
            int pos = m_lookup_table.get_cursor_pos ();
            PrimeCandidate &cand = m_candidates[pos];

            m_registering_value += cand.m_conversion;

            m_prime.learn_word (cand.m_basekey, cand.m_base,
                                cand.m_part,    m_context,
                                cand.m_suffix,  cand.m_rest);
            m_context = cand.m_base + cand.m_suffix + cand.m_rest;

            m_candidates.clear();
            m_converting = false;
            if (m_session)
                m_session->edit_erase();
            m_lookup_table.clear ();
            hide_lookup_table ();

            set_preedition ();

        } else if (is_preediting ()) {
            if (m_session) {
                WideString left, cursor, right, all;
                m_session->edit_get_preedition (left, cursor, right);
                m_registering_value += left + cursor + right;
            }

            if (m_session)
                m_session->edit_erase();
            m_lookup_table.clear ();
            hide_lookup_table ();

            set_preedition ();

        } else {
            if (m_registering_key.length () > 0 &&
                m_registering_value.length () > 0)
            {
                m_prime.learn_word (m_registering_key, m_registering_value,
                                    WideString (), WideString (),
                                    WideString (), WideString ());
            }

            reset ();
        }

    } else if (is_converting ()) {
        int pos = m_lookup_table.get_cursor_pos ();
        PrimeCandidate &cand = m_candidates[pos];
        commit_string (cand.m_conversion);
        m_prime.learn_word (cand.m_basekey, cand.m_base,
                            cand.m_part,    m_context,
                            cand.m_suffix,  cand.m_rest);
        m_context = cand.m_base + cand.m_suffix + cand.m_rest;

        reset ();

    } else if (is_preediting ()) {
        if (m_session) {
            WideString left, cursor, right, all;
            m_session->edit_get_preedition (left, cursor, right);
            commit_string (left + cursor + right);
            all = left + cursor + right;
        }

        reset ();

    } else {
        return false;
    }

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

        m_candidates.clear();
        m_prime.set_context (m_context);
        m_prime.lookup (query, m_candidates, PRIME_LOOKUP_ALL);
        for (unsigned int i = 0; i < m_candidates.size (); i++) {
            WideString label;
            get_candidate_label (label, m_candidates[i]);
            m_lookup_table.append_candidate (label);
        }

        m_lookup_table.set_cursor_pos (0);
        update_lookup_table (m_lookup_table);
        show_lookup_table ();
    }

    set_preedition ();

    return true;
}

bool
PrimeInstance::action_revert (void)
{
    if (!is_preediting () && !is_registering ())
        return false;

    if (is_converting ()) {
        m_lookup_table.clear ();
        hide_lookup_table ();
        m_candidates.clear();
        m_converting = false;
        set_preedition ();

    } else if (is_registering ()) {
        if (is_preediting ()) {
            if (m_session)
                m_session->edit_erase ();
            m_lookup_table.clear ();
            hide_lookup_table ();
            set_preedition ();
        } else {
            reset ();
        }

    } else {
        reset ();
    }

    return true;
}

bool
PrimeInstance::action_modify_caret_left (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_left ();

    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_right (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_right ();
    
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_modify_caret_left_edge (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_left_edge ();
    
    set_preedition ();

    return false;
}

bool
PrimeInstance::action_modify_caret_right_edge (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_cursor_right_edge ();
    
    set_preedition ();

    return false;
}

bool
PrimeInstance::action_edit_backspace (void)
{
    if (!is_preediting ())
        return false;
    if (is_converting ())
        return false;

    if (m_session)
        m_session->edit_backspace ();

    set_preedition ();

    return true;
}

bool
PrimeInstance::action_edit_delete (void)
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
PrimeInstance::action_conv_next_candidate (void)
{
    if (!is_converting ())
        return false;

    int last_candidate = m_lookup_table.number_of_candidates () - 1;

    if (m_lookup_table.get_cursor_pos () == last_candidate) {
        if (m_factory->m_auto_register)
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
PrimeInstance::action_set_mode_default (void)
{
    if (is_converting ())
        action_revert ();

    m_session->edit_set_mode (PRIME_PREEDITION_DEFAULT);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_katakana (void)
{
    if (is_converting ())
        action_revert ();

    m_session->edit_set_mode (PRIME_PREEDITION_KATAKANA);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_half_katakana (void)
{
    if (is_converting ())
        action_revert ();

    m_session->edit_set_mode (PRIME_PREEDITION_HALF_KATAKANA);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_raw (void)
{
    if (is_converting ())
        action_revert ();

    m_session->edit_set_mode (PRIME_PREEDITION_RAW);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_set_mode_wide_ascii (void)
{
    if (is_converting ())
        action_revert ();

    m_session->edit_set_mode (PRIME_PREEDITION_WIDE_ASCII);
    set_preedition ();

    return true;
}

bool
PrimeInstance::action_register_a_word (void)
{
    if (!m_session)
        return false;

    // FIXME!
    if (!is_preediting ())
        return false;

    if (is_converting ())
        action_revert ();

    WideString left, cursor, right;
    m_session->edit_get_preedition (left, cursor, right);
    m_registering_key = left + cursor + right;

    m_registering = true;

    m_session->edit_erase();
    m_lookup_table.clear ();
    hide_lookup_table ();

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

    if (m_factory->m_show_annotation && cand.m_annotation.length () > 0) {
        label += utf8_mbstowcs (" (");
        label += cand.m_annotation;
        label += utf8_mbstowcs (" )");
    }

    if (m_factory->m_show_usage && cand.m_usage.length () > 0) {
        label += utf8_mbstowcs ("\t\xE2\x96\xBD");
        label += cand.m_usage;
    }
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
