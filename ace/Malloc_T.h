/* -*- C++ -*- */
// $Id$


// ============================================================================
//
// = LIBRARY
//    ace
// 
// = FILENAME
//    Malloc_T.h
//
// = AUTHOR
//    Doug Schmidt and Irfan Pyarali
// 
// ============================================================================

#if !defined (ACE_MALLOC_T_H)
#define ACE_MALLOC_T_H

#include "ace/ACE.h"
#include "ace/Log_Msg.h"
#include "ace/Synch.h"
#include "ace/Malloc.h"

template <class MALLOC>
class ACE_Allocator_Adapter : public ACE_Allocator
  // = TITLE
  //    This class is an Adapter that allows the <ACE_Allocator> to
  //    use the <Malloc> class below.
{
public:
  // Trait.
  typedef MALLOC ALLOCATOR;
  typedef MALLOC::MEMORY_POOL_OPTIONS MEMORY_POOL_OPTIONS;

  // = Initialization.
  ACE_Allocator_Adapter (const char *pool_name = 0);
  // Constructor.
  ACE_Allocator_Adapter (const char *pool_name,
			 const char *lock_name);
  // Constructor.
  ACE_Allocator_Adapter (const MEMORY_POOL_OPTIONS &options,
			 const char *pool_name,
			 const char *lock_name)
    : allocator_ (options, pool_name, lock_name)
    { 
      ACE_TRACE ("ACE_Allocator_Adapter<MALLOC>::ACE_Allocator_Adapter");
    }

  // Constructor (must be here to avoid template bugs).

  ACE_Allocator_Adapter (const MEMORY_POOL_OPTIONS &options,
			 const char *pool_name = 0)
    : allocator_ (options, pool_name)
    { 
      ACE_TRACE ("ACE_Allocator_Adapter<MALLOC>::ACE_Allocator_Adapter");
    }

  // Constructor.

  // = Memory Management

  virtual void *malloc (size_t nbytes);
  // Allocate <nbytes>, but don't give them any initial value.

  virtual void *calloc (size_t nbytes, char initial_value = '\0');
  // Allocate <nbytes>, giving them all an <initial_value>.

  virtual void free (void *ptr);
  // Free <ptr> (must have been allocated by <ACE_Allocator::malloc>).

  virtual int remove (void);
  // Remove any resources associated with this memory manager.
  
  // = Map manager like functions

  int bind (const char *name, void *pointer, int duplicates = 0);
  // Associate <name> with <pointer>.  If <duplicates> == 0 then do
  // not allow duplicate <name>/<pointer> associations, else if
  // <duplicates> != 0 then allow duplicate <name>/<pointer>
  // assocations.  Returns 0 if successfully binds (1) a previously
  // unbound <name> or (2) <duplicates> != 0, returns 1 if trying to
  // bind a previously bound <name> and <duplicates> == 0, else
  // returns -1 if a resource failure occurs.  

  int trybind (const char *name, void *&pointer);
  // Associate <name> with <pointer>.  Does not allow duplicate
  // <name>/<pointer> associations.  Returns 0 if successfully binds
  // (1) a previously unbound <name>, 1 if trying to bind a previously
  // bound <name>, or returns -1 if a resource failure occurs.  When
  // this call returns <pointer>'s value will always reference the
  // void * that <name> is associated with.  Thus, if the caller needs
  // to use <pointer> (e.g., to free it) a copy must be maintained by
  // the caller.

  int find (const char *name, void *&pointer);
  // Locate <name> and pass out parameter via pointer.  If found,
  // return 0, Returns -1 if failure occurs.

  int find (const char *name);
  // returns 0 if the name is in the mapping. -1, otherwise.

  int unbind (const char *name);
  // Unbind (remove) the name from the map.  Don't return the pointer
  // to the caller

  int unbind (const char *name, void *&pointer);
  // Break any association of name.  Returns the value of pointer in
  // case the caller needs to deallocate memory.

  // = Protection and "sync" (i.e., flushing data to backing store).

  int sync (ssize_t len = -1, int flags = MS_SYNC);
  // Sync <len> bytes of the memory region to the backing store
  // starting at <this->base_addr_>.  If <len> == -1 then sync the
  // whole region.

  int sync (void *addr, size_t len, int flags = MS_SYNC);
  // Sync <len> bytes of the memory region to the backing store
  // starting at <addr_>.

  int protect (ssize_t len = -1, int prot = PROT_RDWR);
  // Change the protection of the pages of the mapped region to <prot>
  // starting at <this->base_addr_> up to <len> bytes.  If <len> == -1
  // then change protection of all pages in the mapped region.

  int protect (void *addr, size_t len, int prot = PROT_RDWR);
  // Change the protection of the pages of the mapped region to <prot>
  // starting at <addr> up to <len> bytes.

  ALLOCATOR &allocator (void);
  // Returns the underlying allocator.

  virtual void dump (void) const;
  // Dump the state of the object.

private:
  ALLOCATOR allocator_;
  // ALLOCATOR instance. 
};

