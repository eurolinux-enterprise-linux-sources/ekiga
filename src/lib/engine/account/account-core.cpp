
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         account-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : implementation of the main account managing object
 *
 */

#include <iostream>

#include "config.h"

#include "account-core.h"
#include "bank.h"

Ekiga::AccountCore::AccountCore ()
{
}


Ekiga::AccountCore::~AccountCore ()
{
}


bool
Ekiga::AccountCore::populate_menu (MenuBuilder & builder)
{
  bool populated = false;

  for (bank_const_iterator iter = banks.begin ();
       iter != banks.end ();
       iter++) {

    populated = (*iter)->populate_menu (builder);
  }

  return populated;
}


void
Ekiga::AccountCore::add_bank (BankPtr bank)
{
  banks.push_back (bank);

  bank->account_added.connect (boost::bind (boost::ref (account_added), bank, _1));
  bank->account_removed.connect (boost::bind (boost::ref (account_removed), bank, _1));
  bank->account_updated.connect (boost::bind (boost::ref (account_updated), bank, _1));

  bank_added (bank);

  bank->questions.connect (boost::ref (questions));
}


void
Ekiga::AccountCore::visit_banks (boost::function1<bool, BankPtr> visitor) const
{
  bool go_on = true;

  for (bank_const_iterator iter = banks.begin ();
       iter != banks.end () && go_on;
       iter++)
    go_on = visitor (*iter);
}
