// ACE.cpp
// $Id$

#define ACE_BUILD_DLL
#include "ace/IPC_SAP.h"
#include "ace/Time_Value.h"
#include "ace/Handle_Set.h"
#include "ace/ACE.h"
#include "ace/Thread_Manager.h"
#include "ace/Reactor.h"

int
ACE::register_stdin_handler (ACE_Event_Handler *eh,
			     ACE_Reactor *reactor,
			     ACE_Thread_Manager *thr_mgr,
			     int flags)
{
#if defined (ACE_WIN32)
  return thr_mgr->spawn (&ACE::read_adapter, eh, flags);
#else  
  // Keep compilers happy.
  flags = flags;
  thr_mgr = thr_mgr;
  return reactor->register_handler (ACE_STDIN, eh, ACE_Event_Handler::READ_MASK);
#endif /* ACE_WIN32 */
}

// Used to read from non-socket ACE_HANDLEs in our own thread to work
// around Win32 limitations that don't allow us to select() on
// non-sockets (such as ACE_STDIN).  This is commonly used in
// situations where the Reactor is used to demultiplex read events on
// ACE_STDIN on UNIX.  Note that <event_handler> must be a subclass of
// <ACE_Event_Handler>.  If the <get_handle> method of this event
// handler returns <ACE_INVALID_HANDLE> we default to reading from
// ACE_STDIN.

void *
ACE::read_adapter (void *t)
{
  ACE_Event_Handler *this_ptr = (ACE_Event_Handler *) t;
  ACE_HANDLE handle = this_ptr->get_handle ();

  if (handle == ACE_INVALID_HANDLE)
    handle = ACE_STDIN;

  while (this_ptr->handle_input (handle) != -1)
    continue;

  this_ptr->handle_close (handle, ACE_Event_Handler::READ_MASK);
  return 0;
}

const char *
ACE::execname (const char *old_name)
{
#if defined (ACE_HAS_WIN32)
  if (ACE_OS::strstr (old_name, ".exe") == 0)
    {
      char *new_name;

      ACE_NEW_RETURN (new_name, char[ACE_OS::strlen (old_name) +
				     ACE_OS::strlen (".exe") + 
				     1], -1);
      char *end = new_name;
      end = ACE::strecpy (new_name, old_name);
      // Concatenate the .exe suffix onto the end of the executable.
      ACE_OS::strcpy (end, ".exe");
      return new_name;
    }
#endif /* ACE_HAS_WIN32 */
  return old_name;
}

u_long
ACE::hash_pjw (const char *str)
{
  u_long hash = 0;
   
  for (const char *temp = str; *temp != 0; temp++)
    {
      hash = (hash << 4) + (*temp * 13);

      u_long g = hash & 0xf0000000;

      if (g)
        {
          hash ^= (g >> 24);
          hash ^= g;
        }
    }

  return hash;
}

char *
ACE::strenvdup (const char *str)
{
  ACE_TRACE ("ACE::strenvdup");

  char *temp;

  if (str[0] == '$'
      && (temp = ACE_OS::getenv (&str[1])) != 0)
    return ACE_OS::strdup (temp);
  else
    return ACE_OS::strdup (str);
}

int
ACE::ldfind (const char *filename, 
	     char *pathname, 
	     size_t maxlen)
{
  ACE_TRACE ("ACE::ldfind");
  if (ACE_OS::strchr (filename, ACE_DIRECTORY_SEPARATOR_CHAR) != 0)
    {
      // Use absolute pathname.
      ACE_OS::strncpy (pathname, filename, maxlen);
      return 0;
    }
  else
    {
      // Using LD_LIBRARY_PATH
      char *ld_path = ACE_OS::getenv (ACE_LD_SEARCH_PATH);

      if (ld_path != 0 && (ld_path = ACE_OS::strdup (ld_path)) != 0)
	{
	  // Look at each dynamic lib directory in the search path.
	  char *path_entry = ACE_OS::strtok (ld_path,
					     ACE_LD_SEARCH_PATH_SEPARATOR_STR);
      
	  int result = 0;

	  while (path_entry != 0)
	    {
	      if (ACE_OS::strlen (path_entry) + 1 + ACE_OS::strlen (filename) >= maxlen)
		{
		  errno = ENOMEM;
		  result = -1;
		  break;
		}
	      ACE_OS::sprintf (pathname, "%s%c%s", 
			       path_entry,
			       ACE_DIRECTORY_SEPARATOR_CHAR, 
			       filename);

	      if (ACE_OS::access (pathname, R_OK) == 0)
		break;
	      path_entry = ACE_OS::strtok (0, ACE_LD_SEARCH_PATH_SEPARATOR_STR);
	    }

	  ACE_OS::free ((void *) ld_path);
	  return result;
	}
    }

  errno = ENOENT;
  return -1;
}

