// ============================================================================
// @(#)Shared_Memory_MM_Test.cpp	1.1	10/18/96

//
// = LIBRARY
//    tests
// 
// = FILENAME
//    Shared_Memory_MM_Test.cpp
//
// = DESCRIPTION
//     This is a simple test of ACE_Shared_Memory_MM. The test forks
//     two processes or spawns two threads (depending upon the
//     platform) and then executes client and server allowing them to
//     exchange data using shared memory. No user input is required as
//     far as command line arguments are concerned.
//
// = AUTHOR
//    Prashant Jain and Doug Schmidt
// 
// ============================================================================

#include "ace/Log_Msg.h"
#include "ace/Shared_Memory_MM.h"
#include "ace/Thread.h"
#include "ace/Thread_Manager.h"
#include "test_config.h"

#define SHMSZ 27
static char shm_key[] = "/tmp/fooXXXXXX";

// Global thread manager.
static ACE_Thread_Manager thr_mgr;

static void *
client (void *dummy)
{
#if defined (ACE_WIN32)
  // Insert thread into thr_mgr
  ACE_Thread_Control thread_control (&thr_mgr);  
  ACE_NEW_THREAD;
#endif

  ACE_OS::sleep (3);
  char t = 'a';
  ACE_Shared_Memory *shm_client = new ACE_Shared_Memory_MM (shm_key);
  char *shm = (char *) shm_client->malloc ();

  for (char *s = shm; *s != '\0'; s++)
    {
      ACE_ASSERT (t == s[0]);
      t++;
    }
  *shm = '*';
#if !defined (ACE_WIN32)
  ACE_OS::exit (0);
#endif
  return 0;
}

static void *
server (void *dummy)
{
#if defined (ACE_WIN32)
  // Insert thread into thr_mgr
  ACE_Thread_Control thread_control (&thr_mgr);  
  ACE_NEW_THREAD;
#endif

  ACE_Shared_Memory *shm_server = new ACE_Shared_Memory_MM (shm_key, SHMSZ);
  char *shm = (char *) shm_server->malloc ();
  char *s   = shm;

  for (char c = 'a'; c <= 'z'; c++)
    *s++ = c;

  *s = '\0';

  // Perform a busy wait (ugh)
  while (*shm != '*')
    ACE_OS::sleep (1);

  if (shm_server->remove () < 0)
    ACE_ERROR ((LM_ERROR, "%p\n", "remove"));
  ACE_OS::unlink (shm_key);
  return 0;
}

void
spawn ()
{
#if !defined (ACE_WIN32)
  switch (ACE_OS::fork ())
    {
    case -1:
      ACE_ERROR ((LM_ERROR, "%p\n%a", "fork failed"));
      exit (-1);
    case 0: 
      client (0);
    default:
      server (0);
    }
#elif defined (ACE_HAS_THREADS)
  if (thr_mgr.spawn (ACE_THR_FUNC (client),
		     (void *) 0,
		     THR_NEW_LWP | THR_DETACHED) == -1)
    ACE_ERROR ((LM_ERROR, "%p\n%a", "thread create failed"));

  if (thr_mgr.spawn (ACE_THR_FUNC (server),
		     (void *) 0,
		     THR_NEW_LWP | THR_DETACHED) == -1)
    ACE_ERROR ((LM_ERROR, "%p\n%a", "thread create failed"));
  thr_mgr.wait ();
#else
  ACE_ERROR ((LM_ERROR, "only one thread may be run in a process on this platform\n%a", 1));
#endif /* ACE_HAS_THREADS */	
}

int
main (int argc, char *argv[])
{
  ACE_START_TEST;

  if (ACE_OS::mktemp (shm_key) == 0 || (ACE_OS::unlink (shm_key) == -1 && errno == EPERM))
    ACE_ERROR_RETURN ((LM_ERROR, "%p\n", shm_key), 1);

  spawn ();

  ACE_END_TEST;
  return 0;
}

