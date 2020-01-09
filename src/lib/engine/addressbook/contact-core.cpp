
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
 *                         contact-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the main contact managing object
 *
 */

#include <iostream>
#include <glib/gi18n.h>

#include "config.h"

#include "contact-core.h"

/*
static void
on_search ()
{
  std::cout << "Search not implemented yet" << std::endl;
}
*/

Ekiga::ContactCore::~ContactCore ()
{
  for (std::list<boost::signals::connection>::iterator iter = conns.begin (); iter != conns.end (); ++iter)
    iter->disconnect ();
}

bool
Ekiga::ContactCore::populate_menu (MenuBuilder &builder)
{
  bool populated = false;

  //builder.add_action ("search", _("_Find"), &on_search);
  //populated = true;

  for (std::list<SourcePtr >::const_iterator iter = sources.begin ();
       iter != sources.end ();
       ++iter) {

    if (populated)
      builder.add_separator ();
    populated = (*iter)->populate_menu (builder);
  }

  return populated;
}

void
Ekiga::ContactCore::add_source (SourcePtr source)
{
  sources.push_back (source);
  source_added (source);
  conns.push_back (source->updated.connect (boost::ref (updated)));
  conns.push_back (source->book_added.connect (boost::bind (boost::ref (book_added), source, _1)));
  conns.push_back (source->book_removed.connect (boost::bind (boost::ref (book_removed), source, _1)));
  conns.push_back (source->book_updated.connect (boost::bind (boost::ref (book_updated), source, _1)));
  conns.push_back (source->contact_added.connect (boost::bind (boost::ref (contact_added), source, _1, _2)));
  conns.push_back (source->contact_removed.connect (boost::bind (boost::ref (contact_removed), source, _1, _2)));
  conns.push_back (source->contact_updated.connect (boost::bind (boost::ref (contact_updated), source, _1, _2)));
  source->questions.connect (boost::ref (questions));

  updated ();
}

void
Ekiga::ContactCore::visit_sources (boost::function1<bool, SourcePtr > visitor) const
{
  bool go_on = true;

  for (std::list<SourcePtr >::const_iterator iter = sources.begin ();
       iter != sources.end () && go_on;
       ++iter)
    go_on = visitor (*iter);
}

void
Ekiga::ContactCore::add_contact_decorator (boost::shared_ptr<ContactDecorator> decorator)
{
  contact_decorators.push_back (decorator);
}


bool
Ekiga::ContactCore::populate_contact_menu (ContactPtr contact,
					   const std::string uri,
					   MenuBuilder &builder)
{
  bool populated = false;

  for (std::list<boost::shared_ptr<ContactDecorator> >::const_iterator iter
	 = contact_decorators.begin ();
       iter != contact_decorators.end ();
       ++iter) {

    populated = (*iter)->populate_menu (contact, uri, builder) || populated;
  }

  return populated;
}
