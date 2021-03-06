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
                                        String ("ff7ea8c6-2fd5-494c-a691-e6bd0101b7d3"),
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
    : m_uuid                     (uuid),
      m_config                   (config),
      m_convert_on_period        (SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT),
      m_commit_period            (SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT),
      m_commit_on_upper          (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT),
      m_predict_on_preedition    (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT),
      m_predict_win_pos          (SCIM_PRIME_CONFIG_PREDICT_WIN_POS_DEFAULT),
      m_inline_prediction        (SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT),
      m_cand_win_page_size       (SCIM_PRIME_CONFIG_CAND_WIN_PAGE_SIZE_DEFAULT),
      m_auto_register            (SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT),
      m_close_cand_win_on_select (SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT),
      m_show_annotation          (SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT),
      m_show_usage               (SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT),
      m_show_comment             (SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT)
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
    m_reload_signal_connection.disconnect ();
}

WideString
PrimeFactory::get_name () const
{
    return utf8_mbstowcs (String ("PRIME"));
}

WideString
PrimeFactory::get_authors () const
{
    const char *package =
        PACKAGE "-" PACKAGE_VERSION "\n"
        "\n";
    const char *authors =
        _("Authors of sicm-prime:\n"
          "  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>\n"
          "  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>\n"
          "  \n"
          "Authors of PRIME:\n"
          "  Copyright (C) 2002-2005 Hiroyuki Komatsu <komatsu@taiyaki.org>\n");

    return utf8_mbstowcs (package) + utf8_mbstowcs (authors);
}

WideString
PrimeFactory::get_credits () const
{
    return utf8_mbstowcs (
        _("Taiyaki icon:\n"
          "  FUJITA Yuji <yuji@webmasters.gr.jp>\n"
          "Special thanks:\n"
          "  UTUMI Hirosi <utuhiro78@yahoo.co.jp>\n"));
}

