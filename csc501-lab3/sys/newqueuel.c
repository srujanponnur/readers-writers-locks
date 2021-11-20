/* newqueue.c  -  newqueue */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <lock_q.h>

/*------------------------------------------------------------------------
 * newqueue  --  initialize a new list in the q_l structure
 *------------------------------------------------------------------------
 */
int newqueue_l()
{
	struct	qent_l *hptr;
	struct	qent_l *tptr;
	int	hindex, tindex;

	hptr = &q_l[hindex=nextqueue_l++]; /* assign and rememeber queue	*/
	tptr = &q_l[tindex=nextqueue_l++]; /* index values for head&tail	*/
	hptr->qnext = tindex;
	hptr->qprev = EMPTY;
	hptr->qkey = MININT;
	tptr->qnext = EMPTY;
	tptr->qprev = hindex;
	tptr->qkey = MAXINT;
	return(hindex);
}
