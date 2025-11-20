/*--------------------------------------------------------------------
 * Symbols referenced in this file:
 * - MyProc
 *--------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 *
 * proc.c
 *	  routines to manage per-process shared memory data structure
 *
 * Portions Copyright (c) 1996-2022, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/storage/lmgr/proc.c
 *
 *-------------------------------------------------------------------------
 */
/*
 * Interface (a):
 *		ProcSleep(), ProcWakeup(),
 *		ProcQueueAlloc() -- create a shm queue for sleeping processes
 *		ProcQueueInit() -- create a queue without allocing memory
 *
 * Waiting for a lock causes the backend to be put to sleep.  Whoever releases
 * the lock wakes the process up again (and gives it an error code so it knows
 * whether it was awoken on an error condition).
 *
 * Interface (b):
 *
 * ProcReleaseLocks -- frees the locks associated with current transaction
 *
 * ProcKill -- destroys the shared memory state (and locks)
 * associated with the process.
 */
#include "postgres.h"

#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include "access/transam.h"
#include "access/twophase.h"
#include "access/xlogutils.h"
#include "miscadmin.h"
#include "pgstat.h"
#include "postmaster/autovacuum.h"
#include "replication/slot.h"
#include "replication/syncrep.h"
#include "replication/walsender.h"
#include "storage/condition_variable.h"
#include "storage/ipc.h"
#include "storage/lmgr.h"
#include "storage/pmsignal.h"
#include "storage/proc.h"
#include "storage/procarray.h"
#include "storage/procsignal.h"
#include "storage/spin.h"
#include "storage/standby.h"
#include "utils/timeout.h"
#include "utils/timestamp.h"

/* GUC variables */







/* Pointer to this process's PGPROC struct, if any */
__thread PGPROC	   *MyProc = NULL;







/*
 * This spinlock protects the freelist of recycled PGPROC structures.
 * We cannot use an LWLock because the LWLock manager depends on already
 * having a PGPROC and a wait semaphore!  But these structures are touched
 * relatively infrequently (only at backend startup or shutdown) and not for
 * very long, so a spinlock is okay.
 */


/* Pointers to shared-memory structures */






/* If we are waiting for a lock, this points to the associated LOCALLOCK */




/* Is a deadlock check pending? */


static void RemoveProcFromArray(int code, Datum arg);
static void ProcKill(int code, Datum arg);
static void AuxiliaryProcKill(int code, Datum arg);
static void CheckDeadLock(void);

/*
 * Report shared-memory space needed by InitProcGlobal.
 */


/*
 * Report number of semaphores needed by InitProcGlobal.
 */


/*
 * InitProcGlobal -
 *	  Initialize the global process table during postmaster or standalone
 *	  backend startup.
 *
 *	  We also create all the per-process semaphores we will need to support
 *	  the requested number of backends.  We used to allocate semaphores
 *	  only when backends were actually started up, but that is bad because
 *	  it lets Postgres fail under load --- a lot of Unix systems are
 *	  (mis)configured with small limits on the number of semaphores, and
 *	  running out when trying to start another backend is a common failure.
 *	  So, now we grab enough semaphores to support the desired max number
 *	  of backends immediately at initialization --- if the sysadmin has set
 *	  MaxConnections, max_worker_processes, max_wal_senders, or
 *	  autovacuum_max_workers higher than his kernel will support, he'll
 *	  find out sooner rather than later.
 *
 *	  Another reason for creating semaphores here is that the semaphore
 *	  implementation typically requires us to create semaphores in the
 *	  postmaster, not in backends.
 *
 * Note: this is NOT called by individual backends under a postmaster,
 * not even in the EXEC_BACKEND case.  The ProcGlobal and AuxiliaryProcs
 * pointers must be propagated specially for EXEC_BACKEND operation.
 */


/*
 * InitProcess -- initialize a per-process data structure for this backend
 */
#ifdef USE_ASSERT_CHECKING
#endif

/*
 * InitProcessPhase2 -- make MyProc visible in the shared ProcArray.
 *
 * This is separate from InitProcess because we can't acquire LWLocks until
 * we've created a PGPROC, but in the EXEC_BACKEND case ProcArrayAdd won't
 * work until after we've done CreateSharedMemoryAndSemaphores.
 */


/*
 * InitAuxiliaryProcess -- create a per-auxiliary-process data structure
 *
 * This is called by bgwriter and similar processes so that they will have a
 * MyProc value that's real enough to let them wait for LWLocks.  The PGPROC
 * and sema that are assigned are one of the extra ones created during
 * InitProcGlobal.
 *
 * Auxiliary processes are presently not expected to wait for real (lockmgr)
 * locks, so we need not set up the deadlock checker.  They are never added
 * to the ProcArray or the sinval messaging mechanism, either.  They also
 * don't get a VXID assigned, since this is only useful when we actually
 * hold lockmgr locks.
 *
 * Startup process however uses locks but never waits for them in the
 * normal backend sense. Startup process also takes part in sinval messaging
 * as a sendOnly process, so never reads messages from sinval queue. So
 * Startup process does have a VXID and does show up in pg_locks.
 */
#ifdef USE_ASSERT_CHECKING
#endif

/*
 * Used from bufmgr to share the value of the buffer that Startup waits on,
 * or to reset the value to "not waiting" (-1). This allows processing
 * of recovery conflicts for buffer pins. Set is made before backends look
 * at this value, so locking not required, especially since the set is
 * an atomic integer set operation.
 */


/*
 * Used by backends when they receive a request to check for buffer pin waits.
 */


/*
 * Check whether there are at least N free PGPROC objects.
 *
 * Note: this is designed on the assumption that N will generally be small.
 */


/*
 * Check if the current process is awaiting a lock.
 */


