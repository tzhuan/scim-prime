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

    /* flags */
    bool                    m_converting;

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
    virtual bool is_learning                   (void);

private:
    void   set_preedition                      (void);
    void   set_prediction                      (void);

    void   select_candidate_no_direct          (unsigned int item);

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

    bool   action_move_caret_backward          (void);
    bool   action_move_caret_forward           (void);
    bool   action_move_caret_first             (void);
    bool   action_move_caret_last              (void);

    bool   action_back                         (void);
    bool   action_delete                       (void);
    bool   action_select_next_candidate        (void);
    bool   action_select_prev_candidate        (void);

    /* utility */
    bool   match_key_event (const KeyEventList &keys,
                            const KeyEvent &key) const;
};
#endif /* __SCIM_PRIME_IMENGINE_H__ */
/*
vi:ts=4:nowrap:ai:expandtab
*/
