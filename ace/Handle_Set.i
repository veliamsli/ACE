/* -*- C++ -*- */
// $Id$

// Handle_Set.i

// Initialize the bitmask to all 0s and reset the associated fields.

ACE_INLINE void 
ACE_Handle_Set::reset (void)
{
  ACE_TRACE ("ACE_Handle_Set::reset");
  this->max_handle_ = ACE_INVALID_HANDLE;
  this->size_ = 0;
  FD_ZERO (&this->mask_);
}

// Returns the number of the large bit.

ACE_INLINE ACE_HANDLE  
ACE_Handle_Set::max_set (void) const
{
  ACE_TRACE ("ACE_Handle_Set::max_set");
  return this->max_handle_;
}

// Checks whether handle is enabled.

ACE_INLINE int
ACE_Handle_Set::is_set (ACE_HANDLE handle) const
{
  ACE_TRACE ("ACE_Handle_Set::is_set");
  return FD_ISSET (handle, &this->mask_);
}

// Enables the handle.

ACE_INLINE void 
ACE_Handle_Set::set_bit (ACE_HANDLE handle)
{
  ACE_TRACE ("ACE_Handle_Set::set_bit");
  if (!this->is_set (handle))
    {
#if defined(ACE_WIN32)
      FD_SET ((SOCKET)handle, &this->mask_);
#else /* !ACE_WIN32 */
      FD_SET (handle, &this->mask_);
      this->size_++;
      if (handle > this->max_handle_)
	this->max_handle_ = handle;
#endif /* ACE_WIN32 */
    }
}

// Disables the handle.

ACE_INLINE void 
ACE_Handle_Set::clr_bit (ACE_HANDLE handle)
{
  ACE_TRACE ("ACE_Handle_Set::clr_bit");
  if (this->is_set (handle))
    {
#if defined(ACE_WIN32)
      FD_CLR ((SOCKET)handle, &this->mask_);
#else /* !ACE_WIN32 */
      FD_CLR (handle, &this->mask_);
      this->size_--;

      if (handle == this->max_handle_)
	this->set_max (this->max_handle_);
#endif /* ACE_WIN32 */
    }
}

// Returns a count of the number of enabled bits.

ACE_INLINE int 
ACE_Handle_Set::num_set (void) const
{
  ACE_TRACE ("ACE_Handle_Set::num_set");
#if defined(ACE_WIN32)
  return this->mask_.fd_count;
#else /* !ACE_WIN32 */
  return this->size_;
#endif /* ACE_WIN32 */
}

// Returns a pointer to the underlying fd_set.

ACE_INLINE
ACE_Handle_Set::operator fd_set *()
{
  ACE_TRACE ("ACE_Handle_Set::operator ACE_FD_SET_TYPE *");
  return (fd_set *) &this->mask_;
}

ACE_INLINE ACE_HANDLE
ACE_Handle_Set_Iterator::operator () (void)
{
  ACE_TRACE ("ACE_Handle_Set_Iterator::operator");
#if defined(ACE_WIN32)
  return this->index_ < this->handles_.mask_.fd_count
    ? (ACE_HANDLE)this->handles_.mask_.fd_array[this->index_]
    : ACE_INVALID_HANDLE;
#else /* !ACE_WIN32 */
  return this->num_ <= this->handles_.max_handle_ ? this->num_ : -1;
#endif /* ACE_WIN32 */
}
