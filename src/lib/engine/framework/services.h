
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
 *                         services.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a service object
 *
 */

#ifndef __SERVICES_H__
#define __SERVICES_H__

/* We want to register some named services to a central location : this is
 * it!
 */

#include <boost/smart_ptr.hpp>
#include <boost/optional.hpp>

#include <list>
#include <string>
#include <boost/signals.hpp>
#include <boost/bind.hpp>

namespace Ekiga
{

/**
 * @defgroup services Services
 * @{
 */

  struct Service
  {
    virtual ~Service () {}

    virtual const std::string get_name () const = 0;

    virtual const std::string get_description () const = 0;

    /* beware that if you check the result directly (in an if, or passing it
     * through a function, the obtained value will be wether or not the value
     * is available, and not the value itself!
     *
     * To be more specific, you're supposed to:
     * val = foo.get_bool_property ("bar");
     * if (val) {
     *   <do something with *val> (notice *val not val!)
     * }
     */
    virtual boost::optional<bool> get_bool_property (const std::string) const;

    virtual boost::optional<int> get_int_property (const std::string) const;

    virtual boost::optional<std::string> get_string_property (const std::string) const;
  };
  typedef boost::shared_ptr<Service> ServicePtr;


  class ServiceCore
  {
  public:

    ServiceCore () {}

    ~ServiceCore ();

    bool add (ServicePtr service);

    ServicePtr get (const std::string name);

    template<typename T>
    boost::shared_ptr<T> get (const std::string name)
    { return boost::dynamic_pointer_cast<T> (get (name)); }

    void dump (std::ostream &stream) const;

    boost::signal1<void, ServicePtr> service_added;

  private:

    typedef std::list<ServicePtr> services_type;
    services_type services;

  };

  typedef boost::shared_ptr<ServiceCore> ServiceCorePtr;

  class BasicService: public Service
  {
  public:

    BasicService (const std::string name_,
		  const std::string description_):
      name(name_), description(description_)
    {}

    const std::string get_name () const
    { return name; }

    const std::string get_description () const
    { return description; }

  private:

    std::string name;
    std::string description;
  };

/**
 * @}
 */

};

#endif
