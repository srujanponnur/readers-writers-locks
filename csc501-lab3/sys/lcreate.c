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
	int free_lock, proc_index;
	free_lock = newlockid();
	if (free_lock == SYSERR) {
		restore(ps);
		return SYSERR;
	}
	locks[free_lock].lstatus = LINIT;
	for (proc_index = 0; proc_index < NPROC; proc_index++) {
		locks[free_lock].proc_list[proc_index] = 0;
	}
	locks[free_lock].is_writer = 0;
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
		if (locks[lock].lstatus == LFREE)
			return(lock);
	}
	return(SYSERR);
}