FILE *
ACE::ldopen (const char *filename, const char *type)
{
  ACE_TRACE ("ACE::ldopen");
  char buf[MAXPATHLEN];

  if (ACE::ldfind (filename, buf, sizeof buf) == -1)
    return 0;
  else
    return ACE_OS::fopen (buf, type);
}

const char *
ACE::basename (const char *pathname, char delim)
{
  ACE_TRACE ("ACE::basename");
  const char *temp = ::strrchr (pathname, delim);
    
  if (temp == 0)
    return pathname;
  else
    return temp + 1;
}

// Miscellaneous static methods used throughout ACE.

ssize_t
ACE::send_n (ACE_HANDLE handle, const void *buf, size_t len)
{
  ACE_TRACE ("ACE::send_n");
  size_t bytes_written;
  int	 n;

  for (bytes_written = 0; bytes_written < len; bytes_written += n)
    if ((n = ACE::send (handle, (const char *) buf + bytes_written, 
			len - bytes_written)) == -1)
      return -1;

  return bytes_written;
}

ssize_t
ACE::send_n (ACE_HANDLE handle, const void *buf, size_t len, int flags)
{
  ACE_TRACE ("ACE::send_n");
  size_t bytes_written;
  int	 n;

  for (bytes_written = 0; bytes_written < len; bytes_written += n)
    if ((n = ACE_OS::send (handle, (const char *) buf + bytes_written, 
			   len - bytes_written, flags)) == -1)
      return -1;

  return bytes_written;
}

ssize_t
ACE::recv_n (ACE_HANDLE handle, void *buf, size_t len)
{
  ACE_TRACE ("ACE::recv_n");
  size_t bytes_read;
  int	 n;

  for (bytes_read = 0; bytes_read < len; bytes_read += n)
    if ((n = ACE::recv (handle, (char *) buf + bytes_read,
			len - bytes_read)) == -1)
      return -1;
    else if (n == 0)
      break;

  return bytes_read;      
}

ssize_t
ACE::recv_n (ACE_HANDLE handle, void *buf, size_t len, int flags)
{
  ACE_TRACE ("ACE::recv_n");
  size_t bytes_read;
  int	 n;

  for (bytes_read = 0; bytes_read < len; bytes_read += n)
    if ((n = ACE_OS::recv (handle, (char *) buf + bytes_read, 
			   len - bytes_read, flags)) == -1)
      return -1;
    else if (n == 0)
      break;

  return bytes_read;      
}

  // Receive <len> bytes into <buf> from <handle> (uses the <read>
  // system call on UNIX and the <ReadFile> call on Win32).
ssize_t 
ACE::read_n (ACE_HANDLE handle, 
	     void *buf, 
	     size_t len)
{
  ACE_TRACE ("ACE::read_n");

  size_t bytes_read;
  int	 n;

  for (bytes_read = 0; bytes_read < len; bytes_read += n)
    if ((n = ACE_OS::read (handle, (char *) buf + bytes_read,
			   len - bytes_read)) == -1)
      return -1;
    else if (n == 0)
      break;

  return bytes_read;      
}

// Receive <len> bytes into <buf> from <handle> (uses the <write>
// system call on UNIX and the <WriteFile> call on Win32).