/*
 * Cancel any pending wait for lock, when aborting a transaction, and revert
 * any strong lock count acquisition for a lock being acquired.
 *
 * (Normally, this would only happen if we accept a cancel/die
 * interrupt while waiting; but an ereport(ERROR) before or during the lock
 * wait is within the realm of possibility, too.)
 */



/*
 * ProcReleaseLocks() -- release locks associated with current transaction
 *			at main transaction commit or abort
 *
 * At main transaction commit, we release standard locks except session locks.
 * At main transaction abort, we release all locks including session locks.
 *
 * Advisory locks are released only if they are transaction-level;
 * session-level holds remain, whether this is a commit or not.
 *
 * At subtransaction commit, we don't release any locks (so this func is not
 * needed at all); we will defer the releasing to the parent transaction.
 * At subtransaction abort, we release all locks held by the subtransaction;
 * this is implemented by retail releasing of the locks under control of
 * the ResourceOwner mechanism.
 */



/*
 * RemoveProcFromArray() -- Remove this process from the shared ProcArray.
 */


/*
 * ProcKill() -- Destroy the per-proc data structure for
 *		this process. Release any of its held LW locks.
 *
 * If you are going to edit this, take a look at postmaster.c:reaper as well.
 * That function handles as much as this as possible but from the perspective
 * of the parent of the terminated child, to handle cases where the child was
 * not able to clean itself up.
 */
#ifdef USE_ASSERT_CHECKING
#endif

/*
 * Detach from any lock group of which we are a member.  If the leader
 * exist before all other group members, its PGPROC will remain allocated
 * until the last group process exits; that process must return the
 * leader's PGPROC to the appropriate list.
 */




/*
 * AuxiliaryProcKill() -- Cut-down version of ProcKill for auxiliary
 *		processes (bgwriter, etc).  The PGPROC and sema are not released, only
 *		marked as not-in-use.
 */


/*
 * AuxiliaryPidGetProc -- get PGPROC for an auxiliary process
 * given its PID
 *
 * Returns NULL if not found.
 */


/*
 * ProcQueue package: routines for putting processes to sleep
 *		and  waking them up
 */

/*
 * ProcQueueAlloc -- alloc/attach to a shared memory process queue
 *
 * Returns: a pointer to the queue
 * Side Effects: Initializes the queue if it wasn't there before
 */
#ifdef NOT_USED
PROC_QUEUE *
ProcQueueAlloc(const char *name)
{
	PROC_QUEUE *queue;
	bool		found;

	queue = (PROC_QUEUE *)
		ShmemInitStruct(name, sizeof(PROC_QUEUE), &found);

	if (!found)
		ProcQueueInit(queue);

	return queue;
}
#endif

/*
 * ProcQueueInit -- initialize a shared memory process queue
 */



/*
 * ProcSleep -- put a process to sleep on the specified lock
 *
 * Caller must have set MyProc->heldLocks to reflect locks already held
 * on the lockable object by this process (under all XIDs).
 *
 * The lock table's partition lock must be held at entry, and will be held
 * at exit.
 *
 * Result: PROC_WAIT_STATUS_OK if we acquired the lock, PROC_WAIT_STATUS_ERROR if not (deadlock).
 *
 * ASSUME: that no one will fiddle with the queue until after
 *		we release the partition lock.
 *
 * NOTES: The process queue is now a priority queue for locking.
 */



/*
 * ProcWakeup -- wake up a process by setting its latch.
 *
 *	 Also remove the process from the wait queue and set its links invalid.
 *	 RETURN: the next process in the wait queue.
 *
 * The appropriate lock partition lock must be held by caller.
 *
 * XXX: presently, this code is only used for the "success" case, and only
 * works correctly for that case.  To clean up in failure case, would need
 * to twiddle the lock's request counts too --- see RemoveFromWaitQueue.
 * Hence, in practice the waitStatus parameter must be PROC_WAIT_STATUS_OK.
 */


/*
 * ProcLockWakeup -- routine for waking up processes when a lock is
 *		released (or a prior waiter is aborted).  Scan all waiters
 *		for lock, waken any that are no longer blocked.
 *
 * The appropriate lock partition lock must be held by caller.
 */


/*
 * CheckDeadLock
 *
 * We only get to this routine, if DEADLOCK_TIMEOUT fired while waiting for a
 * lock to be released by some other process.  Check if there's a deadlock; if
 * not, just return.  (But signal ProcSleep to log a message, if
 * log_lock_waits is true.)  If we have a real deadlock, remove ourselves from
 * the lock's wait queue and signal an error to ProcSleep.
 */
#ifdef LOCK_DEBUG
#endif

/*
 * CheckDeadLockAlert - Handle the expiry of deadlock_timeout.
 *
 * NB: Runs inside a signal handler, be careful.
 */


/*
 * ProcWaitForSignal - wait for a signal from another backend.
 *
 * As this uses the generic process latch the caller has to be robust against
 * unrelated wakeups: Always check that the desired state has occurred, and
 * wait again if not.
 */


/*
 * ProcSendSignal - set the latch of a backend identified by pgprocno
 */


/*
 * BecomeLockGroupLeader - designate process as lock group leader
 *
 * Once this function has returned, other processes can join the lock group
 * by calling BecomeLockGroupMember.
 */


/*
 * BecomeLockGroupMember - designate process as lock group member
 *
 * This is pretty straightforward except for the possibility that the leader
 * whose group we're trying to join might exit before we manage to do so;
 * and the PGPROC might get recycled for an unrelated process.  To avoid
 * that, we require the caller to pass the PID of the intended PGPROC as
 * an interlock.  Returns true if we successfully join the intended lock
 * group, and false if not.
 */

