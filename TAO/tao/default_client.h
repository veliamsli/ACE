// This may look like C, but it's really -*- C++ -*-

// ============================================================================
//
// = LIBRARY
//    TAO
// 
// = FILENAME
//     default_client.h
//
// = AUTHOR
//     Chris Cleeland
//
// = VERSION
//     $Id$
// ============================================================================

#if !defined (TAO_DEFAULT_CLIENT_H)
#  define TAO_DEFAULT_CLIENT_H

#  include "ace/SOCK_Acceptor.h"
#  include "ace/SOCK_Connector.h"
#  include "ace/Strategies_T.h"
#  include "ace/Connector.h"
#  include "ace/Synch.h"

#  include "tao/params.h"
#  include "tao/connect.h"
#  include "tao/objtable.h"

class TAO_Default_Client_Strategy_Factory : public TAO_Client_Strategy_Factory
// = TITLE
//   This is the "default" client strategy factor for TAO...basically, it
//   includes strategies that are configured through command-line options
//   so that everything operates as if there were no dynamically-linkable
//   strategies.
{
public:
  typedef ACE_Cached_Connect_Strategy<TAO_Client_Connection_Handler, 
                                      ACE_SOCK_CONNECTOR,
				      ACE_SYNCH_RW_MUTEX>
          CACHED_CONNECT_STRATEGY;

  TAO_Default_Client_Strategy_Factory (void);
  virtual ~TAO_Default_Client_Strategy_Factory (void);
  
  virtual TAO_Client_Strategy_Factory::CONNECTOR *connector (void);
  // Return a pointer to a connector using appropriate strategies.

  // = SERVICE CONFIGURATOR HOOKS
  virtual int init(int argc, char* argv[]);
  // Dynamic linking hook

  int parse_args(int argc, char* argv[]);
  // Parse svc.conf arguments
  
private:
#  if defined (TAO_HAS_CLIENT_CONCURRENCY)
  CONCURRENCY_STRATEGY *concurrency_strategy_;
#  endif
  CONNECTOR connector_;
  NULL_CREATION_STRATEGY null_creation_strategy_;
  CACHED_CONNECT_STRATEGY caching_connect_strategy_;
};

ACE_SVC_FACTORY_DECLARE (TAO_Default_Client_Strategy_Factory)

#endif /* TAO_DEFAULT_CLIENT_H */
