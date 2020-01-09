
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
 *                         call-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call core.
 *                          A call core manages CallManagers.
 *
 */

#ifndef __CALL_CORE_H__
#define __CALL_CORE_H__

#include "form-request.h"
#include "chain-of-responsibility.h"
#include "services.h"
#include "call.h"
#include "call-manager.h"
#include "call-protocol-manager.h"
#include <boost/smart_ptr.hpp>

#include <boost/signals.hpp>
#include <boost/bind.hpp>
#include <set>
#include <map>
#include <iostream>


namespace Ekiga
{

/**
 * @defgroup calls Calls and protocols
 * @{
 */

  class CallManager;

  class CallCore
    : public Service
    {

  public:
      typedef std::set<boost::shared_ptr<CallManager> >::iterator iterator;
      typedef std::set<boost::shared_ptr<CallManager> >::const_iterator const_iterator;

      /** The constructor
       */
      CallCore () { nr_ready = 0; }

      /** The destructor
       */
      ~CallCore ();


      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "call-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tCall Core managing Call Manager objects"; }


      /** Adds a call handled by the CallCore serice.
       * @param call is the call to be added.
       * @param manager is the CallManager handling it.
       */
      void add_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);

      /** Remove a call handled by the CallCore serice.
       * @param call is the call to be removed.
       */
      void remove_call (boost::shared_ptr<Call> call);

      /** Adds a CallManager to the CallCore service.
       * @param The manager to be added.
       */
      void add_manager (boost::shared_ptr<CallManager> manager);

      /** Return iterator to beginning
       * @return iterator to beginning
       */
      iterator begin ();
      const_iterator begin () const;

      /** Return iterator to end
       * @return iterator to end 
       */
      iterator end ();
      const_iterator end () const;

      /** This signal is emitted when a Ekiga::CallManager has been
       * added to the CallCore Service.
       */
      boost::signal1<void, boost::shared_ptr<CallManager> > manager_added;


      /*** Call Management ***/                 

      /** Create a call based on the remote uri given as parameter
       * @param: an uri to call
       * @return: true if a Ekiga::Call could be created
       */
      bool dial (const std::string uri); 

      /*** Call Related Signals ***/
      
      /** See call.h for the API
       */
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > ringing_call;
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > setup_call;
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > missed_call;
      boost::signal3<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call>, std::string> cleared_call;
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > established_call;
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > held_call;
      boost::signal2<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call> > retrieved_call;
      boost::signal5<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call>, std::string, Call::StreamType, bool> stream_opened;
      boost::signal5<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call>, std::string, Call::StreamType, bool> stream_closed;
      boost::signal4<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call>, std::string, Call::StreamType> stream_paused;
      boost::signal4<void, boost::shared_ptr<CallManager> , boost::shared_ptr<Call>, std::string, Call::StreamType> stream_resumed;

      /*** Misc ***/
      boost::signal1<void, boost::shared_ptr<CallManager> > manager_ready;
      boost::signal0<void> ready;

      /** This chain allows the CallCore to report errors to the user
       */
      ChainOfResponsibility<std::string> errors;

  private:
      void on_new_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_ringing_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_setup_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_missed_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_cleared_call (std::string, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_established_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_held_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_retrieved_call (boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_stream_opened (std::string name, Call::StreamType type, bool is_transmitting, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_stream_closed (std::string name, Call::StreamType type, bool is_transmitting, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_stream_paused (std::string name, Call::StreamType type, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);
      void on_stream_resumed (std::string name, Call::StreamType type, boost::shared_ptr<Call> call, boost::shared_ptr<CallManager> manager);

      void on_im_failed (std::string, std::string, boost::shared_ptr<CallManager> manager);
      void on_im_sent (std::string, std::string, boost::shared_ptr<CallManager> manager);
      void on_im_received (std::string, std::string, std::string, boost::shared_ptr<CallManager> manager);
      void on_new_chat (std::string, std::string, boost::shared_ptr<CallManager> manager);

      void on_manager_ready (boost::shared_ptr<CallManager> manager);

      void on_call_removed (boost::shared_ptr<Call> call);

      
      std::set<boost::shared_ptr<CallManager> > managers;
      std::list<boost::signals::connection> manager_connections;
      std::map<std::string, std::list<boost::signals::connection> > call_connections;
      unsigned nr_ready;
    };

/**
 * @}
 */

};


#endif
