#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <lock_q.h>

int lock(int lockdescriptor, int type, int priority) {

	STATWORD ps;
	disable(ps);
	
	if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstatus == LFREE || (type != READ || type != WRITE)) { // either bad lock descriptor or the lock has not been created yet.
		restore(ps);
		return SYSERR;
	}

	struct pentry* ptr;
	ptr = &proctab[currpid];
	int is_deleted = locks[lockdescriptor].is_deleted;
	
	if((ptr->plused[lockdescriptor] == PLUSED || ptr->plused[lockdescriptor] == PLDEL) && is_deleted)  { // this lock was recreated but old lock was used by this process
		restore(ps);
		return SYSERR
	} 
	
	ptr->plused[lockdescriptor] = PLUSED;

	if (locks[lockdescriptor].lstatus == LINIT) { // lock has been created but not used

		locks[lockdescriptor].lstatus = LUSED;
		locks[lockdescriptor].ltype = type;
		if (type == WRITE) {
			locks[lockdescriptor].is_writer = 1;
		}
		else { // it is read type
			locks[lockdescriptor].reader_count++;
		}
		locks[lockdescriptor].proc_list[currpid] = 1;
	}

    else if (locks[lockdescriptor].lstatus == LUSED) { // either read or write lock has been acquired

		if (type == WRITE) {
			make_process_wait(currpid, lockdescriptor, WRITE, priority);
			resched();
		}
		else {
			if ((locks[lockdescriptor].ltype == WRITE || locks[lockdescriptor].is_writer) || is_writer_waiting(lockdescriptor, priority)) { //lock is currently acquired by writer process
				make_process_wait(currpid, lockdescriptor, READ, priority);
				resched();
			}
			else { // read lock can be acquired
				locks[lockdescriptor].reader_count++;
				locks[lockdescriptor].proc_list[currpid] = 1;
			}

		}

	}
	restore(ps);
	return OK;
}

int is_writer_waiting(int lockdescriptor, int priority) {
	int qtail = locks[lockdescriptor].lqtail;
	int last = q_l[qtail].qprev;
	int qhead = locks[lockdescriptor].lqhead;
	int should_wait = false;
	while (last != qhead) {
		if (q_l[last].qtype == WRITE && q_l[last].qkey > priority) {
			should_wait = true;
			break;
		}
		last = q_l[last].qprev;
	}
	return shoud_wait;
}

void set_priority_inheritance(int pid) {
	struct pentry* pptr, *wpptr;
	struct lentry* lptr;
	int prio,inh_prio,l_index,proc_index,curr,max_prio,l_index;
	pptr = &proctab[pid];
	for (l_index = 0; l_index < NLOCKS; l_index++) {
		lptr = &locks[l_index];
		if (lptr->proc_list[pid]) {
			curr = lptr->lprio;
			if (max_prio == NULL || curr > max) {
				max_prio = curr;
			}
		}
	}

	if (max == NULL) {
		pptr->pinh = 0
	}
	else {
		pptr->pinh = max_prio;
	}

	if (pptr->plock != -1) {
		lptr = &locks[pptr->plock];
		prio = ptr->pinh != 0 ? ptr->pinh : ptr->pprio;
		lptr->lprio = (lptr->lprio > prio) ? lptr->lprio : prio; // since process's priority is being increased, change the max priority in queue
		for (proc_index = 0; proc_index < NPROC; proc_index++) {
			if (lptr->proc_list[proc_index]) {
				wpptr = &proctab[proc_index];
				inh_prio = wpptr->pinh != 0 ? wpptr->pinh : wpptr->pprio;
				set_priority_inheritance(proc_index);
			}
		}
	}
	return;
}


void make_process_wait(int pid, int lockdescriptor, int type, int priority) {
	struct pentry* ptr;
	struct lentry* lptr;
	int proc_index;
	ptr = &proctab[pid];
	int prio = ptr->pinh != 0 ? ptr->pinh : ptr->pprio;
	lptr = &locks[lockdescriptor];
	ptr->pstate = PRWAIT;
	ptr->plock = lockdescriptor;
	insert_lq(pid, lptr->lqhead, priority, type);
	if (prio > lptr->lprio) {
		lptr->lprio = prio;
	}

	for (proc_index = 0; proc_index < NPROC; proc_index++) {
		if (lptr->proc_list[proc_index]) {
			ptr = &proctab[proc_index];
			set_priority_inheritance(proc_index);
		}
	}
	return;
}
