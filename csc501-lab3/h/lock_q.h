#ifndef _LOCK_QUEUE_H_
#define _LOCK_QUEUE_H_


#ifndef	NLOCKQENT
#define	NLOCKQENT NLOCKS + NLOCKS + 4	/* for ready & sleep	*/
#endif


struct	qent_l {		/* one for each process plus two for	*/
				/* each list				*/
	int	qkey;		/* key on which the queue is ordered	*/
	int	qnext;		/* pointer to next process or tail	*/
	int	qprev;		/* pointer to previous process or head	*/
	int qtype;      /* this is the type of the lock in the queue*/
	unsigned long added_at; /*when this entry was made*/
};


extern struct qent_l q_l[];
extern	int	nextqueue_l;


#define	isempty_l(list)	(q_l[(list)].qnext >= NPROC)
#define	nonempty_l(list)	(q_l[(list)].qnext < NPROC)
#define	firstkey_l(list)	(q_l[q_l[(list)].qnext].qkey)
#define lastkey_l(tail)	(q_l[q_l[(tail)].qprev].qkey)
#define firstid_l(list)	(q_l[(list)].qnext)


extern int enqueue_l(int item, int tail);
extern int dequeue_l(int item);
extern int newqueue_l();
extern int insert_lq(int proc, int head, int key, int ltype);
extern int getfirst_l(int head);
extern int getlast_l(int tail);

#endif