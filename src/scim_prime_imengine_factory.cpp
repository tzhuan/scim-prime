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
#include "intl.h"

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
      m_predict_on_preedition (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION),
      m_auto_register (SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT),
      m_commit_on_upper (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT),
      m_close_cand_win_on_select (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT),
      m_show_annotation (SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT),
      m_show_usage (SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT),
      m_show_comment (SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT)
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

#define APPEND_ACTION(key, func) \
{ \
    String name = "func", str; \
    str = config->read (String (SCIM_PRIME_CONFIG_##key##_KEY), \
                        String (SCIM_PRIME_CONFIG_##key##_KEY_DEFAULT)); \
    m_actions.push_back (PrimeAction (name, str, &PrimeInstance::func)); \
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
    m_predict_on_preedition
        = config->read (String (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION),
                        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT);
    m_auto_register
        = config->read (String (SCIM_PRIME_CONFIG_AUTO_REGISTER),
                        SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT);
    m_commit_on_upper
        = config->read (String (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER),
                        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT);
    m_close_cand_win_on_select
        = config->read (String (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT),
                        SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT);
    m_show_annotation
        = config->read (String (SCIM_PRIME_CONFIG_SHOW_ANNOTATION),
                        SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT);
    m_show_usage
        = config->read (String (SCIM_PRIME_CONFIG_SHOW_USAGE),
                        SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT);
    m_show_comment
        = config->read (String (SCIM_PRIME_CONFIG_SHOW_COMMENT),
                        SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT);
    m_space_char
        = config->read (String (SCIM_PRIME_CONFIG_SPACE_CHAR),
                        String (SCIM_PRIME_CONFIG_SPACE_CHAR_DEFAULT));
    m_alt_space_char
        = config->read (String (SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_CHAR),
                        String (SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_CHAR_DEFAULT));

    m_actions.clear ();

    // edit keys
    APPEND_ACTION (COMMIT,                  action_commit_with_learn);
    APPEND_ACTION (COMMIT_WITHOUT_LEARN,    action_commit_without_learn);
    APPEND_ACTION (CONVERT,                 action_convert);
    APPEND_ACTION (CANCEL,                  action_revert);
    APPEND_ACTION (BACKSPACE,               action_edit_backspace);
    APPEND_ACTION (DELETE,                  action_edit_delete);
    APPEND_ACTION (SPACE,                   action_insert_space);
    APPEND_ACTION (ALTERNATIVE_SPACE,       action_insert_alternative_space);

    // caret keys
    APPEND_ACTION (MODIFY_CARET_LEFT,       action_modify_caret_left);
    APPEND_ACTION (MODIFY_CARET_RIGHT,      action_modify_caret_right);
    APPEND_ACTION (MODIFY_CARET_LEFT_EDGE,  action_modify_caret_left_edge);
    APPEND_ACTION (MODIFY_CARET_RIGHT_EDGE, action_modify_caret_right_edge);

    // candidate keys
    APPEND_ACTION (CONV_NEXT_CANDIDATE,     action_conv_next_candidate);
    APPEND_ACTION (CONV_PREV_CANDIDATE,     action_conv_prev_candidate);
    APPEND_ACTION (CONV_NEXT_PAGE,          action_conv_next_page);
    APPEND_ACTION (CONV_PREV_PAGE,          action_conv_prev_page);
    APPEND_ACTION (SELECT_CANDIDATE_1,      action_select_candidate_1);
    APPEND_ACTION (SELECT_CANDIDATE_2,      action_select_candidate_2);
    APPEND_ACTION (SELECT_CANDIDATE_3,      action_select_candidate_3);
    APPEND_ACTION (SELECT_CANDIDATE_4,      action_select_candidate_4);
    APPEND_ACTION (SELECT_CANDIDATE_5,      action_select_candidate_5);
    APPEND_ACTION (SELECT_CANDIDATE_6,      action_select_candidate_6);
    APPEND_ACTION (SELECT_CANDIDATE_7,      action_select_candidate_7);
    APPEND_ACTION (SELECT_CANDIDATE_8,      action_select_candidate_8);
    APPEND_ACTION (SELECT_CANDIDATE_9,      action_select_candidate_9);
    APPEND_ACTION (SELECT_CANDIDATE_10,     action_select_candidate_10);

    // segment keys
	APPEND_ACTION (SELECT_FIRST_SEGMENT,    action_select_first_segment);
	APPEND_ACTION (SELECT_LAST_SEGMENT,     action_select_last_segment);
	APPEND_ACTION (SELECT_NEXT_SEGMENT,     action_select_next_segment);
	APPEND_ACTION (SELECT_PREV_SEGMENT,     action_select_prev_segment);
	APPEND_ACTION (SHRINK_SEGMENT,          action_shrink_segment);
	APPEND_ACTION (EXPAND_SEGMENT,          action_expand_segment);

    // mode keys
    APPEND_ACTION (SET_MODE_DEFAULT,        action_set_mode_default);
    APPEND_ACTION (SET_MODE_KATAKANA,       action_set_mode_katakana);
    APPEND_ACTION (SET_MODE_HALF_KATAKANA,  action_set_mode_half_katakana);
    APPEND_ACTION (SET_MODE_RAW,            action_set_mode_raw);
    APPEND_ACTION (SET_MODE_WIDE_ASCII,     action_set_mode_wide_ascii);

    // language
    APPEND_ACTION (TOGGLE_LANGUAGE,         action_toggle_language);

    // register a word
    APPEND_ACTION (REGISTER_WORD,           action_register_a_word);
}
