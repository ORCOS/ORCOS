/*
    ORCOS - an Organic Reconfigurable Operating System
    Copyright (C) 2008 University of Paderborn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <orcos.h>
#include "string.hh"
#include <sys/timer.h>
#include <stdlib.h>
#include "stdio.h"
#include <string.h>

#define OK   0
#define FAIL 1


#define LINEFEED "\r\n"

#define TEST( x , name ) { puts("Testing " name); if (x(str) == OK) { puts("[OK]"LINEFEED);} else { puts("[FAILED]"LINEFEED); puts(str); puts(""LINEFEED);}  usleep(10000); }



#define ASSERT(x) if (!(x)) {str = temp; sprintf(str,"TEST (" #x ") FAILED. Line %d", __LINE__); return (FAIL);}

//#define ASSERT(value, msg, ...) if( value == 0) { str = temp; sprintf(str, msg ". value was: %d (%x)", ##__VA_ARGS__, value, value); return (FAIL);}
#define ASSERT_GREATER(value,comp,msg) if( !(value > comp)) {str = temp; sprintf(str,"TEST ASSERTION %u: " msg ". value was: %d (%x)", __LINE__, value, value);return (FAIL);}
#define ASSERT_EQUAL(value,comp,msg) if( ! (value == comp)) {str = temp; sprintf(str,"TEST ASSERTION IN %u: " msg ". value was: %d (%x)", __LINE__, value, value); return (FAIL);}
#define ASSERT_SMALLER(value,comp,msg) if( ! (value < comp)) {str = temp; sprintf(str,"TEST ASSERTION IN %u: " msg ". value was: %d (%x)", __LINE__, value, value); return (FAIL);}

char temp[200];

int test_new(char* &str) {
    void* address = malloc(1);
    ASSERT(address != 0);
    free(address);

    address = malloc(0);
    ASSERT(address != 0);
    free(address);

    unint4 addr = 0;
    unint4 size = 4096;
    int result = shm_map("/mem/mem", &addr, &size, cAllocate, 0);
    ASSERT(result > 0);
    ASSERT(addr != 0);
    ASSERT(size == 4096);

    // try mapping physical address 0x800000
    result = shm_map("/mem/mem", &addr, &size, 0, 0x800000);
    ASSERT(result > 0);
    ASSERT(addr != 0);
    ASSERT(size == 4096);

    while (1) {
        unint4 addr = 0;
        unint4 size = 0x100000;
        int result = shm_map("/mem/mem", &addr, &size, cAllocate, 0);
        if (result < 0) break;
        ASSERT(addr != 0);
        ASSERT(size == 0x100000);
    }

    close(result);

    for (int i = 0; i < 1024; i++) {
        address = malloc(4096);
        ASSERT(address != 0);
    }

    return (OK);
}

int test_task_run(char* &str) {
    int result;
    result = task_run(0,0);
    ASSERT(result < 0);

    result = task_run(0,(char*) 10);
    ASSERT(result < 0);

    result = task_run((char*) 1244,(char*) 10);
    ASSERT(result < 0);

    result = task_run("NOTASKHERE",0);
    ASSERT(result < 0);

    result = task_run("/",0);
    ASSERT(result < 0);

    result = task_run("//",0);
    ASSERT(result < 0);

    result = task_run("/////",0);
    ASSERT(result < 0);

    return (OK);
}

int test_task_kill(char* &str) {
    int result;
    result = task_kill(0);
    ASSERT(result < 0);

    result = task_kill(-1);
    ASSERT(result < 0);

    result = task_kill(-200);
    ASSERT(result < 0);

    result = task_kill(-65324);
    ASSERT(result < 0);

    return (OK);
}

int test_task_stop(char* &str) {
    int result;
    result = task_stop(0);
    ASSERT(result < 0);

    result = task_stop(-1);
    ASSERT(result < 0);

    result = task_stop(-200);
    ASSERT(result < 0);

    result = task_stop(-65324);
    ASSERT(result < 0);

    return (OK);
}

void* thread_entry(void* arg) {

}


int test_thread_create(char* &str) {
    int result;
    result = thread_create(0,0,0,0);
    ASSERT(result < 0);

    result = thread_create(0,0,thread_entry,0);
    ASSERT(result < 0);

    thread_attr_t attr;
    memset(&attr,0,sizeof(thread_attr_t));

    result = thread_create(0,&attr,thread_entry,0);
    ASSERT(result == cOk);

    result = thread_run(-1);
    ASSERT(result < 0);

    result = thread_run(0);
    ASSERT(result == 0);

    return (OK);
}

int test_files(char* &str) {
    int result;

    result = create(0,0);
    ASSERT(result < 0);

    result = create("//NOSUCHDIR//");
    ASSERT(result < 0);

    result = open("//NOSUCHDIR//",0);
    ASSERT(result < 0);

    result = open(0,0);
    ASSERT(result < 0);

    // open cwd
    result = open("",0);
    ASSERT(result > 0);

    result = close(result);
    ASSERT(result == cOk);

    result = open("/",0);
    ASSERT(result > 0);

    result = close(result);
    ASSERT(result == cOk);

    result = close(0);
    ASSERT(result < 0);

    result = close(-1);
    ASSERT(result < 0);

    result = close(-1123141);
    ASSERT(result < 0);

  /*  srand(getCycles());
    for (int i = 0; i < 1000; i++) {
        int num  = rand();
        int num2 = rand();
        int num3 = rand();
        result = fwrite(num, (char*) num2, num3);
        ASSERT_SMALLER(result,0,"fwrite() succeeded:");
        if (result >= 0) {
            printf("fwrite(%d,%d,%d)\n",num,num2,num3);
        }
    }*/

    result = create("testfile1");
    ASSERT(result > 0);
    result = write(result, "test1", 4);
    ASSERT(result == cOk);
    result = remove("testfile1");
    ASSERT(result == cOk);

    result = create("dirtest", cTYPE_DIR);
    ASSERT(result > 0);
    int result2 = create("dirtest/testfile1");
    ASSERT(result2 > 0);
    result2 = write(result2, "test1", 4);
    ASSERT(result2 == cOk);
    result2 = remove("dirtest/testfile1");
    ASSERT(result2 == cOk);
    result2 = remove("dirtest");
    ASSERT(result2 == cOk);

    /*
     * This tests file creation/removal on bugs as well
     * as the kernel memory allocator as we will be allocating
     * and deallocating lots of small file pointers.
     */
    for (int i = 0; i < 2000; i++) {
        result = create("/mnt/ramdisk/testfile1");
        ASSERT(result > 0);
        result = write(result, "test1", 4);
        ASSERT(result == cOk);

        result = create("/mnt/ramdisk/testfile2");
        ASSERT(result > 0);
        result = write(result, "test2", 4);
        ASSERT(result == cOk);

        result = create("/mnt/ramdisk/testfile3");
        ASSERT(result > 0);
        result = write(result, "test3", 4);
        ASSERT(result == cOk);

        result = remove("/mnt/ramdisk/testfile1");
        ASSERT(result == cOk);
        result = remove("/mnt/ramdisk/testfile2");
        ASSERT(result == cOk);
        result = remove("/mnt/ramdisk/testfile3");
        ASSERT(result == cOk);
    }


    for (int i = 0; i < 100; i++) {
        result = create("/mnt/ramdisk/testfile4");
        write(result, "test",4);
        result = create("/mnt/ramdisk/testfile5");
        write(result, "test",4);
        result = create("/mnt/ramdisk/testfile6");
        result = create("/mnt/ramdisk/testfile7");
        write(result, "test",4);
        result = remove("/mnt/ramdisk/testfile4");
        ASSERT_EQUAL(result,cOk,"remove() failed");
        result = remove("/mnt/ramdisk/testfile7");
        ASSERT_EQUAL(result,cOk,"remove() failed");
        result = remove("/mnt/ramdisk/testfile6");
        ASSERT_EQUAL(result,cOk,"remove() failed");
        result = remove("/mnt/ramdisk/testfile5");
        ASSERT_EQUAL(result,cOk,"remove() failed");
    }

    return (OK);
}


