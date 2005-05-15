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

#ifndef __SCIM_PRIME_PREFS_H__
#define __SCIM_PRIME_PREFS_H__

#define SCIM_PRIME_CONFIG_COMMAND                             "/IMEngine/PRIME/Command"
#define SCIM_PRIME_CONFIG_TYPING_METHOD                       "/IMEngine/PRIME/TypingMethod"
#define SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD                   "/IMEngine/PRIME/ConvertOnPeriod"
#define SCIM_PRIME_CONFIG_COMMIT_PERIOD                       "/IMEngine/PRIME/CommitPeriod"
#define SCIM_PRIME_CONFIG_COMMIT_ON_UPPER                     "/IMEngine/PRIME/CommitOnUpper"

#define SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION               "/IMEngine/PRIME/PredictOnPreedition"
#define SCIM_PRIME_CONFIG_PREDICT_WIN_POS                     "/IMEngine/PRIME/PredictWinPos"
#define SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION         "/IMEngine/PRIME/DirectSelectOnPrediction"
#define SCIM_PRIME_CONFIG_INLINE_PREDICTION                   "/IMEngine/PRIME/InlinePrediction"

#define SCIM_PRIME_CONFIG_AUTO_REGISTER                       "/IMEngine/PRIME/AutoRegister"
#define SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT            "/IMEngine/PRIME/CloseCandWinOnSelect"
#define SCIM_PRIME_CONFIG_SHOW_ANNOTATION                     "/IMEngine/PRIME/ShowAnnotation"
#define SCIM_PRIME_CONFIG_SHOW_USAGE                          "/IMEngine/PRIME/ShowUsage"
#define SCIM_PRIME_CONFIG_SHOW_COMMENT                        "/IMEngine/PRIME/ShowComment"

#define SCIM_PRIME_CONFIG_SPACE_CHAR                          "/IMEngine/PRIME/SpaceChar"
#define SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_CHAR              "/IMEngine/PRIME/AlternativeSpaceChar"

#define SCIM_PRIME_CONFIG_COMMIT_KEY                          "/IMEngine/PRIME/CommitKey"
#define SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY            "/IMEngine/PRIME/CommitWithoutLearnKey"
#define SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY              "/IMEngine/PRIME/CommitAlternativeKey"
#define SCIM_PRIME_CONFIG_CONVERT_KEY                         "/IMEngine/PRIME/ConvertKey"
#define SCIM_PRIME_CONFIG_CANCEL_KEY                          "/IMEngine/PRIME/CancelKey"
#define SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY               "/IMEngine/PRIME/SelectPredictionKey"
#define SCIM_PRIME_CONFIG_BACKSPACE_KEY                       "/IMEngine/PRIME/BackSpaceKey"
#define SCIM_PRIME_CONFIG_DELETE_KEY                          "/IMEngine/PRIME/DeleteKey"
#define SCIM_PRIME_CONFIG_SPACE_KEY                           "/IMEngine/PRIME/SpaceKey"
#define SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY               "/IMEngine/PRIME/AlternativeSpaceKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY               "/IMEngine/PRIME/MoveCaretLeftKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY              "/IMEngine/PRIME/MoveCaretRightKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY          "/IMEngine/PRIME/MoveCaretLeftEdgeKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY         "/IMEngine/PRIME/MoveCaretRightEdgeKey"
#define SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY             "/IMEngine/PRIME/ConvNextCandidateKey"
#define SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY             "/IMEngine/PRIME/ConvPrevCandidateKey"
#define SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY                  "/IMEngine/PRIME/ConvNextPageKey"
#define SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY                  "/IMEngine/PRIME/ConvPrevPageKey"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY              "/IMEngine/PRIME/SelectCandidates1Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY              "/IMEngine/PRIME/SelectCandidates2Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY              "/IMEngine/PRIME/SelectCandidates3Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY              "/IMEngine/PRIME/SelectCandidates4Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY              "/IMEngine/PRIME/SelectCandidates5Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY              "/IMEngine/PRIME/SelectCandidates6Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY              "/IMEngine/PRIME/SelectCandidates7Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY              "/IMEngine/PRIME/SelectCandidates8Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY              "/IMEngine/PRIME/SelectCandidates9Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY             "/IMEngine/PRIME/SelectCandidates10Key"
#define SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY            "/IMEngine/PRIME/SelectFirstSegmentKey"
#define SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY             "/IMEngine/PRIME/SelectLastSegmentKey"
#define SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY             "/IMEngine/PRIME/SelectNextSegmentKey"
#define SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY             "/IMEngine/PRIME/SelectPrevSegmentKey"
#define SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY                  "/IMEngine/PRIME/ShrinkSegmentKey"
#define SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY                  "/IMEngine/PRIME/ExpandSegmentKey"
#define SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY                "/IMEngine/PRIME/SetModeDefaultKey"
#define SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY               "/IMEngine/PRIME/SetModeKatakanaKey"
#define SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY          "/IMEngine/PRIME/SetModeHalfKatakanaKey"
#define SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY                    "/IMEngine/PRIME/SetModeRawKey"
#define SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY             "/IMEngine/PRIME/SetModeWideAsciiKey"
#define SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY                 "/IMEngine/PRIME/ToggleLanguageKey"
#define SCIM_PRIME_CONFIG_REGISTER_WORD_KEY                   "/IMEngine/PRIME/RegisterWordKey"

