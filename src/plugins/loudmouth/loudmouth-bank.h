
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2008 Damien Sandras

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
 *                         loudmouth-bank.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of the loudmouth account manager
 *
 */

#ifndef __LOUDMOUTH_BANK_H__
#define __LOUDMOUTH_BANK_H__

#include "services.h"

#include "bank-impl.h"

#include "loudmouth-account.h"
#include "loudmouth-dialect.h"

namespace LM
{

  class Bank:
    public Ekiga::Service,
    public Ekiga::BankImpl<Account>
  {
  public:
    Bank (boost::shared_ptr<Ekiga::PersonalDetails> details_,
	  boost::shared_ptr<Dialect> dialect_,
	  boost::shared_ptr<Cluster> cluster_);

    ~Bank ();

    /* Service */

    const std::string get_name () const
    { return "loudmouth-bank"; }

    const std::string get_description () const
    { return "\tManager of the XMPP/Jabber accounts"; }

    /* BankImpl */

    bool populate_menu (Ekiga::MenuBuilder& builder);

  private:

    boost::shared_ptr<Ekiga::PersonalDetails> details;
    boost::shared_ptr<Cluster> cluster;
    boost::shared_ptr<Dialect> dialect;

    xmlDocPtr doc;

    void add (boost::shared_ptr<Account> account);

    void save () const;

    void new_account ();

    void on_new_account_form_submitted (bool submitted,
					Ekiga::Form& result);
  };

  typedef boost::shared_ptr<Bank> BankPtr;
};

#endif