ssize_t 
ACE::write_n (ACE_HANDLE handle, 
	      const void *buf, 
	      size_t len)
{
  ACE_TRACE ("ACE::write_n");

  size_t bytes_written;
  int	 n;

  for (bytes_written = 0; bytes_written < len; bytes_written += n)
    if ((n = ACE_OS::write (handle, (const char *) buf + bytes_written, 
			    len - bytes_written)) == -1)
      return -1;

  return bytes_written;
}

// Format buffer into printable format.  This is useful for debugging.
// Portions taken from mdump by J.P. Knight (J.P.Knight@lut.ac.uk)
// Modifications by Todd Montgomery.

int 
ACE::format_hexdump (char *buffer, int size, char *obuf, int obuf_sz)
{
  ACE_TRACE ("ACE::format_hexdump");

  u_char c;
  char textver[16 + 1];

  int maxlen = (obuf_sz / 68) * 16;

  if (size > maxlen)
    size = maxlen;

  int i;

  for (i = 0; i < (size >> 4); i++)
    {
      int j;

      for (j = 0 ; j < 16; j++) 
	{
	  c = buffer[(i << 4) + j];
	  ::sprintf (obuf, "%02x ", c);
	  obuf += 3;
	  if (j == 7)
	    {
	      ::sprintf (obuf, " ");
	      obuf++;
	    }
	  textver[j] = (c < 0x20 || c > 0x7e) ? '.' : c;
	}

      textver[j] = 0;

      ::sprintf (obuf, "  %s\n", textver);

      while (*obuf != '\0') 
	obuf++;
    }

  if (size % 16)
    {
      for (i = 0 ; i < size % 16; i++) 
	{
	  c = buffer[size - size % 16 + i];
	  ::sprintf (obuf,"%02x ",c);
	  obuf += 3;
	  if (i == 7)
	    {
	      ::sprintf (obuf, " ");
	      obuf++;
	    }
	  textver[i] = (c < 0x20 || c > 0x7e) ? '.' : c;
	}

      for (i = size % 16; i < 16; i++) 
	{
	  ::sprintf (obuf, "   ");
	  obuf += 3;
	  textver[i] = ' ';
	}	

      textver[i] = 0;
      ::sprintf (obuf, "  %s\n", textver);
    }
  return size;
}

// Returns the current timestamp in the form
// "hour:minute:second:microsecond."  The month, day, and year are
// also stored in the beginning of the date_and_time array.  Returns 0
// if unsuccessful, else returns pointer to beginning of the "time"
// portion of <day_and_time>.

char *
ACE::timestamp (char date_and_time[], int date_and_timelen)
{
  ACE_TRACE ("ACE::timestamp");

  if (date_and_timelen < 35)
    {
      errno = EINVAL;
      return 0;
    }

#if defined (WIN32)
  // @@ Jesper, I think Win32 supports all the UNIX versions below.
  // Therefore, we can probably remove this WIN32 ifdef altogether.
  SYSTEMTIME local;
  ::GetLocalTime (&local);

  ACE_OS::sprintf (date_and_time, "%02d/%02d/%04d%02d.%02d.%02d.%06d",
                   (int) local.wMonth, // new, also the %02d in sprintf
                   (int) local.wDay,   // new, also the %02d in sprintf
                   (int) local.wYear,  // new, also the %02d in sprintf
		   (int) local.wHour,
		   (int) local.wMinute,
		   (int) local.wSecond,
		   (int) local.wMilliseconds * 1000);
#else  // UNIX
  char timebuf[26]; // This magic number is based on the ctime(3c) man page.
  ACE_Time_Value cur_time = ACE_OS::gettimeofday ();
  time_t secs = cur_time.sec ();
  ACE_OS::ctime_r (&secs, timebuf, sizeof timebuf);
  ACE_OS::strncpy (date_and_time, timebuf, date_and_timelen);
  ACE_OS::sprintf (&date_and_time[19], ".%06d", cur_time.usec ());
#endif /* WIN32 */
  date_and_time[26] = '\0';        
  return &date_and_time[11];
}

// This function rounds the request to a multiple of the page size.

