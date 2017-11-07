#include "packet.h"
#include "socket.h"
#include "sender.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
//Definition of the assert
#define MY_CU_ASSERT(value, args...) do { \
    int __b__ = (value); \
    if(! __b__) printf(args); \
    CU_ASSERT_FATAL(__b__); \
} while(0)


/*
Suite initialisation function 
Returns 0 on succes 
*/
int init_suite(void){
	return 0;
}

/*
Suite cleanup function
Returns 0 on success
*/
int clean_suite(void){
	return 0;
}


void test_getTime(){
	struct timeval * start = malloc(sizeof(struct timeval));
	struct timeval * stop = malloc(sizeof(struct timeval));
	(*start).tv_sec = 3;
	(*start).tv_usec = 750000;
	(*stop).tv_sec = 10;
	(*stop).tv_usec = 500000;
	Timer time = getTime(start, stop);
	MY_CU_ASSERT(time.sec == 7, "[getTimer]=>sec=3!");
	MY_CU_ASSERT(time.usec == 250000, "[getTimer]=>sec=250000!");
	
}

void test_create_socket(){
	
}

/*
 *Function main.
 *This function builds the suite of tests.
 *After execution it shows a tabe that records all the tests and 
 *that shows where there is an error.
 */
int main(){

	CU_pSuite pSuite = NULL; 
	if(CUE_SUCCESS != CU_initialize_registry()){
		return CU_get_error();
	}

	pSuite = CU_add_suite("Suite de tests pour sender.c et receiver.c", init_suite,clean_suite); 
	if (NULL == pSuite) { 
		CU_cleanup_registry(); 
		return CU_get_error(); 
	}
	
	if( (NULL == CU_add_test(pSuite,"test create socket",test_create_socket)))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();
}
 
