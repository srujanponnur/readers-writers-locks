#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock_q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

void linit() {
	int lock_index, proc_index;
	for (lock_index = 0; lock_index < NLOCKS; lock_index++) {
		locks[lock_index].lstatus = LFREE;
		locks[lock_index].ltype = -1;
		locks[lock_index].lqtail = 1 + (locks[lock_index].lqhead = newqueue_l());
		for (proc_index = 0; proc_index < NPROC; proc_index++) {
			locks[lock_index].proc_list[proc_index] = 0;
		}
		locks[lock_index].reader_count = 0;
		locks[lock_index].is_writer = 0;
		locks[lock_index].is_deleted = 0; /* was this lock ever got deleted?*/
	}
	return;
}