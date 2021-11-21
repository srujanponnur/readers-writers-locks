
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
#include <q.h>

int releaseall(int numlocks, int ldesc1) { // variable argument validation with numlocks is pending!

	STATWORD ps;
	disable(ps);

	int l_index, reader, writer, *lock, ltype, temp, waitprio, q_head, r_waittime, w_waittime, entry_count, q_index;
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

		if ((ltype == WRITE && lptr->is_writer == 0) || (ltype == READ && lptr->reader_count == 0)) {
			
			int result;
			kprintf("Coming inside reacquisition\n");
			get_last_in_queue(*lock, &result);
			if (result != -1) { // if -1 no entries in the lock's queue, moving on.
				if (q[result].qtype == WRITE) {
					kprintf("Acquiring write lock...\n");
					acquire_lock(result, *lock, WRITE);
				}
				else if (q[result].qtype == READ) {
					kprintf("Acquiring read lock...\n");
					get_type(*lock, &writer, WRITE);
					kprintf("In read, does writer exist: %d\n", writer);
					if (writer != -1) { // there is a writer existing in the queue
						if (q[result].qkey == q[writer].qkey) { /* writer and reader has same priority*/
							r_waittime = ctr1000 - q[result].qkey;
							w_waittime = ctr1000 - q[writer].qkey;
							if ((w_waittime - r_waittime) < 1000) {
								// a writer has arrived within a second, writer acquring lock
								acquire_lock(writer, *lock, WRITE);
							}
							else { // wait time is more than a second, select all the readers that arrived more than 1 second with same wait priority
								temp = result;
								waitprio = q[result].qkey;
								while (q[temp].qkey == waitprio && temp!= q_head) {
									r_waittime = ctr1000 - q[temp].qkey;
									w_waittime = ctr1000 - q[writer].qkey;
									if ((w_waittime - r_waittime) > 1000) {
										acquire_lock(temp, *lock, READ);
									}
									temp = q[temp].qprev;
								}
							}
						}
						else { /* reader has more wait priority , read acquires the lock*/
							waitprio = q[writer].qkey;
							temp = result;
							while (temp != writer && q[temp].qkey > waitprio && temp!=q_head) { /* all readers with wait priority greater than the writer */
								acquire_lock(temp, *lock, READ);
								temp = q[temp].qprev;
							}
						}
					}
					else { /* no writers in queue, all readers in queue can acquire the lock*/
						kprintf("No writer in queue, just acquiring this\n");
						print_queue(*lock);
						entry_count = get_queue_count(*lock);
						for (q_index = 0; q_index < entry_count; q_index++) {
							get_last_in_queue(*lock,&temp);
							acquire_lock(temp, *lock, READ);
						}
					}
				}
			}

		}

		lock++;
	}
	reset_inherited_priority(pid);
	restore(ps);
	return OK;
}

void acquire_lock(int result, int lockdescriptor, int type) {
	struct lentry* lptr;
	struct pentry* pptr;
	kprintf("Coming to acquire lock: %d\n", lockdescriptor);
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
	dequeue(result);
	lptr->lprio = get_max_in_queue(lockdescriptor);
	ready(result, RESCHNO);
	return;
}

void reset_inherited_priority(int pid) {
	STATWORD ps;
	disable(ps);
	struct pentry* pptr;
	struct lentry* lptr;
	int l_index, proc_index, curr, max, tail, head, last;
	max = NULL;
	pptr = &proctab[pid];
	for (l_index; l_index < NLOCKS; l_index++) {
		lptr = &locks[l_index];
		if (lptr->proc_list[pid]) {
			tail = lptr->lqtail;
			head = lptr->lqhead;
			if (nonempty(head)) {
				last = q[tail].qprev;
				while (last != head) {
					curr = q[last].qkey;
					if (max == NULL || curr > max) {
						max = curr;
					}
					last = q[last].qkey;
				}
				
			}
		}
	}

	if (max == NULL) {
		pptr->pinh = 0;
	}
	else {
		pptr->pinh = max;
	}

	restore(ps);
	return;
}

int get_max_in_queue(int lock) {
	struct lentry* lptr;
	struct pentry* pptr;
	int last,curr,max, head, tail;
	max = NULL;
	lptr = &locks[lock];
	head = lptr->lqhead;
	tail = lptr->lqtail;
	last = q[tail].qprev;
	while (last != head) {
		pptr = &proctab[last];
		curr = pptr->pinh != 0 ? pptr->pinh : pptr->pprio;;
		if (max == NULL || curr > max) {
			max = curr;
		}
		last = q[last].qprev;
	}
	return max;
}

void get_last_in_queue(int lockdescriptor, int *result) {

	STATWORD ps;
	disable(ps);
	struct lentry* lptr;
	int last, head;
	*result = -1;
	lptr = &locks[lockdescriptor];
	head = lptr->lqhead;
	if (nonempty(head)) {
		last = q[lptr->lqtail].qprev;
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
	if (nonempty(head)) {
		last = q[lptr->lqtail].qprev;
		while (last != head) {
			if (q[last].qtype == type) {
				*result = last;
				break;
			}
			last = q[last].qprev;
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