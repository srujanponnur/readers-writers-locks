
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <lock_q.h>

int releaseall(int numlocks, int ldesc1) { // variable argument validation with numlocks is pending!

	STATWORD ps;
	disable(ps);

	int l_index, reader, writer, *lock, ltype, temp, waitprio, q_head, r_waittime, w_waittime;
	int pid = currpid;
	struct lentry* lptr;
	lock = &ldesc1;

	for (l_index = 0; l_index < numlocks; l_index++) {

		if (isbadlock(*lock) || !check_process_locks(*lock, currpid)) {
			restore(ps);
			return SYSERR;
		}

		
		lptr = &locks[*lock];
		lptr->proc_list[currpid] = 0;
		ltype = lptr->ltype;
		q_head = lptr->lqhead;

		if (ltype == READ) {
			lptr->reader_count--;
		}
		else {
			lptr->is_writer = 0;
		}

		if ((type == WRITE && lptr->is_writer == 0) || (type == READ && lptr->reader_count == 0)) {
			
			int result;
			get_last_in_queue(*lock, &result);
			if (result != -1) { // if -1 no entries in the lock's queue, moving on.
				if (q_l[result].qtype == WRITE) {
					acquire_lock(result, *lock, WRITE);
				}
				else if (q_l[result].qtype == READ) {
					get_type(*lock, &writer, WRITE);
					if (writer != -1) { // there is a writer existing in the queue
						if (q_l[result].qkey == q_l[writer].qkey) { /* writer and reader has same priority*/
							r_waittime = ctr1000 - q_l[result].qkey;
							w_waittime = ctr1000 - q_l[writer].qkey;
							if ((w_waittime - r_waittime) < 1000) {
								// a writer has arrived within a second, writer acquring lock
								acquire_lock(writer, *lock, WRITE);
							}
							else { // wait time is more than a second, select all the readers that arrived more than 1 second with same wait priority
								temp = result;
								waitprio = q_l[result].qkey;
								while (q_l[temp].qkey == waitprio && temp!= q_head) {
									r_waittime = ctr1000 - q_l[temp].qkey;
									w_waittime = ctr1000 - q_l[writer].qkey;
									if ((w_waittime - r_waittime) > 1000) {
										acquire_lock(temp, *lock, READ);
									}
									temp = q_l[temp].qprev;
								}
							}
						}
						else { /* reader has more wait priority , read acquires the lock*/
							waitprio = q_l[writer].qkey;
							temp = result;
							while (temp != writer && q_l[temp].qkey > waitprio && temp!=q_head) { /* all readers with wait priority greater than the writer */
								acquire_lock(temp, *lock, READ);
								temp = q_l[temp].qprev;
							}
						}
					}
					else { /* no writers in queue, all readers in queue acquire the lock*/
						temp = result;
						while (temp != q_head) {
							acquire_lock(temp, *lock, READ);
							temp = q_l[temp].qprev;
						}
					}
				}
			}

		}

		lock++;
	}

	return OK;
}

void acquire_lock(int result, int lockdescriptor, int type) {
	struct lentry* lptr;
	struct pentry* pptr;
	pptr = &proctab[result];
	lptr = &locks[lockdescriptor];
	lptr->ltype = type;
	lptr->proc_list[result] = 1;
	pptr->plused[lockdescriptor] = 1;
	pptr->plock = -1;
	if (type == READ) {
		lptr->reader_count++;
	}
	else {
		lptr->is_writer = 1;
	}
	dequeue_l(result);
	ready(result, RESCHNO);
	return;
}

void get_last_in_queue(int lockdescriptor, int *result) {

	STATWORD ps;
	disable(ps);
	struct lentry* lptr;
	int last, head;
	*result = -1;
	lptr = &locks[lockdescriptor];
	head = lptr->lqhead;
	if (nonempty_l(head)) {
		last = q_l[lptr->lqtail].qprev;
		*result = last;
	}
	restore(ps);
	return;
}

void get_type(int lockdescriptor, int *result, int type) {
	STATWORD ps;
	disable(ps);

	struct lentry* lptr;
	int last, head;
	*result = -1;
	if (type != READ || type != WRITE)  {
		restore(ps);
		return;
	}
	lptr = &locks[lockdescriptor];
	head = lptr->lqhead;
	if (nonempty_l(head)) {
		last = q_l[lptr->lqtail].qprev;
		while (last != head) {
			if (q_l[last].qtype == type) {
				*result = last;
				break;
			}
			last = q_l[last].qprev;
		}
	}
	restore(ps);
	return;
}

int check_process_locks(int lockdescriptor, int pid) {

	int present = 0, proc_index;
	struct lentry* lptr;
	lptr = &locks[lockdescriptor];

	for (proc_index = 0; proc_index < NPROC; proc_index++) {
		if (lptr->proc_list[proc_index] == 1 && proc_index == pid) {
			present = 1;
			break;
		}
	}
	return present;
}