size_t
ACE::round_to_pagesize (off_t len)
{
  ACE_TRACE ("ACE::round_to_pagesize");
  return (len + (ACE_PAGE_SIZE - 1)) & ~(ACE_PAGE_SIZE - 1);
}

ACE_HANDLE 
ACE::handle_timed_complete (ACE_HANDLE h,
			    ACE_Time_Value *timeout)
{
  ACE_TRACE ("ACE::handle_timed_complete");
  ACE_Handle_Set rd_handles;
  ACE_Handle_Set wr_handles;

  rd_handles.set_bit (h);
  wr_handles.set_bit (h);

  int n = ACE_OS::select (int (h) + 1, 
			  rd_handles,
			  wr_handles, 
			  0, timeout);
  // If we failed to connect within the time period allocated by the
  // caller, then we fail (e.g., the remote host might have been too
  // busy to accept our call).
  if (n <= 0)
    {
      if (n == 0)
	errno = ETIME;
      return ACE_INVALID_HANDLE;
    }
  // Check if the handle is ready for reading and the handle is *not*
  // ready for writing, which may indicate a problem.  But we need to
  // make sure...
#if defined (ACE_HAS_TLI)
  else if (rd_handles.is_set (h) && !wr_handles.is_set (h))
#else
  else if (rd_handles.is_set (h))
#endif /* ACE_HAS_TLI */
    {
      char dummy;
      // The following recv() won't block provided that the
      // ACE_NONBLOCK flag has not been turned off .

      n = ACE::recv (h, &dummy, 1, MSG_PEEK);
      if (n <= 0)
	{
	  if (n == 0)
	    errno = ECONNREFUSED;
	  return ACE_INVALID_HANDLE;
	}
    }

  // 1. The HANDLE is ready for writing or 2. recv() returned that
  // there are data to be read, which indicates the connection was
  // successfully established.
  return h;   
}

ACE_HANDLE
ACE::handle_timed_open (ACE_Time_Value *timeout,
			LPCTSTR name,
			int flags, 
			int perms)
{
  ACE_TRACE ("ACE::handle_timed_open");

  if (timeout != 0)
    {
      // Open the named pipe or file using non-blocking mode...
      ACE_HANDLE handle = ACE_OS::open (name, 
					flags | ACE_NONBLOCK, 
					perms);
      if (handle == ACE_INVALID_HANDLE
	  && (errno == EWOULDBLOCK 
	      && (timeout->sec () > 0 || timeout->usec () > 0)))
	// This expression checks if we were polling.
	errno = ETIMEDOUT;

      return handle;
    }
  else
    return ACE_OS::open (name, flags, perms);
}

// Wait up to <timeout> amount of time to accept a connection.

int 
ACE::handle_timed_accept (ACE_HANDLE listener, 
			  ACE_Time_Value *timeout,
			  int restart)
{
  ACE_TRACE ("ACE::handle_timed_accept");
  // Make sure we don't bomb out on erroneous values.
  if (listener == ACE_INVALID_HANDLE)
    return -1;

  // Use the select() implementation rather than poll().
  ACE_Handle_Set rd_handle;
  rd_handle.set_bit (listener);

  // We need a loop here if <restart> is enabled.

  for (;;)
    {
      switch (ACE_OS::select (int (listener) + 1, 
			      rd_handle, 0, 0, 
			      timeout))
	{
	case -1:
	  if (errno == EINTR && restart)
	    continue;
	  else
	    return -1;
	  /* NOTREACHED */
	case 0:
	  if (timeout != 0 && timeout->sec() == 0 && timeout->usec() == 0)
	    errno = EWOULDBLOCK;
	  else
	    errno = ETIMEDOUT;
	  return -1;
	  /* NOTREACHED */
	case 1:
	  return 0;
	  /* NOTREACHED */
	default:
	  errno = EINVAL;
	  return -1;
	  /* NOTREACHED */
	}
    }
  /* NOTREACHED */
  return 0;
}

// Bind socket to an unused port.

