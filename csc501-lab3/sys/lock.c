#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock_q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>


int lock(int lockdescriptor, int type, int priority) {

	STATWORD ps;
	disable(ps);

	if (isbadlock(lockdescriptor) || locks[lockdescriptor].lstatus == LFREE) { // either bad lock descriptor or the lock has not been created yet.
		restore(ps);
		return SYSERR;
	}
	else if (type != READ || type != WRITE) {
		restore(ps);
		return SYSERR
	}

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
			make_process_wait(currpid, lockdescriptor);
			resched();
		}
		else {
			if ((locks[lockdescriptor].ltype == WRITE || locks[lockdescriptor].is_writer) || is_writer_waiting(lockdescriptor, priority)) { //lock is currently acquired by writer process
				make_process_wait(currpid, lockdescriptor);
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

void make_process_wait(int pid, int lockdescriptor) {
	struct pentry* ptr;
	ptr = &proctab[pid];
	ptr->pstate = PRWAIT;
	ptr->plock = lockdescriptor;
	return;
}