int test_net(char* &str) {
    int result;

    result = socket(0,0,0);
    ASSERT_SMALLER(result,0,"socket(0,0,0) succeeded");

    // TODO some more tests here

    return (OK);
}


static int signal_value;
static unint4 latency_time;

void* thread_entry_synchro(void* arg) {
    signal_value = 0xff;
    signal_value = signal_wait((void*) 200);
    // hopefully no overrun
    latency_time = (unint4) getCycles() - latency_time;
    //printf(LINEFEED"signal took: %d Cycles = %d ns"LINEFEED, time, time * 41);
}


int test_synchro(char* &str) {
    int result;
    thread_attr_t attr;
    memset(&attr,0,sizeof(thread_attr_t));

    // higher priority thread .. will directly preempt us on run
    attr.priority = 2000;

    result = thread_create(0,&attr,thread_entry_synchro,0);
    ASSERT_EQUAL(result,cOk,"thread_create(0,&attr,thread_entry,0) failed");

    result = thread_run(0);

    signal_value = -1;
    latency_time = (unint4) getCycles();
    signal_signal((void*) 200,723100);
    ASSERT_EQUAL(signal_value,723100,"signal_signal test failed..");

    return (OK);
}


int test_shmmem(char* &str) {
    int result;

    unint4 mapped_address = 0;
    unint4 mapped_size    = 1024;

    int handle = shm_map("/mem/sharedArea", &mapped_address, &mapped_size, cCreate);
    ASSERT_GREATER(handle,0,"shm_map failed with code.");

    ASSERT_GREATER(mapped_address, 0, "mapped address invalid");
    ASSERT_GREATER(mapped_size, 0, "mapped size invalid");

    // try writing to the shared memory area
    char* test = (char*) mapped_address;
    strcpy(test,"Hello");

    int cmp = strcmp(test,"Hello");
    ASSERT_EQUAL(cmp,cOk,"strcmp() on shared mem failed");

    result = close(handle);
    ASSERT_EQUAL(result,cOk,"fclose() on shared mem failed");

    return (OK);
}


