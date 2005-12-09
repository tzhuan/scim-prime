/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 Takuro Ashie
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

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include "scim_prime_prefs.h"
#include "intl.h"

namespace scim_prime {
BoolConfigData __config_bool_common [] =
{
    {
        SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD,
        SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT,
        N_("_Start conversion on inputting comma or period"),
        NULL,
        N_("Start conversion on inputting comma or period."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_PERIOD,
        SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT,
        N_("Commit comma and period _immediately"),
        NULL,
        N_("Commit comma and period immediatly on inputting these characters "
           "when no preedition string exists."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER,
        SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT,
        N_("Commit when input an _upper letter"),
        NULL,
        N_("Commit previous preedit string when a upper letter is entered."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION,
        SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT,
        N_("_Predict while typing letters"),
        NULL,
        N_("Show candidates window to display predicted candidates "
           "while typing letters."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION,
        SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION_DEFAULT,
        N_("Use _direct select keys also on prediction"),
        NULL,
        N_("Use direct select keys not only for conversion state "
           "but also for prediction state."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_INLINE_PREDICTION,
        SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT,
        N_("_Inline prediction"),
        NULL,
        N_("Show first candidate of predictions instead of reading on preedit area. "
           "Reading is shown by external window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_AUTO_REGISTER,
        SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT,
        N_("Switch to the inline word _register mode automatically"),
        NULL,
        N_("Invoke the inline word register mode when the cursor in the "
           "candidates window is move to out of range."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT,
        SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT,
        N_("_Close candidates window when a candidate is selected directly"),
        NULL,
        N_("Close candidates window when a candidate is selected directly."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_ANNOTATION,
        SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT,
        N_("Show _annotation of the word on the candidates window"),
        NULL,
        N_("Show annotation of the word on the candidates window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_USAGE,
        SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT,
        N_("Show _usage of the word on the candidates window"),
        NULL,
        N_("Show usage of the word on the candidates window."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHOW_COMMENT,
        SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT,
        N_("Show co_mment of the word on the candidates window"),
        NULL,
        N_("Show comment of the word on the candidates window."),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_string_common [] =
{
    {
        SCIM_PRIME_CONFIG_COMMAND,
        SCIM_PRIME_CONFIG_COMMAND_DEFAULT,
        N_("PRIME _command:"),
        NULL,
        N_("The PRIME command to use as conversion engine."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_LANGUAGE,
        SCIM_PRIME_CONFIG_LANGUAGE_DEFAULT,
        N_("Default _language:"),
        NULL,
        N_("Default language:"),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_PREDICT_WIN_POS,
        SCIM_PRIME_CONFIG_PREDICT_WIN_POS_DEFAULT,
        N_("Prediction _window position:"),
        NULL,
        N_("The prediction window position to show."),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_edit [] =
{
    {
        SCIM_PRIME_CONFIG_SPACE_KEY,
        SCIM_PRIME_CONFIG_SPACE_KEY_DEFAULT,
        N_("Insert space"),
        N_("Select space keys"),
        N_("The key events to insert space letter on non-preediting state. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY,
        SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY_DEFAULT,
        N_("Insert alternative space"),
        N_("Select alternative space keys"),
        N_("The key events to insert alterenative space letter on non-preediting state. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_BACKSPACE_KEY,
        SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT,
        N_("Backspace"),
        N_("Select backspace keys"),
        N_("The key events to delete a character before caret. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_DELETE_KEY,
        SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT,
        N_("Delete"),
        N_("Select delete keys"),
        N_("The key events to delete a character after caret. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_convert [] =
{
    {
        SCIM_PRIME_CONFIG_CONVERT_KEY,
        SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT,
        N_("Convert"),
        N_("Select convert keys"),
        N_("The key events to convert the preedit string to kanji. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANCEL_KEY,
        SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT,
        N_("Cancel"),
        N_("Select cancel keys"),
        N_("The key events to cancel preediting or converting. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_KEY,
        SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT,
        N_("Commit"),
        N_("Select commit keys"),
        N_("The key events to commit the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY,
        SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY_DEFAULT,
        N_("Commit witout learn"),
        N_("Select commit witout learn keys"),
        N_("The key events to commit the preedit string without learn. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY,
        SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY_DEFAULT,
        N_("Commit alternative"),
        N_("Select commit alternative keys"),
        N_("The key events to commit the first predicted string. "
           "On inline prediction mode, reading string will be commited by this key events."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY_DEFAULT,
        N_("Complete"),
        N_("Select complete keys"),
        N_("The key events to complete a word using predictions. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY,
        SCIM_PRIME_CONFIG_REGISTER_WORD_KEY_DEFAULT,
        N_("Register a word"),
        N_("Select register a word keys"),
        N_("The key events to invoke registering a word mode. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_mode [] =
{
    {
        SCIM_PRIME_CONFIG_TOGGLE_ON_OFF_KEY,
        SCIM_PRIME_CONFIG_TOGGLE_ON_OFF_KEY_DEFAULT,
        N_("On/Off"),
        N_("Select on/off keys"),
        N_("The key events to toggle on/off PRIME."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT,
        N_("Default mode"),
        N_("Select default mode keys"),
        N_("The key events to set to default mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT,
        N_("Katakana mode"),
        N_("Select katakana mode keys"),
        N_("The key events to set to katakana mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT,
        N_("Half katakana mode"),
        N_("Select half katakana mode keys"),
        N_("The key events to set to half katakana mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT,
        N_("Wide ascii mode"),
        N_("Select wide ascii mode keys"),
        N_("The key events to set to wide ascii mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY,
        SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT,
        N_("Raw mode"),
        N_("Select raw mode keys"),
        N_("The key events to set to raw mode. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY,
        SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY_DEFAULT,
        N_("Toggle language"),
        N_("Select toggle language keys"),
        N_("The key events to toggle language. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_caret [] =
{
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT,
        N_("Move to first"),
        N_("Select move caret to first keys"),
        N_("The key events to move the caret to the first of preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT,
        N_("Move to last"),
        N_("Select move caret to last keys"),
        N_("The key events to move the caret to the last of the preedit string. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT,
        N_("Move to forward"),
        N_("Select move caret to forward keys"),
        N_("The key events to move the caret to forward. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY,
        SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT,
        N_("Move to backward"),
        N_("Select move caret to backward keys"),
        N_("The key events to move the caret to backward. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_segments [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT,
        N_("First segment"),
        N_("Select first segment keys"),
        N_("The key events to select first segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT,
        N_("Last segment"),
        N_("Select last segment keys"),
        N_("The key events to select last segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT,
        N_("Next segment"),
        N_("Select next segment keys"),
        N_("The key events to select next segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT,
        N_("Previous segment"),
        N_("Select previous segment keys"),
        N_("The key events to select previous segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT,
        N_("Shrink segment"),
        N_("Select shrink segment keys"),
        N_("The key events to shrink the selected segment. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY,
        SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT,
        N_("Expand segment"),
        N_("Select expand segment keys"),
        N_("The key events to expand the selected segment. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_candidates [] =
{
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT,
        N_("Next candidate"),
        N_("Select next candidate keys"),
        N_("The key events to select next candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT,
        N_("Previous candidate"),
        N_("Select previous candidate keys"),
        N_("The key events to select previous candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY_DEFAULT,
        N_("Page up"),
        N_("Select page up candidates keys"),
        N_("The key events to select page up candidates. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY,
        SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY_DEFAULT,
        N_("Page down"),
        N_("Select page down candidates keys"),
        N_("The key events to select page down candidates. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

StringConfigData __config_keyboards_direct_select_candidate [] =
{
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT,
        N_("1st candidate"),
        N_("Select keys to select 1st candidate"),
        N_("The key events to select 1st candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT,
        N_("2nd candidate"),
        N_("Select keys to select 2nd candidate"),
        N_("The key events to select 2nd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT,
        N_("3rd candidate"),
        N_("Select keys to select 3rd candidate"),
        N_("The key events to select 3rd candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT,
        N_("4th candidate"),
        N_("Select keys to select 4th candidate"),
        N_("The key events to select 4th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT,
        N_("5th candidate"),
        N_("Select keys to select 5th candidate"),
        N_("The key events to select 5th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT,
        N_("6th candidate"),
        N_("Select keys to select 6th candidate"),
        N_("The key events to select 6th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT,
        N_("7th candidate"),
        N_("Select keys to select 7th candidate"),
        N_("The key events to select 7th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT,
        N_("8th candidate"),
        N_("Select keys to select 8th candidate"),
        N_("The key events to select 8th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT,
        N_("9th candidate"),
        N_("Select keys to select 9th candidate"),
        N_("The key events to select 9th candidate. "),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY,
        SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT,
        N_("10th candidate"),
        N_("Select keys to select 10th candidate"),
        N_("The key events to select 10th candidate. "),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};

ColorConfigData __config_color_common [] =
{
    {
        SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_FORM_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the annotaion text"),
        N_("The color of the annotaion text in the candidate list."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_USAGE_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the usage text"),
        N_("The color of the usage text in the candidate list."),
        NULL,
        false,
    },
    {
        SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR,
        SCIM_PRIME_CONFIG_CANDIDATE_COMMENT_COLOR_DEFAULT,
        N_("Co_lor:"),
        N_("The color of the comment text"),
        N_("The color of the comment text in the candidate list."),
        NULL,
        false,
    },
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
        false,
    },
};
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