WideString
PrimeFactory::get_help () const
{
    const char *title =
        _("Basic operation:\n"
          "  \n");

    const char *text1 =
        _("1. Switch Japanese input mode:\n"
          "  You can switch on/off Japanese input mode by pressing Zenkaku_Hankaku\n"
          "  key or Control+J.\n"
          "  \n");

    const char *text2 =
        _("2. Input hiragana and katakana:\n"
          "  You can input hiragana by inputting romaji.\n"
          "  If some predicted candidates exists, they will be shown in the lookup\n"
          "  window. You can select one of it by pressing numeric key or clicking it\n"
          "  by mouse. Or you can select a next candidate by pressing Tab key. If the\n"
          "  cursor on the lookup window beyonds the range, then scim-prime will be\n"
          "  switched to conversion mode automatically.\n"
          "  \n");

    const char *text3 =
        _("3. Convert to Japanese kanji:\n"
          "  After inputting hiragana or katakana, you can convert it to kanji by\n"
          "  pressing Space key. Then it will show some candidates. You can select a\n"
          "  next candidate by pressing Space key. If the cursor on the lookup window\n"
          "  beyonds the range, scim-prime will be switched to register mode. You\n"
          "  can register a word on this mode. If you want to escape from this mode,\n"
          "  press Escape or more Space key.\n"
          "  \n");

    const char *text4 =
        _("4. Modify sentence segments:\n"
          "  When you input a sentence, you can split it to some segments by pressing\n"
          "  left or right cursor key after starting conversion. You can also select\n"
          "  a next or previous segment by these keys. And you can shrink and extend\n"
          "  the selected segment by pressing Shift + left or right cursor key.\n"
          "  \n");

    const char *text5 =
        _("5. Commit:\n"
          "  You can commit the preedit string by pressing Enter key.\n"
          "  \n");

    const char *text6 =
        _("6. Predict English words:\n"
          "  You can switch language to English by pressing F11 key.\n"
          "  On English mode, PRIME can predict English words.\n"
          "  Basic operation is almost same with Japanese mode, but you can't use\n"
          "  Space key for starting conversion. Use down cursor key or Tab key\n"
          "  instead."
          "  \n");

    return utf8_mbstowcs (title)
        + utf8_mbstowcs (text1)
        + utf8_mbstowcs (text2)
        + utf8_mbstowcs (text3)
        + utf8_mbstowcs (text4)
        + utf8_mbstowcs (text5)
        + utf8_mbstowcs (text6);
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

#if 0
#define APPEND_ACTION(key, func)                                         \
{                                                                        \
    String name = "func", str;                                           \
    str = config->read (String (SCIM_PRIME_CONFIG_##key##_KEY),          \
                        String (SCIM_PRIME_CONFIG_##key##_KEY_DEFAULT)); \
    m_actions.push_back (PrimeAction (name, str, &PrimeInstance::func)); \
}
#else
#define APPEND_ACTION(key, func)                                         \
{                                                                        \
    String name = "func", str;                                           \
    str = config->read (String (SCIM_PRIME_CONFIG_##key##_KEY),          \
                        String (SCIM_PRIME_CONFIG_##key##_KEY_DEFAULT)); \
    m_actions.push_back (PrimeAction (name, str, func));                 \
}
#endif

#define PRIME_DEFINE_ACTION(func) \
static bool                       \
func (PrimeInstance *anthy)       \
{                                 \
    return anthy->func ();        \
}

PRIME_DEFINE_ACTION (action_insert_space);
PRIME_DEFINE_ACTION (action_insert_alternative_space);
PRIME_DEFINE_ACTION (action_commit_with_learn);
PRIME_DEFINE_ACTION (action_commit_without_learn);
PRIME_DEFINE_ACTION (action_commit_alternative);
PRIME_DEFINE_ACTION (action_convert);
PRIME_DEFINE_ACTION (action_revert);
PRIME_DEFINE_ACTION (action_start_selecting_prediction);
PRIME_DEFINE_ACTION (action_edit_backspace);
PRIME_DEFINE_ACTION (action_edit_delete);
PRIME_DEFINE_ACTION (action_modify_caret_left);
PRIME_DEFINE_ACTION (action_modify_caret_right);
PRIME_DEFINE_ACTION (action_modify_caret_left_edge);
PRIME_DEFINE_ACTION (action_modify_caret_right_edge);
PRIME_DEFINE_ACTION (action_conv_next_candidate);
PRIME_DEFINE_ACTION (action_conv_prev_candidate);
PRIME_DEFINE_ACTION (action_conv_next_page);
PRIME_DEFINE_ACTION (action_conv_prev_page);
PRIME_DEFINE_ACTION (action_select_candidate_1);
PRIME_DEFINE_ACTION (action_select_candidate_2);
PRIME_DEFINE_ACTION (action_select_candidate_3);
PRIME_DEFINE_ACTION (action_select_candidate_4);
PRIME_DEFINE_ACTION (action_select_candidate_5);
PRIME_DEFINE_ACTION (action_select_candidate_6);
PRIME_DEFINE_ACTION (action_select_candidate_7);
PRIME_DEFINE_ACTION (action_select_candidate_8);
PRIME_DEFINE_ACTION (action_select_candidate_9);
PRIME_DEFINE_ACTION (action_select_candidate_10);
PRIME_DEFINE_ACTION (action_select_first_segment);
PRIME_DEFINE_ACTION (action_select_last_segment);
PRIME_DEFINE_ACTION (action_select_next_segment);
PRIME_DEFINE_ACTION (action_select_prev_segment);
PRIME_DEFINE_ACTION (action_shrink_segment);
PRIME_DEFINE_ACTION (action_expand_segment);
PRIME_DEFINE_ACTION (action_set_mode_default);
PRIME_DEFINE_ACTION (action_set_mode_katakana);
PRIME_DEFINE_ACTION (action_set_mode_half_katakana);
PRIME_DEFINE_ACTION (action_set_mode_raw);
PRIME_DEFINE_ACTION (action_set_mode_wide_ascii);
PRIME_DEFINE_ACTION (action_toggle_on_off);
PRIME_DEFINE_ACTION (action_toggle_language);
PRIME_DEFINE_ACTION (action_register_a_word);
PRIME_DEFINE_ACTION (action_recovery);

void
PrimeFactory::reload_config (const ConfigPointer &config)
{
    if (!config) return;

    String str;

    m_command
        = config->read (String (SCIM_PRIME_CONFIG_COMMAND),
                        String (SCIM_PRIME_CONFIG_COMMAND_DEFAULT));
    m_language
        = config->read (String (SCIM_PRIME_CONFIG_LANGUAGE),
                        String (SCIM_PRIME_CONFIG_LANGUAGE_DEFAULT));
    m_typing_method
        = config->read (String (SCIM_PRIME_CONFIG_TYPING_METHOD),
                        String (SCIM_PRIME_CONFIG_TYPING_METHOD_DEFAULT));
    m_convert_on_period
        = config->read (String (SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD),
                        SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT);
    m_commit_period
        = config->read (String (SCIM_PRIME_CONFIG_COMMIT_PERIOD),
                        SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT);
    m_commit_on_upper
        = config->read (String (SCIM_PRIME_CONFIG_COMMIT_ON_UPPER),
                        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT);

    m_predict_on_preedition
        = config->read (String (SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION),
                        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT);
    m_predict_win_pos
        = config->read (String (SCIM_PRIME_CONFIG_PREDICT_WIN_POS),
                        String (SCIM_PRIME_CONFIG_PREDICT_WIN_POS_DEFAULT));
    m_direct_select_on_prediction
        = config->read (String (SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION),
                        SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION_DEFAULT);
    m_inline_prediction
        = config->read (String (SCIM_PRIME_CONFIG_INLINE_PREDICTION),
                        SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT);

    m_cand_win_page_size
        = config->read (String (SCIM_PRIME_CONFIG_CAND_WIN_PAGE_SIZE),
                        SCIM_PRIME_CONFIG_CAND_WIN_PAGE_SIZE_DEFAULT);
    m_auto_register
        = config->read (String (SCIM_PRIME_CONFIG_AUTO_REGISTER),
                        SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT);
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
    // color settings
    int red, green, blue;
    str = config->read (String (SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR),
                        String (SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR_DEFAULT));
    sscanf (str.c_str (), "#%02X%02X%02X", &red, &green, &blue);
    m_candidate_form_color = SCIM_RGB_COLOR (red, green, blue);
    str = config->read (String (SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR),
                        String (SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR_DEFAULT));
    sscanf (str.c_str (), "#%02X%02X%02X", &red, &green, &blue);
    m_candidate_usage_color = SCIM_RGB_COLOR (red, green, blue);
    str = config->read (String (SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR),
                        String (SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR_DEFAULT));
    sscanf (str.c_str (), "#%02X%02X%02X", &red, &green, &blue);
    m_candidate_comment_color = SCIM_RGB_COLOR (red, green, blue);

    m_actions.clear ();

    // edit keys
    APPEND_ACTION (SPACE,                   action_insert_space);
    APPEND_ACTION (ALTERNATIVE_SPACE,       action_insert_alternative_space);
    APPEND_ACTION (COMMIT,                  action_commit_with_learn);
    APPEND_ACTION (COMMIT_WITHOUT_LEARN,    action_commit_without_learn);
    APPEND_ACTION (COMMIT_ALTERNATIVE,      action_commit_alternative);
    APPEND_ACTION (CONVERT,                 action_convert);
    APPEND_ACTION (CANCEL,                  action_revert);
    APPEND_ACTION (SELECT_PREDICTION,       action_start_selecting_prediction);
    APPEND_ACTION (BACKSPACE,               action_edit_backspace);
    APPEND_ACTION (DELETE,                  action_edit_delete);

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
    APPEND_ACTION (TOGGLE_ON_OFF,           action_toggle_on_off);
    APPEND_ACTION (TOGGLE_LANGUAGE,         action_toggle_language);

    // register a word
    APPEND_ACTION (REGISTER_WORD,           action_register_a_word);

    // recovery
    APPEND_ACTION (RECOVERY,                action_recovery);
    m_recovery_action = &m_actions[m_actions.size () - 1];
}