int values[10];

static int num ;

void* thread_entry_rt(void* arg) {
    signal_value = 0xff;
    signal_value = signal_wait((void*) 70);
    values[num] = (int) arg;
    num++;
}

int test_rt(char* &str) {
    int result;

    thread_attr_t attr;
    memset(&attr,0,sizeof(thread_attr_t));


    num = 0;
    for (int i = 0; i < 10; i++) {
        values[i] = -1;
        attr.priority = 2000 + i;
        result = thread_create(0,&attr,thread_entry_rt,(void*) i);
        ASSERT_EQUAL(result,cOk,"thread_create(0,&attr,thread_entry,i) failed");
    }

    result = thread_run(0);

    //puts("Signaling. Expected Order: 9-0\n");
    signal_signal((void*) 70,5);
    // now check values

    for (int i = 0; i < 10; i++) {
        ASSERT_EQUAL(values[i],9-i,"signal_wait unblocking order test1 failed!");
    }

    num = 0;
    for (int i = 0; i < 10; i++) {
        values[i] = -1;
        attr.priority = 2000 - i;
        result = thread_create(0,&attr,thread_entry_rt,(void*) i);
        ASSERT_EQUAL(result,cOk,"thread_create(0,&attr,thread_entry,i) failed");
    }

    result = thread_run(0);

    //puts("Signaling. Expected Order: 0-9\n");
    signal_signal((void*) 70,5);
    // now check values

    for (int i = 0; i < 10; i++) {
        ASSERT_EQUAL(values[i],i,"signal_wait unblocking order test2 failed!");
    }

    return (OK);
}

unint8 nextTime = 0;
int4 minLatency = 0;
int4 maxLatency = 0;
int4 avgLatency = 0;
unint4 minIteration = 0;
unint4 maxIteration = 0;

int    iterations = 0;

#define MAX_ITERATIONS 10000

bool readKernelVarUInt4(char* filepath, unint4* result) {
    result[0] = 0;
    int file = open(filepath,0);
    if (file < 0)
        return (false);

    int num = read(file, (char*) result, 4);
    close(file);
    return (true);
}


unint4 clockfreqns = 1;
unint4 tscoverhead = 100;

unint8 times[100];

static inline unint4 rdtsc(void) {
    unint4 r = 0;
    asm volatile("mrc p15, 0, %0, c9, c13, 0; ISB;" : "=r"(r) );
    return r;
}


void* thread_entry_latency(void* arg) {
    unint4 tsc1 = rdtsc();
    unint8 now = getCycles();
    int latency = (int) (now - nextTime);
    unint4 tsc2 = rdtsc();


    //if (iterations < 100)
        //times[iterations] = now;


    if ((tsc2 - tsc1) < tscoverhead && latency > 0) {
        iterations++;
        if (iterations < MAX_ITERATIONS && iterations > 1) {
            avgLatency += latency;

            if (latency < minLatency) {
                minLatency      = latency;
                minIteration    = iterations;
            }
            if (latency > maxLatency) {
                maxLatency      = latency;
                maxIteration    = iterations;
            }
        }
    }


    nextTime += 1000 * clockfreqns;
}

int test_llThreadLatency(char* &str) {

    int iterations = 0;
    orcos_timer_t timer_conf;
    timer_conf.period_us = 100000;
    timer_conf.priority  = 0;
    timer_conf.threadId  = 0; // == current thread

    int fd = open("/dev/timer1");
    ASSERT_GREATER(fd,0," open(/dev/timer1) failed!");
    int ret = timer_configure(fd, &timer_conf);
    ASSERT_EQUAL(ret,0,"Configure Timer1 failed!");

    while (iterations < 5) {
        timer_wait();
        puts("llThread wokeup!\n");
        iterations++;
    }
    ret = timer_reset(fd);
    ASSERT_EQUAL(ret,0,"timer_reset failed!");
    ret = close(fd);
    ASSERT_EQUAL(ret,0,"close(timer_fd) failed");


    timer_conf.period_us = 100;
    timer_conf.priority  = 0;
    timer_conf.threadId  = 0; // == current thread
    fd = open("/dev/timer1");
    ASSERT_GREATER(fd,0," open(/dev/timer1) failed!");
    ret = timer_configure(fd, &timer_conf);
    ASSERT_EQUAL(ret,0,"Configure Timer1 failed!");
    iterations=0;
    while (iterations < 10000) {
         timer_wait();
         iterations++;
    }
    ret = timer_reset(fd);
    ASSERT_EQUAL(ret,0,"timer_reset failed!");
    ret = close(fd);
    ASSERT_EQUAL(ret,0,"close(timer_fd) failed");
}

