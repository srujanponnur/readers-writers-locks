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
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
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

