
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
 *                         services.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a service object
 *
 */


/* set DEBUG to 1 and the service core will tell whenever something is added or
 * asked to it...
 */
#define DEBUG 0

#if DEBUG
#include <iostream>
#endif

#include "services.h"

boost::optional<bool>
Ekiga::Service::get_bool_property (const std::string /*name*/) const
{
  boost::optional<bool> result;

  return result;
}

boost::optional<int>
Ekiga::Service::get_int_property (const std::string /*name*/) const
{
  boost::optional<int> result;

  return result;
}

boost::optional<std::string>
Ekiga::Service::get_string_property (const std::string /*name*/) const
{
  boost::optional<std::string> result;

  return result;
}

Ekiga::ServiceCore::~ServiceCore ()
{
#if DEBUG
  std::list<boost::weak_ptr<Service> > remaining_services;
  for (std::list<boost::shared_ptr<Service> >::iterator iter = services.begin();
       iter != services.end ();
       ++iter)
    remaining_services.push_front (*iter);
#endif
  /* this frees the memory, if we're the only to hold references,
   * and frees the last first -- so there's no problem
   */
  while ( !services.empty ())
    services.pop_front ();

#if DEBUG
  for (std::list<boost::weak_ptr<Service> >::iterator iter = remaining_services.begin();
       iter != remaining_services.end ();
       ++iter) {

    ServicePtr service = iter->lock();
    if (service) {

      std::cout << "Ekiga::ServiceCore: "
		<< service->get_name()
		<< " hasn't been freed correctly!"
		<< std::endl;
    }
  }
#endif
}

bool
Ekiga::ServiceCore::add (ServicePtr service)
{
  bool result = false;

  if ( !get (service->get_name ())) {
    services.push_front (service);
    service_added (service);
    result = true;
  } else {

    result = false;
  }
#if DEBUG
  if (result)
    std::cout << "Ekiga::ServiceCore added " << service->get_name () << std::endl;
  else
    std::cout << "Ekiga::ServiceCore already has " << service->get_name () << std::endl;
#endif

  return result;
}

Ekiga::ServicePtr
Ekiga::ServiceCore::get (const std::string name)
{
  ServicePtr result;

  for (services_type::iterator iter = services.begin ();
       iter != services.end () && !result;
       iter++)
    if (name == (*iter)->get_name ()) {

      result = *iter;
    }


#if DEBUG
  if (result)
    std::cout << "Ekiga::ServiceCore returns " << name << std::endl;
  else
    std::cout << "Ekiga::ServiceCore doesn't have " << name << std::endl;
#endif

  return result;
}

void
Ekiga::ServiceCore::dump (std::ostream &stream) const
{
  for (services_type::const_reverse_iterator iter = services.rbegin ();
       iter != services.rend ();
       iter++)
    stream << (*iter)->get_name ()
	   << ":"
	   << std::endl
	   << (*iter)->get_description ()
	   << std::endl;
}
