#define ACE_BUILD_SVC_DLL
// @(#)CCM_App.cpp	1.1	10/18/96

#include "ace/Stream.h"
#include "ace/Task.h"
#include "ace/Module.h"

typedef ACE_Task<ACE_SYNCH> MT_Task;
typedef ACE_Stream<ACE_SYNCH> MT_Stream;
typedef ACE_Module<ACE_SYNCH> MT_Module;

class ACE_Svc_Export Test_Task : public MT_Task
{
public:
  virtual int open (void *);
  virtual int close (u_long);
  virtual int put (ACE_Message_Block *, ACE_Time_Value * = 0);
  virtual int svc (void);
  virtual int info (char **, size_t) const;
  virtual int init (int, char *[]);
  virtual int fini (void);
  virtual int suspend (void);
  virtual int resume (void);
};

int 
Test_Task::open (void *)
{ 
  ACE_DEBUG ((LM_DEBUG, "opening %s\n", this->name () ? this->name () : "task"));
  return 0;
}

int 
Test_Task::close (u_long) 
{
  ACE_DEBUG ((LM_DEBUG, "closing %s\n", this->name () ? this->name () : "task"));
  return 0;
}

int 
Test_Task::suspend (void) 
{ 
  ACE_DEBUG ((LM_DEBUG, "suspending in %s\n", this->name () ? this->name () : "task")); 
  return 0; 
}

int 
Test_Task::resume (void) 
{
  ACE_DEBUG ((LM_DEBUG, "resuming in %s\n", this->name () ? this->name () : "task"));
  return 0;
}

int 
Test_Task::put (ACE_Message_Block *, ACE_Time_Value *)
{
  return 0;
}

int 
Test_Task::svc (void)
{
  return 0;
}

int
Test_Task::info (char **, size_t) const
{
  return 0;
}

int 
Test_Task::init (int, char *[])
{
  ACE_DEBUG ((LM_DEBUG, "initializing %s\n", this->name () ? this->name () : "task"));

  return 0; 
}

int 
Test_Task::fini (void)
{
  ACE_DEBUG ((LM_DEBUG, "finalizing %s\n", this->name () ? this->name () : "task"));
  return 0; 
}

// Dynamically linked functions used to control configuration.

extern "C" ACE_Svc_Export MT_Stream *make_stream (void);
extern "C" ACE_Svc_Export MT_Module *make_da (void);
extern "C" ACE_Svc_Export MT_Module *make_ea (void);
extern "C" ACE_Svc_Export MT_Module *make_mr (void);
extern "C" ACE_Svc_Export ACE_Service_Object *make_task (void);

ACE_Service_Object *
make_task (void)
{
  return new Test_Task;
}

MT_Stream *
make_stream (void)
{
  return new MT_Stream;
}

MT_Module *
make_da (void)
{
  return new MT_Module ("Device_Adapter", new Test_Task, new Test_Task);
}

MT_Module *
make_ea (void)
{
  return new MT_Module ("Event_Analyzer", new Test_Task, new Test_Task);
}

MT_Module *
make_mr (void)
{
  return new MT_Module ("Multicast_Router", new Test_Task, new Test_Task);
}
