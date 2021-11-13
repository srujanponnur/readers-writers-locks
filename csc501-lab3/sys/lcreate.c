#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

int lcreate() {

	STATWORD ps;
	disable(ps);
	int free_lock;
	free_lock = newlockid();
	if (free_lock == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	locks[free_lock] = LINIT;
	for (proc_index = 0; proc_index < NPROC; prox_index++) {
		locks[lock_index].proc_list[proc_index] = 0;
	}
	locks[lock_index].is_writer = 0;
	locks[free_lock].reader_count = 0;
	restore(ps);
	return free_lock;
}


int newlockid() {
	int	lock_index, lock;
	for (lock_index = 0; lock_index < NLOCKS; lock_index++) {
		lock = nextlock--;
		if (lock <= 0) {
			lock = NLOCKS - 1;
		}
		if (locks[lock].status == LFREE)
			return(lock);
	}
	return(SYSERR);
}