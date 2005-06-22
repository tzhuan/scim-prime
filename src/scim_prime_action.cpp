/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2005 Takuro Ashie
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

#include "scim_prime_action.h"

PrimeAction::PrimeAction (const String &name, const String &key_bindings,
                          PMF pmf)
    : m_name (name),
      m_pmf  (pmf),
      m_func (NULL)
{
    scim_string_to_key_list (m_key_bindings, key_bindings);
}

PrimeAction::PrimeAction (const String &name, const String &key_bindings,
                          Func func)
    : m_name (name),
      m_pmf  (NULL),
      m_func (func)
{
    scim_string_to_key_list (m_key_bindings, key_bindings);
}

PrimeAction::~PrimeAction (void)
{
}

bool
PrimeAction::perform (PrimeInstance *performer)
{
    if (m_pmf)
        return (performer->*m_pmf) ();
    else if (m_func)
        return m_func (performer);

    return false;
}

bool
PrimeAction::perform (PrimeInstance *performer, const KeyEvent &key)
{
    if (!m_pmf && !m_func)
        return false;

    if (match_key_event (key)) {
        if (m_pmf)
            return (performer->*m_pmf) ();
        else if (m_func)
            return m_func (performer);
    }

    return false;
}

bool
PrimeAction::match_key_event (const KeyEvent &key)
{
    KeyEventList::const_iterator kit;

    for (kit = m_key_bindings.begin (); kit != m_key_bindings.end (); ++kit) {
        uint16 mask = key.mask;
        mask &= ~SCIM_KEY_CapsLockMask;
        if (key.code == kit->code && mask == kit->mask)
             return true;
    }
    return false;
}
