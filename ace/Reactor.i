/* -*- C++ -*- */
// $Id$

// Reactor.i

inline int
ACE_Reactor::resume_handler (ACE_Event_Handler *h)
{
  ACE_TRACE ("ACE_Reactor::resume_handler");
  return this->resume (h->get_handle ());
}

inline int
ACE_Reactor::suspend_handler (ACE_Event_Handler *h)
{
  ACE_TRACE ("ACE_Reactor::suspend_handler");
  return this->suspend (h->get_handle ());
}

inline int
ACE_Reactor::register_handler (int signum, 
			       ACE_Event_Handler *new_sh, 
			       ACE_Sig_Action *new_disp, 
			       ACE_Event_Handler **old_sh,
			       ACE_Sig_Action *old_disp)
{
  ACE_TRACE ("ACE_Reactor::register_handler");
  return this->signal_handler_->register_handler (signum, 
						  new_sh, new_disp,
						  old_sh, old_disp); 
}

inline int
ACE_Reactor::handler (int signum, ACE_Event_Handler **handler)
{
  ACE_TRACE ("ACE_Reactor::handler");
  return this->handler_i (signum, handler);
}

inline int
ACE_Reactor::remove_handler (int signum, 
			     ACE_Sig_Action *new_disp, 
			     ACE_Sig_Action *old_disp,
			     int sigkey)
{
  ACE_TRACE ("ACE_Reactor::remove_handler");
  return this->signal_handler_->remove_handler (signum, new_disp, old_disp, sigkey);
}

// The remaining methods in this file must be called with locks held

// Note the queue handles its own locking.

inline int
ACE_Reactor::cancel_timer (ACE_Event_Handler *handler)
{
  ACE_TRACE ("ACE_Reactor::cancel_timer");
  return this->timer_queue_->cancel (handler);
}

inline int
ACE_Reactor::cancel_timer (int timer_id, const void **arg)
{
  ACE_TRACE ("ACE_Reactor::cancel_timer");
  return this->timer_queue_->cancel (timer_id, arg);
}

// Performs operations on the "ready" bits.

inline int
ACE_Reactor::ready_ops (ACE_Event_Handler *handler, 
			ACE_Reactor_Mask mask, 
			int ops)
{
  ACE_TRACE ("ACE_Reactor::ready_ops");
  return this->ready_ops (handler->get_handle (), mask, ops);
}

// Performs operations on the "dispatch" masks.

inline int
ACE_Reactor::mask_ops (ACE_Event_Handler *handler, 
		       ACE_Reactor_Mask mask, 
		       int ops)
{
  ACE_TRACE ("ACE_Reactor::mask_ops");
  return this->mask_ops (handler->get_handle (), mask, ops);
}

inline int 
ACE_Reactor::any_ready (void) const
{
  ACE_TRACE ("ACE_Reactor::any_ready");
  ACE_Sig_Guard sb;
  
  int result = this->rd_handle_mask_ready_.num_set () > 0
    || this->wr_handle_mask_ready_.num_set () > 0
    || this->ex_handle_mask_ready_.num_set () > 0;

  return result;
}

inline int 
ACE_Reactor::schedule_wakeup (ACE_Event_Handler *eh,
			      ACE_Reactor_Mask mask)
{
  ACE_TRACE ("ACE_Reactor::schedule_wakeup");
  return this->mask_ops (eh->get_handle (), mask, ACE_Reactor::ADD_MASK);
}

inline int 
ACE_Reactor::cancel_wakeup (ACE_Event_Handler *eh,
			    ACE_Reactor_Mask mask)
{
  ACE_TRACE ("ACE_Reactor::cancel_wakeup");
  return this->mask_ops (eh->get_handle (), mask, ACE_Reactor::CLR_MASK);
}

inline int 
ACE_Reactor::schedule_wakeup (ACE_HANDLE handle,
			      ACE_Reactor_Mask mask)
{
  ACE_TRACE ("ACE_Reactor::schedule_wakeup");
  return this->mask_ops (handle, mask, ACE_Reactor::ADD_MASK);
}

inline int 
ACE_Reactor::cancel_wakeup (ACE_HANDLE handle,
			    ACE_Reactor_Mask mask)
{
  ACE_TRACE ("ACE_Reactor::cancel_wakeup");
  return this->mask_ops (handle, mask, ACE_Reactor::CLR_MASK);
}