// Forward declaration.
template <ACE_MEM_POOL_1, class LOCK>
class ACE_Malloc_Iterator;

template <ACE_MEM_POOL_1, class LOCK>
class ACE_Malloc
  // = TITLE
  //     Define a C++ class that uses parameterized types to provide
  //     an extensible mechanism for encapsulating various of dynamic
  //     memory management strategies.
  //
  // = DESCRIPTION
  //     This class can be configured flexibly with different
  //     MEMORY_POOL strategies and different types of LOCK
  //     strategies.
{
friend class ACE_Malloc_Iterator<ACE_MEM_POOL_2, LOCK>;
public:
  typedef ACE_MEM_POOL MEMORY_POOL;
  typedef ACE_MEM_POOL_OPTIONS MEMORY_POOL_OPTIONS;
  
  // = Initialization and termination methods.
  ACE_Malloc (const char *pool_name = 0);
  // Initialize ACE_Malloc.  This constructor passes <pool_name> to
  // initialize the memory pool, and uses <ACE::basename> to
  // automatically extract out the name used for the underlying lock
  // name (if necessary).

  ACE_Malloc (const char *pool_name,
	      const char *lock_name);
  // Initialize ACE_Malloc.  This constructor passes <pool_name> to
  // initialize the memory pool, and uses <lock_name> to automatically
  // extract out the name used for the underlying lock name (if
  // necessary).

  ACE_Malloc (const MEMORY_POOL_OPTIONS &options,
	      const char *pool_name,
	      const char *lock_name)
    : memory_pool_ (options, pool_name),
      lock_ (lock_name)
    {
      ACE_TRACE ("ACE_Malloc<ACE_MEM_POOL_2, LOCK>::ACE_Malloc");
      this->open ();
    }
  // Initialize ACE_Malloc.  This constructor passes <pool_name> to
  // initialize the memory pool, and uses <lock_name> to automatically
  // extract out the name used for the underlying lock name (if
  // necessary).  In addition, <options> is passed through to
  // initialize the underlying memory pool.

  ACE_Malloc (const MEMORY_POOL_OPTIONS &options,
	      const char *pool_name = 0)
    : memory_pool_ (options, pool_name),
      lock_ (pool_name == 0 ? 0 : ACE::basename (pool_name, 
						 ACE_DIRECTORY_SEPARATOR_CHAR))
    {
      ACE_TRACE ("ACE_Malloc<ACE_MEM_POOL_2, LOCK>::ACE_Malloc");
      this->open ();
    }

  // Initialize ACE_Malloc.  This constructor passes <pool_name> to
  // initialize the memory pool.  In addition, <options> is passed
  // through to initialize the underlying memory pool.

  int remove (void);
  // Releases resources allocated by ACE_Malloc. 

  // = Memory management
    
  void *malloc (size_t nbytes);
  // Allocate <nbytes>, but don't give them any initial value.

  void *calloc (size_t nbytes, char initial_value = '\0');
  // Allocate <nbytes>, giving them <initial_value>.
  
  void  free (void *ptr);
  // Deallocate memory pointed to by <ptr>, which must have been
  // allocated previously by <this->malloc>.

  MEMORY_POOL &memory_pool (void);
  // Returns a reference to the underlying memory pool.

  // = Map manager like functions

  int bind (const char *name, void *pointer, int duplicates = 0);
  // Associate <name> with <pointer>.  If <duplicates> == 0 then do
  // not allow duplicate <name>/<pointer> associations, else if
  // <duplicates> != 0 then allow duplicate <name>/<pointer>
  // assocations.  Returns 0 if successfully binds (1) a previously
  // unbound <name> or (2) <duplicates> != 0, returns 1 if trying to
  // bind a previously bound <name> and <duplicates> == 0, else
  // returns -1 if a resource failure occurs.  

  int trybind (const char *name, void *&pointer);
  // Associate <name> with <pointer>.  Does not allow duplicate
  // <name>/<pointer> associations.  Returns 0 if successfully binds
  // (1) a previously unbound <name>, 1 if trying to bind a previously
  // bound <name>, or returns -1 if a resource failure occurs.  When
  // this call returns <pointer>'s value will always reference the
  // void * that <name> is associated with.  Thus, if the caller needs
  // to use <pointer> (e.g., to free it) a copy must be maintained by
  // the caller.

  int find (const char *name, void *&pointer);
  // Locate <name> and pass out parameter via <pointer>.  If found,
  // return 0, returns -1 if failure occurs.

  int find (const char *name);
  // Returns 0 if <name> is in the mapping. -1, otherwise.

  int unbind (const char *name);
  // Unbind (remove) the name from the map.  Don't return the pointer
  // to the caller.  If you want to remove all occurrences of <name>
  // you'll need to call this method multiple times until it fails...

  int unbind (const char *name, void *&pointer);
  // Unbind (remove) one association of <name> to <pointer>.  Returns
  // the value of pointer in case the caller needs to deallocate
  // memory.  If you want to remove all occurrences of <name> you'll
  // need to call this method multiple times until it fails...

  // = Protection and "sync" (i.e., flushing data to backing store).

  int sync (ssize_t len = -1, int flags = MS_SYNC);
  // Sync <len> bytes of the memory region to the backing store
  // starting at <this->base_addr_>.  If <len> == -1 then sync the
  // whole region.

  int sync (void *addr, size_t len, int flags = MS_SYNC);
  // Sync <len> bytes of the memory region to the backing store
  // starting at <addr_>.

  int protect (ssize_t len = -1, int prot = PROT_RDWR);
  // Change the protection of the pages of the mapped region to <prot>
  // starting at <this->base_addr_> up to <len> bytes.  If <len> == -1
  // then change protection of all pages in the mapped region.

  int protect (void *addr, size_t len, int prot = PROT_RDWR);
  // Change the protection of the pages of the mapped region to <prot>
  // starting at <addr> up to <len> bytes.

#if defined (ACE_MALLOC_STATS)
  void print_stats (void);
  // Dump statistics of how malloc is behaving.
#endif /* ACE_MALLOC_STATS */  

  void dump (void) const;
  // Dump the state of an object.

  ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.

private:
  int open (void);
  // Initialize the Malloc pool.

  int shared_bind (const char *name, void *pointer);
  // Associate <name> with <pointer>.  Assumes that locks are held by
  // callers.

  ACE_Name_Node *shared_find (const char *name);
  // Try to locate <name>.  If found, return the associated
  // <ACE_Name_Node>, else returns 0 if can't find the <name>.
  // Assumes that locks are held by callers.

  void *shared_malloc (size_t nbytes);
  // Allocate memory.  Assumes that locks are held by callers.

  void shared_free (void *ptr);
  // Deallocate memory.  Assumes that locks are held by callers.

  ACE_Control_Block *cb_ptr_;
  // Pointer to the control block (stored in memory controlled by
  // MEMORY_POOL).

  MEMORY_POOL memory_pool_;
  // Pool of memory used by ACE_Malloc 

  LOCK lock_;
  // Local that ensures mutual exclusion.

};

