/* -*- C++ -*- */
// $Id$


// ============================================================================
//
// = LIBRARY
//    ace
// 
// = FILENAME
//    ACE_Dynamic.h 
//
// = AUTHOR
//    Doug Schmidt and Irfan Pyrarli.
// 
// ============================================================================

#if !defined (ACE_DYNAMIC_H)
#define ACE_DYNAMIC_H 

#include "ace/ACE.h"

class ACE_Export ACE_Dynamic
  // = TITLE
  //     Checks to see if a Svc_Handler was dynamically allocated.
  //
  // = DESCRIPTION
  //     This class holds the pointer in a thread-safe manner between
  //     the call to operator new and the call to the constructor.
{
public:
  ACE_Dynamic (void);

  void *set (void *x);
  // Assign the new pointer to <instance_> in order to keep it safe
  // until we can compare it in the constructor.

  int is_dynamic (void *x);
  // 1 if we were allocated dynamically, else 0.

private:
  void *instance_;
  // Holds the pointer in a thread-safe manner between the call to
  // operator new and the call to the constructor.
};

#if defined (__ACE_INLINE__)
#include "ace/Dynamic.i"
#endif /* __ACE_INLINE__ */

#endif /* ACE_DYNAMIC_H */

