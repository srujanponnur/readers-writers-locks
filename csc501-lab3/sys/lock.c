#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
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
		if (type == WRITE) {
			locks[lockdescriptor].is_writer = 1;
		}
		else { // it is read type
			locks[lockdescriptor].reader_count++;
		}
		locks[lockdescriptor].proc_list[currpid] = 1;
	}

    else if (locks[lockdescriptor].lstatus == LUSED) { // either read or write lock has been acquired

	}
	

	restore(ps);
	return OK;
}

int releaseall(int numlocks) {
	return OK;
}