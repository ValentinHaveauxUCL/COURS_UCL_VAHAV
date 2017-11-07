#include "packet.h"
#include "socket.h"
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


void real_add_test(){
	struct sockaddr_in6 *addr= malloc(sizeof(struct sockaddr_in6));
	const char local[] = "::1";
	const char * retour = real_address(local, addr);
	MY_CU_ASSERT(retour==NULL,"[REAL addr]=> must be ok");
	const char local2[] = "====";
	const char * try = real_address(local2, addr);
	MY_CU_ASSERT(try!=NULL,"[REAL]:>is not a valid address");
	free(addr);
}

void test_create_socket(){
	struct sockaddr_in6 *addr1= malloc(sizeof(struct sockaddr_in6));
	struct sockaddr_in6 *addr2= malloc(sizeof(struct sockaddr_in6));
	struct sockaddr_in6 *addr3= malloc(sizeof(struct sockaddr_in6));
	const char addr[]="::1";
	const char addr21[] = "====";
	int port1 = 12345;
	int port2 = 54321;
	real_address(addr, addr1);
	real_address(addr, addr2);
	real_address(addr21,addr3);
	int socket = create_socket((struct sockaddr_in6*)addr1,port1,(struct sockaddr_in6*) addr1,port2);
	MY_CU_ASSERT(socket>-1,"[CREATE SOCKET=> bug de création]");
	close(socket);
	socket = create_socket((struct sockaddr_in6*)addr1, port1, 0,0);
	MY_CU_ASSERT(socket>-1,"[CREATE_SOCKET]=>Must be able to create");
	close(socket);
	socket = create_socket((struct sockaddr_in6*)addr1,-2, 0,0);
	MY_CU_ASSERT(socket==-1,"[CREATE_SOCKET]=>Must failed");
	close(socket);
	socket = create_socket((struct sockaddr_in6*)addr1,12345, addr1,-4);
	MY_CU_ASSERT(socket==-1,"[CREATE_SOCKET]=>Must failed");
	close(socket);
	socket = create_socket((struct sockaddr_in6*)addr1,-1,(struct sockaddr_in6*) addr2,port2);
	MY_CU_ASSERT(socket==-1,"[CREATE SOCKET=> Must failed]");
	close(socket);
	socket = create_socket((struct sockaddr_in6*)addr3,12345, addr1,1325);
	MY_CU_ASSERT(socket==-1,"[CREATE_SOCKET]=>Must failed");
	close(socket);
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
	
	if( (NULL == CU_add_test(pSuite,"real address",real_add_test))||(NULL == CU_add_test(pSuite,"test create socket",test_create_socket)))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();
}
 
