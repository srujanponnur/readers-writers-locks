#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>

int insert_lq(int proc, int head, int key, int ltype) {

	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey = key;
	q[proc].qtype = ltype;
	q[proc].added_at = ctr1000;
	//kprintf("The current value is: %d\n", (int)q[proc].added_at);
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
}

void print_queue(int lockdescriptor) {
	struct lentry* lptr;
	int head, tail, last;
	lptr = &locks[lockdescriptor];
	head = lptr->lqhead;
	tail = lptr->lqtail;
	last = q[tail].qprev;
	//kprintf("The queue entries in lock: %d are\n", lockdescriptor);
	while (last != head) {
		//kprintf("%d ", last);
		last = q[last].qprev;
	}
	kprintf("\n");
	return;
}

int get_queue_count(int lockdescriptor) {
	struct lentry* lptr;
	int head, tail, last, count = 0;
	lptr = &locks[lockdescriptor];
	head = lptr->lqhead;
	tail = lptr->lqtail;
	last = q[tail].qprev;
	while (last != head) {
		count++;
		last = q[last].qprev;
	}
	//kprintf("\nThe count in the lock queue: %d\n", count);
	return count;
}


//
//int enqueue_l(int item, int tail) {
//	struct	qent_l* tptr;		/* points to tail entry		*/
//	struct	qent_l* mptr;		/* points to item entry		*/
//
//	tptr = &q[tail];
//	mptr = &q[item];
//	mptr->qnext = tail;
//	mptr->qprev = tptr->qprev;
//	q[tptr->qprev].qnext = item;
//	tptr->qprev = item;
//	return(item);
//}
//
//
//int dequeue_l(int item) {
//	struct	qent_l* mptr;		/* pointer to q entry for item	*/
//
//	mptr = &q[item];
//	q[mptr->qprev].qnext = mptr->qnext;
//	q[mptr->qnext].qprev = mptr->qprev;
//	return(item);
//}
//
//
//int getfirst_l(int head) {
//	int	proc;			/* first process on the list	*/
//
//	if ((proc = q[head].qnext) < NPROC)
//		return(dequeue_l(proc));
//	else
//		return(EMPTY);
//}
//
//int getlast_l(int tail) {
//	int	proc;			/* last process on the list	*/
//
//	if ((proc = q[tail].qprev) < NPROC)
//		return(dequeue_l(proc));
//	else
//		return(EMPTY);
//}

