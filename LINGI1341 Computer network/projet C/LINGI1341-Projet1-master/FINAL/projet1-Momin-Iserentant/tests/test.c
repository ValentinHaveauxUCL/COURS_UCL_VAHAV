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


void newpkt(){
	pkt_t *pkt = pkt_new();
	MY_CU_ASSERT(pkt!=NULL,"La fonction devrait renvoyé un packet alloué");
	pkt_del(pkt);
}

void test_asget(){
	pkt_t *pkt = pkt_new();
	char *ph = "roi";
	pkt_set_type(pkt, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(pkt, 1);
	pkt_set_seqnum(pkt, 2);
	pkt_set_length(pkt, 3);
	pkt_set_crc(pkt, 4);
	pkt_set_payload(pkt, ph,3);

	MY_CU_ASSERT(pkt_get_type(pkt)==PTYPE_DATA,"Doit être de type data...");
	MY_CU_ASSERT(pkt_get_window(pkt)==1,"Doit valloir 2...");
	MY_CU_ASSERT(pkt_get_length(pkt)==3,"Doit valloir 3...");
	MY_CU_ASSERT(pkt_get_crc(pkt)==4,"Doit valloir 4");
	MY_CU_ASSERT(strcmp(ph, pkt_get_payload(pkt))==0,"Doit être semblable");
	pkt_del(pkt);
	
}
void test_encode(){
	pkt_t *pkt = pkt_new();
	char *test = "yolo";
	pkt_set_type(pkt, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(pkt, 1);
	pkt_set_seqnum(pkt, 2);
	pkt_set_length(pkt, 4);
	pkt_set_crc(pkt, 4);
	pkt_set_payload(pkt, test,4);
	char buf[520];
	size_t len = 520;
	int retour = pkt_encode(pkt, (char *)buf, (size_t *) &len);
	MY_CU_ASSERT(len==12,"[ENCODE] =>length must be 12");
	MY_CU_ASSERT(retour==PKT_OK,"[ENCODE ]Must be ok for pkt_ok");
	size_t len2 = 10;
	retour = pkt_encode(pkt, (char *)buf, (size_t *) &len2);
	MY_CU_ASSERT(retour==E_NOMEM,"[ENCODE] must be out of memory");
	char *test2 = "yoloy";
	pkt_set_payload(pkt, test2, 5);
	size_t len3 = 50;
	retour = pkt_encode(pkt, (char *)buf, (size_t *) &len3);
	MY_CU_ASSERT(len3==16,"[ENCODE]=>paddng bug needs to be fixed");
	pkt_del(pkt);
}

void testdecode(){
	pkt_t *pkt = pkt_new();
	pkt_t *pkt2 = pkt_new();
	char *test = "yolo";
	pkt_set_type(pkt, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(pkt, 1);
	pkt_set_seqnum(pkt, 2);
	pkt_set_length(pkt, 4);
	pkt_set_crc(pkt, 4);
	pkt_set_payload(pkt, test,4);
	char buf[520];
	size_t len = 520;
	int retour = pkt_encode(pkt, (char *)buf, (size_t *) &len);
	retour = pkt_decode(buf, len, pkt2);
	MY_CU_ASSERT(pkt_get_type(pkt2)==PTYPE_DATA,"[DECODE]=>must be PTYPE_DATA");
	MY_CU_ASSERT(pkt_get_window(pkt2)==1,"[DECODE]=>win must equals 1");
	MY_CU_ASSERT(pkt_get_seqnum(pkt2)==2,"[DECODE]=>seqnum must equals 2");
	MY_CU_ASSERT(pkt_get_length(pkt2)==4,"[DECODE]=>length must equals 4");
	MY_CU_ASSERT(strcmp(pkt_get_payload(pkt2),"yolo")==0,"[DECODE]=>payload must be 'yolo'");
	retour = pkt_decode(buf, 2, pkt2);
	MY_CU_ASSERT(retour==E_NOHEADER,"[DECODE]=>must have no HEADER");
	retour = pkt_decode(NULL, 18, pkt2);
	MY_CU_ASSERT(retour==E_UNCONSISTENT,"[DECODE]=>must have a data");
	pkt_del(pkt);
	pkt_del(pkt2);
}

void test_amount_of_pad(){
	pkt_t *pkt = pkt_new();
	char *test = "yolo";
	pkt_set_type(pkt, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(pkt, 1);
	pkt_set_seqnum(pkt, 2);
	pkt_set_length(pkt, 12);
	pkt_set_crc(pkt, 4);
	pkt_set_payload(pkt, test,7);
	int amount = getAmountOfRealloc(pkt);
	MY_CU_ASSERT(amount == 1,"[GETPADDING=>must gives 3]");
	pkt_set_length(pkt, 11);
	amount = getAmountOfRealloc(pkt);
	MY_CU_ASSERT(amount==1,"[GETPADDING=>must gives 1]");
	pkt_set_length(pkt, 14);
	amount = getAmountOfRealloc(pkt);
	MY_CU_ASSERT(amount==2,"[GETPADDING=>must gives 1]");
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

void test_header(){
	pkt_t *pkt = pkt_new();
	char *test = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	pkt_set_type(pkt, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(pkt, 1);
	pkt_set_seqnum(pkt, 2);
	pkt_set_length(pkt, strlen(test));
	pkt_set_crc(pkt, 4);
	pkt_set_payload(pkt, test,strlen(test));
	char buf[520];
	size_t len = 520;
	pkt_encode(pkt, (char *)buf, (size_t *) &len);
	MY_CU_ASSERT(len==36,"[TEst_header]=>bad value for the length");
	int i;
	for(i=4;i<strlen(test);i++){
		MY_CU_ASSERT(test[i-4]==buf[i],"[TEST_DATA_PACKET]=>must be the same!");
	}
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

	pSuite = CU_add_suite("Suite de tests pour packet.h et socket.h", init_suite,clean_suite); 
	if (NULL == pSuite) { 
		CU_cleanup_registry(); 
		return CU_get_error(); 
	}
	if( (NULL == CU_add_test(pSuite,"real address",real_add_test))||(NULL == CU_add_test(pSuite,"test create socket",test_create_socket))||(NULL == CU_add_test(pSuite,"newpkt",newpkt))||(NULL == CU_add_test(pSuite,"test accesseur/guetteur",test_asget))||(NULL == CU_add_test(pSuite,"test encode",test_encode)),(NULL == CU_add_test(pSuite,"test_decode",testdecode))||(NULL == CU_add_test(pSuite,"test_amount_of_pad",test_amount_of_pad))||(NULL == CU_add_test(pSuite,"test_header",test_header)))
	{
		printf("Done!\n");
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();
}
 
