/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	struct lentry* lptr;
	int proc_index;
	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	pptr->pprio = newprio;
	pptr->pinh = (pptr->pinh == 0) ? 0 : newprio;
	if (pptr->plock != -1) { // process is waiting for a lock
		lptr = &locks[pptr->plock];
		lptr->lprio = get_max_in_queue(pptr->plock);
		for (proc_index = 0; proc_index < NPROC; proc_index++) {
			if (lptr->proc_list[proc_index]) {
				set_priority_inheritance(proc_index);
			}
		}
	}
	restore(ps);
	return(newprio);
}
