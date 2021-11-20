#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <q.h>

int ldelete(int lockdescriptor) {
	STATWORD ps;
	disable(ps);
	int proc_index;
	if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstatus == LFREE) { // either bad lock descriptor or the lock has not been created yet.
		restore(ps);
		return SYSERR;
	}

	struct pentry* pptr;
	struct lentry* lock_ptr = &locks[lockdescriptor];

	lock_ptr->lstatus = LFREE;
	lock_ptr->is_deleted = 1; 
	

	for (proc_index = 0; proc_index < NPROC; proc_index++) { // clearing the processes 
		if (lock_ptr->proc_list[proc_index]) {
			lock_ptr->proc_list[proc_index] = 0;
			pptr = &proctab[proc_index];
			pptr->plused[lockdescriptor] = PLDEL; /* the lock being used got deleted */
		}
	}

	if (nonempty(lock_ptr->lqhead)) {
		int pid = getfirst(lock_ptr->lqhead);
		while (pid != EMPTY)
		{
			proctab[pid].pwaitret = DELETED;
			ready(pid, RESCHNO);
			pid = getfirst_l(lock_ptr->lqhead);
		}
		resched();
	}

	restore(ps);
	return OK;
}