template <ACE_MEM_POOL_1, class LOCK>
class ACE_Malloc_Iterator
  // = TITLE
  //     Iterator for names stored in Malloc'd memory.
  //
  // = DESCRIPTION
  //     Does not allows deletions while iteration is occurring.
{
public:
  // = Initialization method.
  ACE_Malloc_Iterator (ACE_Malloc<ACE_MEM_POOL_2, LOCK> &malloc, const char *name = 0);
  // if <name> = 0 it will iterate through everything else only
  // through those entries whose <name> match

  ~ACE_Malloc_Iterator (void);

  // = Iteration methods.

  int next (void *&next_entry);
  // Pass back the next <entry> in the set that hasn't yet been
  // visited.  Returns 0 when all items have been seen, else 1.

  int next (void *&next_entry, char *&name);
  // Pass back the next <entry> (and the <name> associated with it) in
  // the set that hasn't yet been visited.  Returns 0 when all items
  // have been seen, else 1.

  int advance (void);
  // Move forward by one element in the set.

  void dump (void) const;
  // Dump the state of an object.

  ACE_ALLOC_HOOK_DECLARE;
  // Declare the dynamic allocation hooks.

private:
  ACE_Malloc<ACE_MEM_POOL_2, LOCK> &malloc_;
  // Malloc we are iterating over.

  ACE_Name_Node *curr_;
  // Keeps track of how far we've advanced...

  ACE_Read_Guard<LOCK> guard_;
  // Lock Malloc for the lifetime of the iterator.

  const char *name_;
  // Name that we are searching for.
};

#if defined (__ACE_INLINE__)
#include "ace/Malloc_T.i"
#endif /* __ACE_INLINE__ */

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
#include "ace/Malloc_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
#pragma implementation ("Malloc_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif /* ACE_MALLOC_H */
