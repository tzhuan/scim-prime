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

#define SCIM_PRIME_CONFIG_COMMIT_KEY                          "/IMEngine/PRIME/CommitKey"
#define SCIM_PRIME_CONFIG_CONVERT_KEY                         "/IMEngine/PRIME/ConvertKey"
#define SCIM_PRIME_CONFIG_CANCEL_KEY                          "/IMEngine/PRIME/CancelKey"
#define SCIM_PRIME_CONFIG_BACKSPACE_KEY                       "/IMEngine/PRIME/BackSpaceKey"
#define SCIM_PRIME_CONFIG_DELETE_KEY                          "/IMEngine/PRIME/DeleteKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY               "/IMEngine/PRIME/MoveCaretLeftKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY              "/IMEngine/PRIME/MoveCaretRightKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY          "/IMEngine/PRIME/MoveCaretLeftEdgeKey"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY         "/IMEngine/PRIME/MoveCaretRightEdgeKey"
#define SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY             "/IMEngine/PRIME/ConvNextCandidateKey"
#define SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY             "/IMEngine/PRIME/ConvPrevCandidateKey"
#define SCIM_PRIME_CONFIG_CONV_NEXT_PAGE_KEY                  "/IMEngine/PRIME/ConvNextPageKey"
#define SCIM_PRIME_CONFIG_CONV_PREV_PAGE_KEY                  "/IMEngine/PRIME/ConvPrevPageKey"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_1_KEY              "/IMEngine/Prime/SelectCandidates1Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_2_KEY              "/IMEngine/Prime/SelectCandidates2Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_3_KEY              "/IMEngine/Prime/SelectCandidates3Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_4_KEY              "/IMEngine/Prime/SelectCandidates4Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_5_KEY              "/IMEngine/Prime/SelectCandidates5Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_6_KEY              "/IMEngine/Prime/SelectCandidates6Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_7_KEY              "/IMEngine/Prime/SelectCandidates7Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_8_KEY              "/IMEngine/Prime/SelectCandidates8Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_9_KEY              "/IMEngine/Prime/SelectCandidates9Key"
#define SCIM_PRIME_CONFIG_SELECT_CANDIDATE_10_KEY             "/IMEngine/Prime/SelectCandidates10Key"
#define SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY                "/IMEngine/PRIME/SetModeDefaultKey"
#define SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY               "/IMEngine/PRIME/SetModeKatakanaKey"
#define SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY          "/IMEngine/PRIME/SetModeHalfKatakanaKey"
#define SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY                    "/IMEngine/PRIME/SetModeRawKey"
#define SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY             "/IMEngine/PRIME/SetModeWideAsciiKey"
#define SCIM_PRIME_CONFIG_LEARN_WORD_KEY                      "/IMEngine/PRIME/LeanWordKey"

#define SCIM_PRIME_CONFIG_COMMIT_KEY_DEFAULT                  "Return,KP_Enter,Control+m,Control+M"
#define SCIM_PRIME_CONFIG_CONVERT_KEY_DEFAULT                 "space,Control+l,Control+L"
#define SCIM_PRIME_CONFIG_CANCEL_KEY_DEFAULT                  "Escape,Control+g,Control+G"
#define SCIM_PRIME_CONFIG_BACKSPACE_KEY_DEFAULT               "BackSpace,Control+h,Control+H"
#define SCIM_PRIME_CONFIG_DELETE_KEY_DEFAULT                  "Delete,Control+d,Control+D"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_EDGE_KEY_DEFAULT  "Control+a,Control+A,Home"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_EDGE_KEY_DEFAULT "Control+e,Control+E,End"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_LEFT_KEY_DEFAULT       "Left,Control+b,Control+B"
#define SCIM_PRIME_CONFIG_MODIFY_CARET_RIGHT_KEY_DEFAULT      "Right,Control+f,Control+F"
#define SCIM_PRIME_CONFIG_CONV_NEXT_CANDIDATE_KEY_DEFAULT     "space,Down,KP_Add,Control+n,Control+N"
#define SCIM_PRIME_CONFIG_CONV_PREV_CANDIDATE_KEY_DEFAULT     "Up,KP_Subtract,Control+p,Control+P,BackSpace"
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
#define SCIM_PRIME_CONFIG_SET_MODE_DEFAULT_KEY_DEFAULT        "F6"
#define SCIM_PRIME_CONFIG_SET_MODE_KATAKANA_KEY_DEFAULT       "F7"
#define SCIM_PRIME_CONFIG_SET_MODE_HALF_KATAKANA_KEY_DEFAULT  "F8"
#define SCIM_PRIME_CONFIG_SET_MODE_WIDE_ASCII_KEY_DEFAULT     "F9"
#define SCIM_PRIME_CONFIG_SET_MODE_RAW_KEY_DEFAULT            "F10"
#define SCIM_PRIME_CONFIG_LEARN_WORD_KEY_DEFAULT              "Control+w, Control+W"

#endif /* __SCIM_PRIME_PREFS_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