int test_latency(char* &str) {
    int result;
    thread_attr_t attr;
    memset(&attr, 0, sizeof(thread_attr_t));


    readKernelVarUInt4("/sys/clockfreq", &clockfreqns);
    clockfreqns /= 1000000;
    printf("%d ns clock frequency"LINEFEED, clockfreqns);
    printf("%d ns per clock-cycle"LINEFEED, 1000 / clockfreqns);


    unint4 count = rdtsc();
    thread_self();
    count = rdtsc() - count;
    printf("thread_self  : %u processor-cycles, %u ns"LINEFEED, count, (count * 1000) / 1000);
    count = rdtsc();
    getCycles();
    count = rdtsc() - count;
    printf("getCycles    : %u processor-cycles, %u ns"LINEFEED, count, (count * 1000) / 1000);
    tscoverhead = ((count * 1200) / 1000);
    printf("getCycles    : allowed overhead: %u cycles, %u ns"LINEFEED, tscoverhead, (tscoverhead * 1000) / 1000);


    unint8 time = getCycles();
    unint4 overhead = ((getCycles() - time)) >> 1;
    printf("getCycles    : overhead: %u clock-cycles"LINEFEED,overhead);

    nextTime = getCycles() + 1000000 * clockfreqns;
    unint8 startTime = nextTime;
    minLatency = 999999;
    iterations = 0;
    maxLatency = 0;
    avgLatency = 0;

    attr.arrivaltime = nextTime;
    attr.priority    = 10000;
    attr.period      = 1000; /* 1 ms */
    ThreadIdT id;
    result = thread_create(&id,&attr,thread_entry_latency,0);
    ASSERT_EQUAL(result,cOk,"thread_create(&id,&attr,thread_entry_latency,0) failed");

    result = thread_run(id);
    ASSERT_EQUAL(result,cOk,"thread_run(id) failed");

    while (iterations < MAX_ITERATIONS) {
        usleep(10000);
    }

    thread_terminate(id, TERM_SOFT);

    minLatency -= overhead;
    maxLatency -= overhead;
    avgLatency /= MAX_ITERATIONS;
    avgLatency -= overhead;

    printf("minLatency   : %d clock-cycles [%d ns] (it. %d)"LINEFEED"avgLatency   : %d clock-cycles [%d ns]"LINEFEED"maxLatency   : %d clock-cycles [%d ns] (it. %d)"LINEFEED,
           minLatency,(minLatency * 41), minIteration,
           avgLatency, (avgLatency * 41),
           maxLatency,(maxLatency * 41), maxIteration);

    return (cOk);
}

extern "C" int main(int argc, char** argv)
{
    char* str;
    puts("Running ORCOS Syscall Tests"LINEFEED);

    TEST(test_new,          "SC_NEW             (Memory allocation tests)    ");
    TEST(test_task_run,     "SC_TASK_RUN        (Task running tests)         ");
    TEST(test_task_kill,    "SC_TASK_KILL       (Task kill syscall tests)    ");
    TEST(test_task_stop,    "SC_TASK_STOP       (Task stopping tests)        ");
    TEST(test_thread_create,"SC_THREAD_CREATE   (Thread creation tests)      ");
    TEST(test_files,        "SC_FILES           (I/O and files syscall tests)");
    TEST(test_net,          "SC_NET             (Networking tests)           ");
    TEST(test_synchro,      "SC_SYNCHRO         (Synchronization test)       ");
    TEST(test_shmmem,       "SC_SHMMEM          (Shared memory tests)        ");
    TEST(test_rt,           "SC_PRIORITY        (Thread Priority tests)      ");

    printf(LINEFEED"---------------------------------------"LINEFEED);
    printf("   Timer based low Latency Thread Test"LINEFEED);
    printf("---------------------------------------"LINEFEED);

    test_llThreadLatency(str);

    printf(LINEFEED"-----------------------"LINEFEED);
    printf("   Latency Test"LINEFEED);
    printf("-----------------------"LINEFEED);

    test_latency(str);
}

