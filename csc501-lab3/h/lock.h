#ifndef _LOCK_H_
#define _LOCK_H_

#define DELETED  -6
#define READ -5
#define WRITE -4

#define NLOCKS 50 //number of locks
#define LINIT 0
#define	LFREE -1
#define LUSED 1

struct lentry {
	int lstatus;
	int ltype;
	int lprio;
	int lqhead;
	int lqtail;
	int proc_list[NPROC];
	int reader_count;
	int is_writer;
	int is_deleted;
};


extern struct lentry locks[];
extern int nextlock;
extern unsigned long ctr1000;

#define	isbadlock(l)	(l<0 || l>=NLOCKS)

extern void linit(void);
extern int lcreate(void);
extern int lock(int, int, int);
extern int newlockid(void);
extern int ldelete(int);
extern int releaseall(int, int);
extern int get_max_in_queue(int);
extern void get_last_in_queue(int, int*);
extern void get_type(int, int*, int);
extern int check_process_locks(int, int);
extern void acquire_lock(int, int, int);
extern int is_writer_waiting(int, int);
extern void set_priority_inheritance(int);
extern void reset_inherited_priority(int);
extern void make_process_wait(int, int, int, int);
#endif