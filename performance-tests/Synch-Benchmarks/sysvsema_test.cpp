#include "ace/SV_Semaphore_Simple.h"
// @(#)sysvsema_test.cpp	1.1	10/18/96

#include "Options.h"
#include "Benchmark.h"

#if defined (ACE_HAS_THREADS)

class SYSVSema_Test : public Benchmark
{
public:
  virtual int svc (void);

private:
  static ACE_SV_Semaphore_Simple sema;
};

ACE_SV_Semaphore_Simple SYSVSema_Test::sema (1234, ACE_SV_Semaphore_Simple::ACE_CREATE, 1);

int
SYSVSema_Test::svc (void)
{
  int ni      = this->thr_id ();
  synch_count = 2;

  while (!this->done ())
    {
      sema.acquire ();
      options.thr_work_count[ni]++;
      buffer++;
      sema.release ();
    }

  sema.remove ();
  /* NOTREACHED */
  return 0;
}

extern "C" ACE_Service_Object *sysvsema_test (void);

ACE_Service_Object *sysvsema_test (void)
{
  return new SYSVSema_Test;
}

// ACE_Service_Object_Type st (&sysvsema_test, "SYSVSema_Test");
#endif /* ACE_HAS_THREADS */
