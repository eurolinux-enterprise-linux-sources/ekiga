
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
 *                         gmconf-bridge.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an object able to do the bridging
 *                          between gmconf and any object
 *
 */

#include <iostream>

#include "gmconf-bridge.h"

#include "gmconf.h"


using namespace Ekiga;


static void entry_changed_nt (gpointer /*id*/,
                              GmConfEntry *entry, 
                              gpointer data);

static void 
entry_changed_nt (gpointer /*id*/,
                  GmConfEntry *entry, 
                  gpointer data)
{
  Ekiga::ConfBridge *bridge = (Ekiga::ConfBridge *) data;
  std::string key = gm_conf_entry_get_key (entry);

  bridge->property_changed (key, entry);
}


void ConfBridge::load (ConfKeys & keys)
{
  for (ConfKeys::iterator it = keys.begin ();
       it != keys.end ();
       it++) {

    gm_conf_notifier_add ((*it).c_str (), entry_changed_nt, this);
    gm_conf_notifier_trigger ((*it).c_str ());
  }
}