#define SCIM_PRIME_CONFIG_COMMAND_DEFAULT                     "prime"
#define SCIM_PRIME_CONFIG_TYPING_METHOD_DEFAULT               ""
#define SCIM_PRIME_CONFIG_CONVERT_ON_PERIOD_DEFAULT           false
#define SCIM_PRIME_CONFIG_COMMIT_PERIOD_DEFAULT               true
#define SCIM_PRIME_CONFIG_COMMIT_ON_UPPER_DEFAULT             false

#define SCIM_PRIME_CONFIG_PREDICT_ON_PREEDITION_DEFAULT       true
#define SCIM_PRIME_CONFIG_PREDICT_WIN_POS_DEFAULT             "head"
#define SCIM_PRIME_CONFIG_DIRECT_SELECT_ON_PREDICTION_DEFAULT true
#define SCIM_PRIME_CONFIG_INLINE_PREDICTION_DEFAULT           false

#define SCIM_PRIME_CONFIG_AUTO_REGISTER_DEFAULT               true
#define SCIM_PRIME_CONFIG_CLOSE_CAND_WIN_ON_SELECT_DEFAULT    true
#define SCIM_PRIME_CONFIG_SHOW_ANNOTATION_DEFAULT             true
#define SCIM_PRIME_CONFIG_SHOW_USAGE_DEFAULT                  true
#define SCIM_PRIME_CONFIG_SHOW_COMMENT_DEFAULT                true
#define SCIM_PRIME_CONFIG_SPACE_CHAR_DEFAULT                  " "
#define SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_CHAR_DEFAULT      "\xE3\x80\x80"

#define SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT                  "Return,KP_Enter,Control+m,Control+M,Henkan"
#define SCIM_PRIME_CONFIG_COMMIT_WITHOUT_LEARN_KEY_DEFAULT    "Muhenkan"
#define SCIM_PRIME_CONFIG_COMMIT_ALTERNATIVE_KEY_DEFAULT      "Shift+Return"
#define SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT                 "space,Control+l,Control+L,Down"
#define SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT                  "Escape,Control+g,Control+G"
#define SCIM_PRIME_CONFIG_SELECT_PREDICTION_KEY_DEFAULT       "Tab"
#define SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT               "BackSpace,Control+h,Control+H"
#define SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT                  "Delete,Control+d,Control+D"
#define SCIM_PRIME_CONFIG_SPACE_KEY_DEFAULT                   "space"
#define SCIM_PRIME_CONFIG_ALTERNATIVE_SPACE_KEY_DEFAULT       "Shift+space,Alt+space"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT  "Control+a,Control+A,Home"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT "Control+e,Control+E,End"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT       "Left,Control+b,Control+B"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT      "Right,Control+f,Control+F"
#define SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT     "space,Tab,Down,KP_Add,Control+n,Control+N"
#define SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT     "Shift+Tab,Up,KP_Subtract,Control+p,Control+P"
#define SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY_DEFAULT          "Page_Up"
#define SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY_DEFAULT          "Page_Down"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY_DEFAULT      "1"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY_DEFAULT      "2"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY_DEFAULT      "3"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY_DEFAULT      "4"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY_DEFAULT      "5"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY_DEFAULT      "6"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY_DEFAULT      "7"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY_DEFAULT      "8"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY_DEFAULT      "9"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY_DEFAULT     "10"
#define SCIM_PRIME_CONFIG_SELECT_FIRST_SEGMENT_KEY_DEFAULT    "Control+a,Control+A,Home"
#define SCIM_PRIME_CONFIG_SELECT_LAST_SEGMENT_KEY_DEFAULT     "Control+e,Control+E,End"
#define SCIM_PRIME_CONFIG_SELECT_NEXT_SEGMENT_KEY_DEFAULT     "Right,Control+f,Control+F"
#define SCIM_PRIME_CONFIG_SELECT_PREV_SEGMENT_KEY_DEFAULT     "Left,Control+b,Control+B"
#define SCIM_PRIME_CONFIG_SHRINK_SEGMENT_KEY_DEFAULT          "Shift+Left,Control+i,Control+I"
#define SCIM_PRIME_CONFIG_EXPAND_SEGMENT_KEY_DEFAULT          "Shift+Right,Control+o,Control+O"
#define SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT        "F6"
#define SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT       "F7"
#define SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT  "F8"
#define SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT     "F9"
#define SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT            "F10"
#define SCIM_PRIME_CONFIG_TOGGLE_LANGUAGE_KEY_DEFAULT         "F11"
#define SCIM_PRIME_CONFIG_REGISTER_WORD_KEY_DEFAULT           "Control+w, Control+W"

#endif /* __SCIM_PRIME_PREFS_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
