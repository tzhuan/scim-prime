/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#ifndef __SCIM_PRIME_IMENGINE_H__
#define __SCIM_PRIME_IMENGINE_H__

#define Uses_SCIM_ICONV
#include <scim.h>
#include <prime_connection.h>
using namespace scim;

class PrimeFactory : public IMEngineFactoryBase
{
    String m_uuid;

    friend class PrimeInstance;

    /* config */
    ConfigPointer  m_config;
    Connection     m_reload_signal_connection;

    /* for preference */
    String       m_command;
    String       m_typing_method;
    bool         m_auto_register;
    bool         m_commit_on_upper;
    bool         m_show_annotation;
    bool         m_show_usage;

    /* for key bindings */
    KeyEventList m_commit_keys;
    KeyEventList m_convert_keys;
    KeyEventList m_cancel_keys;

    KeyEventList m_backspace_keys;
    KeyEventList m_delete_keys;

    KeyEventList m_modify_caret_left_keys;
    KeyEventList m_modify_caret_right_keys;
    KeyEventList m_modify_caret_left_edge_keys;
    KeyEventList m_modify_caret_right_edge_keys;

    KeyEventList m_conv_next_candidate_keys;
    KeyEventList m_conv_prev_candidate_keys;
    KeyEventList m_conv_next_page_keys;
    KeyEventList m_conv_prev_page_keys;
    KeyEventList m_select_candidate_keys[10];

    KeyEventList m_set_mode_default_keys;
    KeyEventList m_set_mode_katakana_keys;
    KeyEventList m_set_mode_half_katakana_keys;
    KeyEventList m_set_mode_raw_keys;
    KeyEventList m_set_mode_wide_ascii_keys;

    KeyEventList m_register_word_keys;

public:
    PrimeFactory (const String &lang,
                  const String &uuid,
                  const ConfigPointer &config);
    virtual ~PrimeFactory ();

    virtual WideString  get_name () const;
    virtual WideString  get_authors () const;
    virtual WideString  get_credits () const;
    virtual WideString  get_help () const;
    virtual String      get_uuid () const;
    virtual String      get_icon_file () const;

    virtual IMEngineInstancePointer create_instance (const String& encoding,
                                                     int id = -1);

private:
    void reload_config (const ConfigPointer &config);
};

class PrimeInstance : public IMEngineInstanceBase
{
private:
    static PrimeConnection  m_prime;

    PrimeSession           *m_session;

    PrimeFactory           *m_factory;

    KeyEvent                m_prev_key;

    /* for candidates window */
    CommonLookupTable       m_lookup_table;

    /* for toolbar */
    PropertyList            m_properties;

    /* values */
    PrimeCandidates         m_candidates;
    WideString              m_context;

    /* flags */
    bool                    m_converting;
    bool                    m_registering;

    /* for register mode */
    WideString              m_registering_key;
    WideString              m_registering_value;
    unsigned int            m_registering_cursor;

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
    virtual bool is_converting                 (void);
    virtual bool is_registering                (void);

private:
    void   set_preedition                      (void);
    void   set_prediction                      (void);

    void   select_candidate_no_direct          (unsigned int    item);

    /* processing key event */
    bool   process_key_event_lookup_keybind    (const KeyEvent &key);
    bool   process_key_event_without_preedit   (const KeyEvent &key);
    bool   process_key_event_with_preedit      (const KeyEvent &key);
    bool   process_key_event_with_candidate    (const KeyEvent &key);
    bool   process_remaining_key_event         (const KeyEvent &key);

    /* actions */
    bool   action_commit                       (void);
    bool   action_convert                      (void);
    bool   action_revert                       (void);

    bool   action_modify_caret_left            (void);
    bool   action_modify_caret_right           (void);
    bool   action_modify_caret_left_edge       (void);
    bool   action_modify_caret_right_edge      (void);

    bool   action_edit_backspace               (void);
    bool   action_edit_delete                  (void);

    bool   action_conv_next_candidate          (void);
    bool   action_conv_prev_candidate          (void);
    bool   action_conv_next_page               (void);
    bool   action_conv_prev_page               (void);

    bool   action_select_candidate             (unsigned int i);
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

    bool   action_set_mode_default             (void);
    bool   action_set_mode_katakana            (void);
    bool   action_set_mode_half_katakana       (void);
    bool   action_set_mode_raw                 (void);
    bool   action_set_mode_wide_ascii          (void);

    bool   action_register_a_word              (void);

    /* utility */
    bool   action_commit_on_register           (void);
    bool   match_key_event                     (const KeyEventList &keys,
                                                const KeyEvent     &key) const;
    void   get_candidate_label                 (WideString         &label,
                                                PrimeCandidate     &cand);
};
#endif /* __SCIM_PRIME_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
