/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 - 2005 Takuro Ashie
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

#ifndef __SCIM_PRIME_IMENGINE_H__
#define __SCIM_PRIME_IMENGINE_H__

#include <scim.h>

#include "prime_connection.h"
#include "prime_session.h"

using namespace scim;

typedef enum {
    SCIM_PRIME_LANGUAGE_OFF,
    SCIM_PRIME_LANGUAGE_JAPANESE,
    // Sessions after this will ignore conversion keys which conflicts with
    // space and alternative space keys.
    SCIM_PRIME_LANGUAGE_ENGLISH,
} SCIMPrimeLanguage;

class PrimeInstance : public IMEngineInstanceBase
{
    friend class PrimeFactory;

public:
    PrimeInstance (PrimeFactory   *factory,
                   const String   &encoding,
                   int             id = -1);
    virtual ~PrimeInstance ();

    virtual bool process_key_event             (const KeyEvent& key);
    virtual void move_preedit_caret            (unsigned int pos);
    virtual void select_candidate              (unsigned int item);
    virtual void update_lookup_table_page_size (unsigned int page_size);
    virtual void lookup_table_page_up          (void);
    virtual void lookup_table_page_down        (void);
    virtual void reset                         (void);
    virtual void focus_in                      (void);
    virtual void focus_out                     (void);
    virtual void trigger_property              (const String &property);

public:
    virtual bool is_preediting                 (void);
    virtual bool is_selecting_prediction       (void);
    virtual bool is_converting                 (void);
    virtual bool is_modifying                  (void);
    virtual bool is_registering                (void);

public: // actions
    bool   action_commit_with_learn            (void);
    bool   action_commit_without_learn         (void);
    bool   action_commit_alternative           (void);
    bool   action_convert                      (void);
    bool   action_revert                       (void);
    bool   action_start_selecting_prediction   (void);
    bool   action_finish_selecting_candidates  (void);

    bool   action_modify_caret_left            (void);
    bool   action_modify_caret_right           (void);
    bool   action_modify_caret_left_edge       (void);
    bool   action_modify_caret_right_edge      (void);

    bool   action_edit_backspace               (void);
    bool   action_edit_delete                  (void);

    bool   action_insert_space                 (void);
    bool   action_insert_alternative_space     (void);

    bool   action_conv_next_candidate          (void);
    bool   action_conv_prev_candidate          (void);
    bool   action_conv_next_page               (void);
    bool   action_conv_prev_page               (void);

    bool   action_select_candidate_1           (void);
    bool   action_select_candidate_2           (void);
    bool   action_select_candidate_3           (void);
    bool   action_select_candidate_4           (void);
    bool   action_select_candidate_5           (void);
    bool   action_select_candidate_6           (void);
    bool   action_select_candidate_7           (void);
    bool   action_select_candidate_8           (void);
    bool   action_select_candidate_9           (void);
    bool   action_select_candidate_10          (void);

    bool   action_modify_start                 (void);
    bool   action_select_prev_segment          (void);
    bool   action_select_next_segment          (void);
    bool   action_select_first_segment         (void);
    bool   action_select_last_segment          (void);
    bool   action_shrink_segment               (void);
    bool   action_expand_segment               (void);

    bool   action_set_mode_default             (void);
    bool   action_set_mode_katakana            (void);
    bool   action_set_mode_half_katakana       (void);
    bool   action_set_mode_raw                 (void);
    bool   action_set_mode_wide_ascii          (void);

    bool   action_toggle_on_off                (void);
    bool   action_set_on                       (void);
    bool   action_set_off                      (void);
    bool   action_toggle_language              (void);
    bool   action_set_language_raw             (void);
    bool   action_set_language_japanese        (void);
    bool   action_set_language_english         (void);

    bool   action_register_a_word              (void);

    bool   action_recovery                     (void);

private:
    PrimeSession *
           get_session                         (void);

    void   set_preedition                      (void);
    void   set_preedition_on_preediting        (void);
    void   set_preedition_on_register          (void);
    void   set_prediction                      (void);

    void   select_candidate_no_direct          (unsigned int    item);
    void   install_properties                  (void);

    /* processing key event */
    bool   process_key_event_lookup_keybind    (const KeyEvent &key);
    bool   process_input_key_event             (const KeyEvent &key);

    /* utility */
    bool   action_commit                       (bool learn);
    bool   action_commit_on_register           (bool learn);
    bool   action_select_candidate             (unsigned int i);
    bool   match_key_event                     (const KeyEventList &keys,
                                                const KeyEvent     &key) const;
    void   get_candidate_label                 (WideString         &label,
                                                AttributeList      &attrs,
                                                PrimeCandidate     &cand);
    void   set_error_message                   (void);

private:
    static unsigned int     m_recovery_count;

    PrimeSession           *m_session;

    PrimeFactory           *m_factory;

    KeyEvent                m_prev_key;

    /* for candidates window */
    CommonLookupTable       m_lookup_table;

    /* for toolbar */
    PropertyList            m_properties;

    /* values */
    PrimeCandidates         m_candidates;

    /* flags */
    SCIMPrimeLanguage       m_language;
    bool                    m_disable;
    bool                    m_converting;
    bool                    m_modifying;
    bool                    m_registering;
    bool                    m_cancel_prediction;
    bool                    m_preedition_visible;
    bool                    m_lookup_table_visible;

    /* for register mode */
    String                  m_query_string;
    WideString              m_registering_key;
    WideString              m_registering_value;
    unsigned int            m_registering_cursor;
};
#endif /* __SCIM_PRIME_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