int
ACE::bind_port (ACE_HANDLE handle)
{
  ACE_TRACE ("ACE::bind_port");
  sockaddr_in sin;
  const int MAX_SHORT = 65535;
  static int upper_limit = MAX_SHORT;
  int lower_limit = IPPORT_RESERVED;
  int round_trip = upper_limit;

  ACE_OS::memset ((void *) &sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
#if defined (ACE_HAS_SIN_LEN)
  sin.sin_family = sizeof sin;
#endif /* ACE_HAS_SIN_LEN */
  sin.sin_addr.s_addr = INADDR_ANY;

  for (;;)
    {
      sin.sin_port = htons (upper_limit);

      if (ACE_OS::bind (handle, (sockaddr *) &sin, sizeof sin) >= 0)
	return 0;
      else if (errno != EADDRINUSE)
	return -1;
      else
	{
	  upper_limit--;
		  
	  /* Wrap back around when we reach the bottom. */
	  if (upper_limit <= lower_limit)
	    upper_limit = MAX_SHORT;
		  
	  /* See if we have already gone around once! */
	  if (upper_limit == round_trip)
	    {
	      errno = EAGAIN;	
	      return -1;
	    }
	}
    }
}

// Make the current process a UNIX daemon.  This is based on Stevens
// code from APUE.

int
ACE::daemonize (void)
{
  ACE_TRACE ("ACE::daemonize");
#if !defined (ACE_WIN32)
  pid_t pid;

  if ((pid = ACE_OS::fork ()) == -1)
    return -1;
  else if (pid != 0)
    ACE_OS::exit (0);			/* parent exits */

  /* child continues */
  ACE_OS::setsid (); /* become session leader */

  ACE_OS::chdir ("/");		/* change working directory */

  ACE_OS::umask (0);			/* clear our file mode creation mask */
  return 0;
#else
  ACE_NOTSUP_RETURN (-1);
#endif /* ACE_WIN32 */
}

int
ACE::max_handles (void)
{
  ACE_TRACE ("ACE::set_handle_limit");
#if defined (_SC_OPEN_MAX)
  return ACE_OS::sysconf (_SC_OPEN_MAX);
#elif defined (RLIMIT_NOFILE)
  rlimit rl;
  ACE_OS::getrlimit (RLIMIT_NOFILE, &rl);
  rl.rlim_cur;
#else
  ACE_NOTSUP_RETURN (-1);
#endif /* ACE_WIN32 */
}

// Set the number of currently open handles in the process.
//
// If NEW_LIMIT == -1 set the limit to the maximum allowable.
// Otherwise, set it to be the value of NEW_LIMIT.

int 
ACE::set_handle_limit (int new_limit)
{
  ACE_TRACE ("ACE::set_handle_limit");
#if defined (RLIMIT_NOFILE)
  struct rlimit rl;

  int max_handles = ACE::max_handles ();

  if (new_limit < 0 || new_limit > max_handles)
    rl.rlim_cur = max_handles;
  else
    rl.rlim_cur = new_limit;

  return ACE_OS::setrlimit (RLIMIT_NOFILE, &rl);
#else
  ACE_NOTSUP_RETURN (-1);
#endif /* ACE_WIN32 */
}

// Flags are file status flags to turn on.

int
ACE::set_flags (ACE_HANDLE handle, int flags)
{
  ACE_TRACE ("ACE::set_flags");
#if defined (ACE_WIN32)
  switch (flags)
    {
    case ACE_NONBLOCK:
      // nonblocking argument (1) 
      // blocking:            (0)
      {
	u_long nonblock = 1;
	return ACE_OS::ioctl (handle, FIONBIO, &nonblock);
      }
    default:
      errno = ENOTSUP;
      return -1;
    }
#else
  int val = ACE_OS::fcntl (handle, F_GETFL, 0);

  if (val == -1)
    return -1;

  // Turn on flags.
  ACE_SET_BITS (val, flags); 

  if (ACE_OS::fcntl (handle, F_SETFL, val) == -1)
    return -1;
  else
    return 0;
#endif /* ACE_WIN32 */
}

// Flags are the file status flags to turn off.

int
ACE::clr_flags (ACE_HANDLE handle, int flags)
{
  ACE_TRACE ("ACE::clr_flags");

#if defined (ACE_WIN32)
  switch (flags)
    {
    case ACE_NONBLOCK:
      // nonblocking argument (1) 
      // blocking:            (0)
      {
	u_long nonblock = 0;
	return ACE_OS::ioctl (handle, FIONBIO, &nonblock);
      }
    default:
      errno = ENOTSUP;
      return -1;
    }
#else
  int val = ACE_OS::fcntl (handle, F_GETFL, 0);

  if (val == -1)
    return -1;

  // Turn flags off.
  ACE_CLR_BITS (val, flags); 

  if (ACE_OS::fcntl (handle, F_SETFL, val) == -1)
    return -1;
  else
    return 0;
#endif /* ACE_WIN32 */
}

int
ACE::map_errno (int error)
{
  switch (error)
    {
#if defined (ACE_WIN32)
    case WSAEWOULDBLOCK:
      return EAGAIN; // Same as UNIX errno EWOULDBLOCK.
#endif /* ACE_WIN32 */
    }

  return error;
}

ssize_t
ACE::send (ACE_HANDLE handle, 
	   const void *buf, 
	   size_t n, 
	   int flags, 
	   const ACE_Time_Value *tv)
{
  if (tv == 0)
    return ACE::send (handle, buf, n, flags);
  else
    {
      // We need to record whether we are already *in* nonblocking mode,
      // so that we can correctly reset the state when we're done.

      int val = ACE::get_flags (handle);

      if (ACE_BIT_ENABLED (val, ACE_NONBLOCK) == 0)
	// Set the descriptor into non-blocking mode if it's not already
	// in it.
	ACE::set_flags (handle, ACE_NONBLOCK);

      ACE_Time_Value timeout (*tv);
      ACE_Time_Value start_time = ACE_OS::gettimeofday();

      ssize_t bytes_written;

      // Use the non-timeout version to do the actual send.
      bytes_written = ACE::send (handle, buf, n, flags);

      if (bytes_written == -1 && errno == EWOULDBLOCK) 
	{
	  // We couldn't send due to flow control.

	  // Compute time that has elapsed thus far.
	  ACE_Time_Value elapsed_time = ACE_OS::gettimeofday () - start_time;

	  if (elapsed_time > timeout) 
	    // We've timed out, so break;
	    errno = ETIME;
	  else
	    {
	      // Update the timeout.
	      timeout -= elapsed_time;
      
	      ACE_Handle_Set handle_set;

	      handle_set.set_bit (handle);

	      switch (ACE_OS::select (int (handle) + 1,
				      0, // read_fds.
				      handle_set, // write_fds.
				      0, // exception_fds.
				      timeout))
		{
		case 0:
		  errno = ETIME;
		  /* FALLTHRU */
		default:
		  /* FALLTHRU */
		case -1:
		  break;
		case 1:
		  // We should be able to send something now.
		  bytes_written = ACE::send (handle, buf, n, flags);
		  break;
		}
	    }
	}

      if (ACE_BIT_ENABLED (val, ACE_NONBLOCK) == 0)
	{
	  // We need to stash errno here because ACE::clr_flags() may
	  // reset it.
	  int error = errno;

	  // Only disable ACE_NONBLOCK if we weren't in non-blocking mode
	  // originally.
	  ACE::clr_flags (handle, ACE_NONBLOCK);
	  errno = error;
	}

      return bytes_written;
    }				 
}

int
ACE::recv (ACE_HANDLE handle, 
	   void *buf, 
	   size_t n, 
	   int flags, 
	   const ACE_Time_Value *tv)
{
  if (tv == 0)
    return ACE::recv (handle, buf, n, flags);
  else
    {
      ACE_Handle_Set handle_set;

      handle_set.set_bit (handle);

      switch (ACE_OS::select (int (handle) + 1,
			      (fd_set *) handle_set, // read_fds.
			      (fd_set *) 0, // write_fds.
			      (fd_set *) 0, // exception_fds.
			      tv))
	{
	case -1:
	  return -1;
	case 0:
	  errno = ETIME;
	  return -1;
	default:
	  return ACE::recv (handle, buf, n, flags);
	}
    }
}
