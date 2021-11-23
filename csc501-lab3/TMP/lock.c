#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <q.h>

int lock(int lockdescriptor, int type, int priority) {

	STATWORD ps;
	disable(ps);
	
	if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstatus == LFREE || (type != READ && type != WRITE)) { // either bad lock descriptor or the lock has not been created yet.
		//kprintf("\nComing inside error\n");
		restore(ps);
		return SYSERR;
	}

	struct pentry* ptr;
	ptr = &proctab[currpid];
	int is_deleted = locks[lockdescriptor].is_deleted;
	
	if((ptr->plused[lockdescriptor] == PLUSED || ptr->plused[lockdescriptor] == PLDEL) && is_deleted)  { // this lock was recreated but old lock was used by this process
		kprintf("Reaching the syncronous delete case\n");
		restore(ps);
		return SYSERR;
	} 
	
	ptr->plused[lockdescriptor] = PLUSED;

	if (locks[lockdescriptor].lstatus == LINIT) { // lock has been created but not used
		//kprintf("\nComing inside linit\n");
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
		//kprintf("\nComing inside lused\n");
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
	int last = q[qtail].qprev;
	int qhead = locks[lockdescriptor].lqhead;
	int should_wait = 0;
	while (last != qhead) {
		if (q[last].qtype == WRITE && q[last].qkey > priority) {
			should_wait = 1;
			break;
		}
		last = q[last].qprev;
	}
	//kprintf("waiting flag is: %d\n", should_wait);
	return should_wait;
}

void set_priority_inheritance(int pid) {
	struct pentry* pptr, *wpptr;
	struct lentry* lptr;
	//kprintf("Coming to set_inh func: %d\n", pid);
	int prio,l_index,proc_index,curr,max_prio = -1000;
	pptr = &proctab[pid];
	for (l_index = 0; l_index < NLOCKS; l_index++) {
		lptr = &locks[l_index];
		if (lptr->proc_list[pid]) {
			//kprintf("Process: %d has lock: %d with lprio: %d\n", pid, l_index, lptr->lprio);
			curr = lptr->lprio;
			if (curr > max_prio) {
				max_prio = curr;
				//kprintf("Reaching here with maxprio: %d\n", max_prio);
			}
		}
	}

	//kprintf("The maximum priority is: %d", max_prio);

	if (max_prio == NULL) {
		pptr->pinh = 0;
	}
	else {
		pptr->pinh = max_prio;
	}

	if (pptr->plock != -1) {
		lptr = &locks[pptr->plock];
		prio = pptr->pinh != 0 ? pptr->pinh : pptr->pprio;
		lptr->lprio = (lptr->lprio > prio) ? lptr->lprio : prio; // since process's priority is being increased, change the max priority in queue
		for (proc_index = 0; proc_index < NPROC; proc_index++) {
			if (lptr->proc_list[proc_index]) {
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
			//kprintf("Calling set_inh func for process: %d\n", proc_index);
			ptr = &proctab[proc_index];
			set_priority_inheritance(proc_index);
		}
	}
	return;
}
