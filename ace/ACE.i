/* -*- C++ -*- */
// $Id$

// ACE.i
// Miscellaneous static methods used throughout ACE.

inline ssize_t
ACE::send (ACE_HANDLE handle, const void *buf, size_t len)
{
  ACE_TRACE ("ACE::send");
  
#if defined (ACE_WIN32)
  return ACE_OS::send (handle, (const char *) buf, len);
#else
  return ACE_OS::write (handle, (const char *) buf, len);
#endif /* ACE_WIN32 */
}

inline ssize_t
ACE::send (ACE_HANDLE handle, const void *buf, size_t len, int flags)
{
  ACE_TRACE ("ACE::send");
  return ACE_OS::send (handle, (const char *) buf, len, flags);
}

inline ssize_t
ACE::recv (ACE_HANDLE handle, void *buf, size_t len)
{
  ACE_TRACE ("ACE::recv");
#if defined (ACE_WIN32)
    return ACE_OS::recv (handle, (char *) buf, len);
#else
    return ACE_OS::read (handle, (char *) buf, len);
#endif /* ACE_WIN32 */
}

inline ssize_t
ACE::recv (ACE_HANDLE handle, void *buf, size_t len, int flags)
{
  ACE_TRACE ("ACE::recv");

  return ACE_OS::recv (handle, (char *) buf, len, flags);
}

inline char *
ACE::strecpy (char *s, const char *t)
{
  ACE_TRACE ("ACE::strecpy");
  register char *dscan = s;
  register const char *sscan = t;

  while ((*dscan++ = *sscan++) != '\0')
    continue;

  return dscan - 1;
}

// Return flags currently associated with handle.

inline int
ACE::get_flags (ACE_HANDLE handle)
{
  ACE_TRACE ("ACE::get_flags");

  return ACE_OS::fcntl (handle, F_GETFL, 0);
}

