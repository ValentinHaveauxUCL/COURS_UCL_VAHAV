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
void delpkt(){
	pkt_t *pkt = pkt_new();
	pkt_del(pkt);
	MY_CU_ASSERT(pkt==NULL,"La fonction devrait avoir free le packet");
}
void testsubstring(){
	char *str =(char *) malloc(sizeof(char)*9);
	*str = "testtest";
	char *newstr =substring(str, 4, 4);
	MY_CU_ASSERT(newstr[0]=='t',"Le debut de la sous chaine de caractère devrait être t");
	MY_CU_ASSERT(newstr[4]=='\0',"La sous chaine de caractère n'est pas correctement terminée");
	free(str);
	free(newstr);

}
void testdecode(){
	pkt_status_code test1 = decode(NULL, 4,NULL);// CHANGER ET PAS METTRE DE NULL
	MY_CU_ASSERT(test1==E_NOHEADER,"Devrait renvoyer E_NOHEADER");
	pkt_status_code test2 = decode(NULL, 10,NULL);
	MY_CU_ASSERT(test2==E_UNCONSISTENT,"Devrait renvoyer E_UNCONSISTENT ");
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
	
	if( (NULL == CU_add_test(pSuite,"Test pkt_new",newpkt))||(NULL == CU_add_test(pSuite,"Test pkt_del",delpkt))||(NULL == CU_add_test(pSuite,"Test substring",testsubstring)))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());
	CU_cleanup_registry();
	return CU_get_error();
}
 
