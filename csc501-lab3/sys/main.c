/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
int mystrncmp(char* des, char* target, int n) {
    int i;
    for (i = 0; i < n; i++) {
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}

/*--------------------------------Test 1--------------------------------*/

void reader1(char* msg, int lck)
{
    lock(lck, READ, DEFAULT_LOCK_PRIO);
    kprintf("  %s: acquired lock, sleep 2s\n", msg);
    sleep(2);
    kprintf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void test1()
{
    int	lck;
    int	pid1;
    int	pid2;

    kprintf("\nTest 1: readers can share the rwlock\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 1 failed");

    pid1 = create(reader1, 2000, 20, "reader a", 2, "reader a", lck);
    pid2 = create(reader1, 2000, 20, "reader b", 2, "reader b", lck);

    resume(pid1);
    resume(pid2);

    sleep(5);
    ldelete(lck);
    kprintf("Test 1 ok\n");
}


/*----------------------------------Test 2---------------------------*/
char output2[13];
int count2;
void reader2(char msg, int lck, int lprio)
{
    int     ret;

    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, READ, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    sleep(3);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}

void writer2(char msg, int lck, int lprio)
{
    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, WRITE, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    sleep(1);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}

void test2()
{
    count2 = 0;
    int     lck;
    int     rd1, rd2, rd3, rd4;
    int     wr1;

    kprintf("\nTest 2: wait on locks with priority. Expected order of"
        " lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 2 failed");

    rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
    rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
    rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
    rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
    wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 25);

    kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
    resume(rd1);
    sleep(1);

    kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
    resume(wr1);
    sleep(1);


    kprintf("-start reader B, D, E. reader B is granted lock.\n");
    resume(rd2);
    resume(rd3);
    resume(rd4);


    sleep(15);
    kprintf("output=%s\n", output2);
    // ABD(ABD in arbitrary orders)CCEE
    assert(mystrncmp(output2, "ABABDDCCEE", 10) == 0, "Test 2 FAILED\n");
    kprintf("Test 2 OK\n");
}


/*----------------------------------Test 3---------------------------*/
void reader3(char* msg, int lck)
{
    int     ret;

    kprintf("  %s: to acquire lock\n", msg);
    lock(lck, READ, DEFAULT_LOCK_PRIO);
    kprintf("  %s: acquired lock\n", msg);
    kprintf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void writer3(char* msg, int lck)
{
    kprintf("  %s: to acquire lock\n", msg);
    lock(lck, WRITE, DEFAULT_LOCK_PRIO);
    kprintf("  %s: acquired lock, sleep 10s\n", msg);
    sleep(10);
    kprintf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void test3()
{
    int     lck;
    int     rd1, rd2;
    int     wr1;

    kprintf("\nTest 3: test the basic priority inheritence\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 3 failed");

    rd1 = create(reader3, 2000, 25, "reader3", 2, "reader A", lck);
    rd2 = create(reader3, 2000, 30, "reader3", 2, "reader B", lck);
    wr1 = create(writer3, 2000, 20, "writer3", 2, "writer", lck);

    kprintf("-start writer, then sleep 1s. lock granted to write (prio 20)\n");
    resume(wr1);
    sleep(1);

    kprintf("-start reader A, then sleep 1s. reader A(prio 25) blocked on the lock\n");
    resume(rd1);
    sleep(1);
    assert(getprio(wr1) == 25, "Test 3 failed here 1");

    kprintf("-start reader B, then sleep 1s. reader B(prio 30) blocked on the lock\n");
    resume(rd2);
    sleep(1);
    assert(getprio(wr1) == 30, "Test 3 failed 2");

    kprintf("-kill reader B, then sleep 1s\n");
    kill(rd2);
    sleep(1);
    assert(getprio(wr1) == 25, "Test 3 failed 3");

    kprintf("-kill reader A, then sleep 1s\n");
    kill(rd1);
    sleep(1);
    assert(getprio(wr1) == 20, "Test 3 failed 4");

    sleep(8);
    kprintf("Test 3 OK\n");
}

void test5()
{
    count2 = 0;
    int     lck;
    int     rd1, rd2, rd3, rd4, rd5;
    int     wr1;

    kprintf("\nTest 5: wait on locks with priority. Expected order of"
        " lock acquisition is: reader A, reader B, reader D, writer C & reader E\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 2 failed");

    rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
    rd2 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 30);
    rd3 = create(reader2, 2000, 20, "reader2", 3, 'D', lck, 25);
    rd4 = create(reader2, 2000, 20, "reader2", 3, 'E', lck, 20);
    rd5 = create(reader2, 2000, 20, "reader2", 3, 'F', lck, 20);
    wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 25);

    kprintf("-start reader A, then sleep 1s. lock granted to reader A\n");
    resume(rd1);
    sleep(1);

    kprintf("-start writer C, then sleep 1s. writer waits for the lock\n");
    resume(wr1);
    sleep(1);


    kprintf("-start reader B, D, E. reader B is granted lock.\n");
    resume(rd2);
    resume(rd3);
    resume(rd4);
    resume(rd5);

    sleep(15);
    kprintf("output=%s\n", output2);
    // ABD(ABD in arbitrary orders)CCEE
    assert(mystrncmp(output2, "ABABDDCCEFFE", 12) == 0, "Test 5 FAILED\n");
    kprintf("Test 2 OK\n");
}

void reader4(char* msg, int lck)
{
    int     ret;
    kprintf("  %s: to acquire lock\n", msg);
    lock(lck, READ, DEFAULT_LOCK_PRIO);
    kprintf("  %s: acquired lock\n", msg);
    sleep(1);
    kprintf("  %s: to release lock\n", msg);
    releaseall(1, lck);
}

void reader5(char* msg, int lck)
{
    int     ret;
    kprintf("  %s: to acquire lock\n", msg);
    lock(lck, READ, DEFAULT_LOCK_PRIO);
    kprintf("  %s: acquired lock\n", msg);
    kprintf("  %s: to release lock\n", msg);
    releaseall(1, lck);
    sleep(5);
    kprintf("Calling this again to trigger ldelete case\n");
    lock(lck, READ, DEFAULT_LOCK_PRIO);
}


void test4() {
    int     lck;
    int     rd1, rd2;
    int     wr1,i;
    kprintf("\nTest 4: ldelete case\n");
    lck = lcreate();
    kprintf("The lock descriptor is: %d\n", lck);
    assert(lck != SYSERR, "Test 4 failed");
    rd1 = create(reader4, 2000, 25, "reader4", 2, "reader A", lck);
    rd2 = create(reader5, 2000, 25, "reader4", 2, "reader B", lck);
    resume(rd1);
    resume(rd2);
    ldelete(lck);
    sleep(4);
    locks[lck].lstatus = LINIT;
    rd1 = create(reader4, 2000, 25, "reader4", 2, "reader C", lck);
    resume(rd1);
    sleep(10);
    return;
}

void writer6(char msg, int lck, int lprio)
{
    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, WRITE, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    sleep(5);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}

void test6() {
    int	lck;
    int	wr1, wr2;

    kprintf("\nTest 6: Writer's can't share the rwlock\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 6 failed");
    wr1 = create(writer6, 2000, 20, "writer6", 3, 'A', lck, 25);
    wr2 = create(writer6, 2000, 20, "writer6", 3, 'B', lck, 25);

    resume(wr1);
    resume(wr2);

    sleep(11);
    kprintf('\n Test 6 OK');
}

void writer7(char msg, int lck, int lprio)
{
    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, WRITE, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    sleep(7);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}


void test7()
{
    count2 = 0;
    int     lck;
    int     rd1, rd2;
    int     wr1, wr2;

    kprintf("\nTest 7: write before read if, equal priorities");
    lck = lcreate();
    assert(lck != SYSERR, "Test 2 failed");


    wr1 = create(writer7, 2000, 20, "writer7", 3, 'A', lck, 25);
    rd1 = create(reader2, 2000, 20, "reader2", 3, 'B', lck, 20);
    rd2 = create(reader2, 2000, 20, "reader2", 3, 'C', lck, 20);
    wr2 = create(writer7, 2000, 20, "writer7", 3, 'D', lck, 20);

    kprintf("-start Writer A, then sleep 5s. lock granted to writer A\n");
    resume(wr1);
   
    kprintf("-start reader B, then sleep 1s. reader waits for the lock\n");
    resume(rd1);
    resume(rd2);
    //sleep(5); //uncomment this to test change order of lock acquisition
    kprintf("-start writer C, writer C should acquire before reader B\n");
    resume(wr2);

    sleep(20);
    kprintf("output=%s\n", output2);
    // AACCBB
    assert(mystrncmp(output2, "AACCBB", 6) == 0, "Test 7 FAILED\n");
    kprintf("Test 7 OK\n");
}

void reader8(char msg, int lck, int lprio)
{
    int     ret;

    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, READ, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}

void writer8(char msg, int lck, int lprio)
{
    kprintf("  %c: to acquire lock\n", msg);
    lock(lck, WRITE, lprio);
    output2[count2++] = msg;
    kprintf("  %c: acquired lock, sleep 3s\n", msg);
    sleep(5);
    output2[count2++] = msg;
    kprintf("  %c: to release lock\n", msg);
    releaseall(1, lck);
}

void test8() {
    int     lck;
    int     rd1;
    int     wr1;
    kprintf("\nTest 8: ldelete case after process kill\n");
    lck = lcreate();
    assert(lck != SYSERR, "Test 4 failed");
    rd1 = create(reader2, 2000, 20, "reader2", 3, 'A', lck, 20);
    wr1 = create(writer2, 2000, 20, "writer2", 3, 'B', lck, 25);
    resume(wr1);
    resume(rd1);
   /* ldelete(lck);
    locks[lck].lstatus = LINIT;
    kill(wr1);
    wr1 = create(writer2, 2000, 20, "writer2", 3, 'C', lck, 25);
    resume(wr1);*/
    sleep(20);
    kprintf("TEST 8 OK");
    return;
}


int main()
{
    /* These test cases are only used for test purpose.
     * The provided results do not guarantee your correctness.
     * You need to read the PA2 instruction carefully.
     */
    test1();
    test2();
    test3();
   /* test4();
    test5();
    test6();
    test7();
    test8();*/

    /* The hook to shutdown QEMU for process-like execution of XINU.
     * This API call exists the QEMU process.
     */
    shutdown();
}
