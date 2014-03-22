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

#include <orcos.hh>
#include "string.hh"

#define OK   0
#define FAIL 1


#define TEST( x , name )  if (x(str) == OK) { puts("[OK] " name "\n\r");} else { puts("[FAILED] " name " '" ); puts(str); puts("'\n\r");}

#define ASSERT(value,msg) if( value == 0) { str = temp; sprintf(str,msg ". value was: %x",value); return (FAIL);}
#define ASSERT_GREATER(value,comp,msg) if( !(value > comp)) {str = temp; sprintf(str,msg ". value was: %x",value);return (FAIL);}
#define ASSERT_EQUAL(value,comp,msg) if( ! (value == comp)) {str = temp; sprintf(str,msg ". value was: %x",value); return (FAIL);}
#define ASSERT_SMALLER(value,comp,msg) if( ! (value < comp)) {str = temp; sprintf(str,msg ". value was: %x",value); return (FAIL);}

char temp[200];

int test_new(char* &str) {
	void* address = malloc(1);
	ASSERT(address,"malloc(1)!= 0");
	free(address);

	address = malloc(0);
	ASSERT(address,"malloc(0) != 0");
	free(address);

//	address = malloc(-1);
//	ASSERT_EQUAL(address,0,"malloc(-1) == 0");

	return (OK);
}

int test_task_run(char* &str) {
	int result;
	result = task_run(0,0);
	ASSERT_SMALLER(result,0,"task_run(0,0) succeeded");

	result = task_run(0,(char*) 10);
	ASSERT_SMALLER(result,0,"task_run(0,10) succeeded");

	result = task_run((char*) 1244,(char*) 10);
	ASSERT_SMALLER(result,0,"task_run(1244,10) succeeded");

	result = task_run("NOTASKHERE",0);
	ASSERT_SMALLER(result,0,"task_run(NOTASKHERE,0) succeeded");

	result = task_run("/",0);
	ASSERT_SMALLER(result,0,"task_run(/,0) succeeded");

	result = task_run("//",0);
	ASSERT_SMALLER(result,0,"task_run(//,0) succeeded");

	result = task_run("/////",0);
	ASSERT_SMALLER(result,0,"task_run(/////,0) succeeded");


	return (OK);
}

int test_task_kill(char* &str) {
	int result;
	result = task_kill(0);
	ASSERT_SMALLER(result,0,"task_kill(0) succeeded");

	result = task_kill(-1);
	ASSERT_SMALLER(result,0,"task_kill(-1) succeeded");

	result = task_kill(-200);
	ASSERT_SMALLER(result,0,"task_kill(-200) succeeded");

	result = task_kill(-65324);
	ASSERT_SMALLER(result,0,"task_kill(-65324) succeeded");

	return (OK);
}

int test_task_stop(char* &str) {
	int result;
	result = task_stop(0);
	ASSERT_SMALLER(result,0,"task_stop(0) succeeded");

	result = task_stop(-1);
	ASSERT_SMALLER(result,0,"task_stop(0) succeeded");

	result = task_stop(-200);
	ASSERT_SMALLER(result,0,"task_stop(-200) succeeded");

	result = task_stop(-65324);
	ASSERT_SMALLER(result,0,"task_stop(-65324) succeeded");

	return (OK);
}

void* thread_entry(void* arg) {

}


int test_thread_create(char* &str) {
	int result;
	result = thread_create(0,0,0,0);
	ASSERT_SMALLER(result,0,"thread_create(0,0,0,0) succeeded");

	result = thread_create(0,0,thread_entry,0);
	ASSERT_SMALLER(result,0,"thread_create(0,0,thread_entry,0) succeeded");

	thread_attr_t attr;
	memset(&attr,0,sizeof(thread_attr_t));

	result = thread_create(0,&attr,thread_entry,0);
	ASSERT_EQUAL(result,cOk,"thread_create(0,&attr,thread_entry,0) failed");

	result = thread_run(-1);
	ASSERT(result,"thread_run(-1) succeeded");

	result = thread_run(0);
	ASSERT_EQUAL(result,0,"thread_run(0) failed");

	return (OK);
}

int test_files(char* &str) {
	int result;

	result = fcreate(0,0);
	ASSERT_SMALLER(result,0,"fcreate(0,0) succeeded");

	result = fcreate(0,"test");
	ASSERT_SMALLER(result,0,"fcreate(0,test) succeeded");

	result = fcreate("test",0);
	ASSERT_SMALLER(result,0,"fcreate(test,0) succeeded");

	result = fcreate("/","//NOSUCHDIR//");
	ASSERT_SMALLER(result,0,"fcreate(test,//NOSUCHDIR//) succeeded");

	result = fopen("//NOSUCHDIR//",0);
	ASSERT_SMALLER(result,0,"fopen(//NOSUCHDIR//) succeeded");

	result = fopen(0,0);
	ASSERT_SMALLER(result,0,"fopen(0) succeeded");

	result = fopen("/",0);
	ASSERT_GREATER(result,0,"fopen(/) failed");

	result = fclose(result);
	ASSERT_EQUAL(result,cOk,"fclose(/) failed");

	result = fclose(0);
	ASSERT_SMALLER(result,0,"fclose(0) succeeded");

	result = fclose(-1);
	ASSERT_SMALLER(result,0,"fclose(-1) succeeded");

	result = fclose(-1123141);
	ASSERT_SMALLER(result,0,"fclose(-1123141) succeeded");

	return (OK);
}


int test_net(char* &str) {
	int result;

	result = socket(0,0,0);
	ASSERT_SMALLER(result,0,"socket(0,0,0) succeeded");


	return (OK);
}


static int signal_value;

void* thread_entry_synchro(void* arg) {
	signal_value = 0xff;
	signal_value = signal_wait((void*) 200);
}

int test_synchro(char* &str) {
	int result;


	thread_attr_t attr;
	memset(&attr,0,sizeof(thread_attr_t));

	result = thread_create(0,&attr,thread_entry_synchro,0);
	ASSERT_EQUAL(result,cOk,"thread_create(0,&attr,thread_entry,0) failed");

	result = thread_run(result);

	sleep(10);
	signal_signal((void*) 200,723100);
	sleep(200);
	ASSERT_EQUAL(signal_value,723100,"signal_signal test failed..");

	return (OK);
}


extern "C" int task_main()
{
	char* str;
	puts("Running ORCOS Syscall Tests\r\n");

	TEST(test_new,			"SC_NEW");
	TEST(test_task_run,		"SC_TASK_RUN");
	TEST(test_task_kill,	"SC_TASK_KILL");
	TEST(test_task_stop,	"SC_TASK_STOP");
	TEST(test_thread_create,"SC_THREAD_CREATE");
	TEST(test_files,		"SC_FILES");
	TEST(test_net,			"SC_NET");
	TEST(test_synchro,		"SC_SYNCHRO");
}

