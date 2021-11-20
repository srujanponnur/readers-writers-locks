#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <lock_q.h>

int insert_lq(int proc, int head, int key, int ltype) {

	int	next;			/* runs through list		*/
	int	prev;

	next = q_l[head].qnext;
	while (q_l[next].qkey < key)	/* tail has maxint as key	*/
		next = q_l[next].qnext;
	q_l[proc].qnext = next;
	q_l[proc].qprev = prev = q_l[next].qprev;
	q_l[proc].qkey = key;
	q_l[proc].qtype = ltype;
	q_l[proc].added_at = ctr1000;
	q_l[prev].qnext = proc;
	q_l[next].qprev = proc;
	return(OK);
}



int enqueue_l(int item, int tail) {
	struct	qent_l* tptr;		/* points to tail entry		*/
	struct	qent_l* mptr;		/* points to item entry		*/

	tptr = &q_l[tail];
	mptr = &q_l[item];
	mptr->qnext = tail;
	mptr->qprev = tptr->qprev;
	q_l[tptr->qprev].qnext = item;
	tptr->qprev = item;
	return(item);
}


int dequeue_l(int item) {
	struct	qent_l* mptr;		/* pointer to q_l entry for item	*/

	mptr = &q_l[item];
	q_l[mptr->qprev].qnext = mptr->qnext;
	q_l[mptr->qnext].qprev = mptr->qprev;
	return(item);
}


int getfirst_l(int head) {
	int	proc;			/* first process on the list	*/

	if ((proc = q_l[head].qnext) < NPROC)
		return(dequeue_l(proc));
	else
		return(EMPTY);
}

int getlast_l(int tail) {
	int	proc;			/* last process on the list	*/

	if ((proc = q_l[tail].qprev) < NPROC)
		return(dequeue_l(proc));
	else
		return(EMPTY);
}

