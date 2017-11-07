
#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <string.h>

#include "packet_interface.h"
#include "real_address.h"
#include "create_socket.h"


///*	Initialisation de la suite de tests
// *
// */
//int initSuite()
//{
//	
//}
//
///* Clean up de la suite de tests
// *
// */
//int cleanSuite()
//{
//	
//}


/* Test de pkt_new()
 *
 */
void testPkt_new()
{
	pkt_t *p = pkt_new();
	CU_ASSERT_PTR_NOT_EQUAL(p, NULL);
	pkt_del(p);
}


/* Test de pkt_decode().
 *
 */
void testPkt_decode()
{
	pkt_t *p = pkt_new();
	pkt_t *p1 = pkt_new();
	//pkt_t *p2 = pkt_new();
	pkt_t *p3 = pkt_new();
	//pkt_t *p4 = pkt_new();
	
	char *c = "test";
	pkt_set_type(p, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(p, 1);
	pkt_set_seqnum(p, 2);
	pkt_set_length(p, 4);
	pkt_set_crc(p, 4);
	pkt_set_payload(p, c, 4);
	
	char buf[400];
	size_t length = 400;
	int result = pkt_encode(p, buf, &length);
	result = pkt_decode(buf, length, p1);
	CU_ASSERT_EQUAL(pkt_get_type(p1), PTYPE_DATA);
	CU_ASSERT_EQUAL(pkt_get_window(p1), 1);
	CU_ASSERT_EQUAL(pkt_get_seqnum(p1), 2);
	CU_ASSERT_EQUAL(pkt_get_length(p1), 4);
	//printf(" crc is %i \n", (unsigned int) pkt_get_crc(p1));
	CU_ASSERT_EQUAL(pkt_get_crc(p1), 4);
	CU_ASSERT_STRING_EQUAL(pkt_get_payload(p1), c);
	CU_ASSERT_EQUAL(result, PKT_OK);
	
	pkt_del(p1);
	p1 = pkt_new();
	result = pkt_decode(buf, 2, p1);
	CU_ASSERT_PTR_NOT_EQUAL(p, NULL);
	CU_ASSERT_EQUAL(result, E_NOHEADER);
	length = 50;
	//result = pkt_decode(NULL, length, p1);
	//CU_ASSERT_EQUAL(result, E_UNCONSISTENT);
	
//	pkt_set_type(p2, (const ptypes_t) 3);
//	pkt_set_window(p2, 1);
//	pkt_set_seqnum(p2, 2);
//	pkt_set_length(p2, 4);
//	pkt_set_crc(p2, 4);
//	pkt_set_payload(p2, c, 4);
//	length = 50;
//	result = pkt_encode(p2, buf, &length);
//	result = pkt_decode(buf, length, p1);
//	CU_ASSERT_EQUAL(result, E_TYPE);
	pkt_set_type(p3, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(p3, 1);
	pkt_set_seqnum(p3, 2);
	pkt_set_length(p3, 3);
	pkt_set_crc(p3, 4);
	pkt_set_payload(p3, c, 4);
	length = 50;
	result = pkt_encode(p3, buf, &length);
	result = pkt_decode(buf, length, p1);
	CU_ASSERT_EQUAL(result, E_LENGTH);
//	pkt_set_type(p4, (const ptypes_t) PTYPE_DATA);
//	pkt_set_window(p4, 1);
//	pkt_set_seqnum(p4, 2);
//	pkt_set_length(p4, 4);
//	pkt_set_crc(p4, 3);
//	pkt_set_payload(p4, c, 4);
//	length = 50;
//	result = pkt_encode(p4, buf, &length);
//	result = pkt_decode(buf, length, p1);
//	CU_ASSERT_EQUAL(result, E_CRC);
	
	pkt_del(p);
	//pkt_del(p1);
	//pkt_del(p2);
	pkt_del(p3);
	//pkt_del(p4);
}

/* Test de pkt_encode().
 *
 */
void testPkt_encode()
{
	pkt_t *p = pkt_new();
	char *c = "test";
	pkt_set_type(p, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(p, 1);
	pkt_set_seqnum(p, 2);
	pkt_set_length(p, 4);
	pkt_set_crc(p, 4);
	pkt_set_payload(p, c, 4);
	
	char buf[400];
	size_t length = 400;
	int result = pkt_encode(p, buf, &length);
	CU_ASSERT_EQUAL(length, 16);
	CU_ASSERT_EQUAL(result, PKT_OK);
	
	size_t length1 = 10;
	result = pkt_encode(p, buf, &length1);
	CU_ASSERT_EQUAL(result, E_NOMEM);
	
	char *c1 = "test2";
	pkt_set_payload(p, c1, 5);
	length = 25;
	result = pkt_encode(p, buf, &length);
	CU_ASSERT_EQUAL(length, 17); //verifier la valeur
	
	pkt_del(p);
	
}

/* Test des differents get et set
 *
 */
void testPkt_getsSets()
{
	pkt_t *p = pkt_new();
	char *c = "tmp";
	pkt_set_type(p, (const ptypes_t) PTYPE_DATA);
	pkt_set_window(p, 1);
	pkt_set_seqnum(p, 2);
	pkt_set_length(p, 3);
	pkt_set_crc(p, 4);
	pkt_set_payload(p, c, 3);
	
	CU_ASSERT_EQUAL(pkt_get_type(p), PTYPE_DATA);
	CU_ASSERT_EQUAL(pkt_get_window(p), 1);
	CU_ASSERT_EQUAL(pkt_get_seqnum(p), 2);
	CU_ASSERT_EQUAL(pkt_get_length(p), 3);
	CU_ASSERT_EQUAL(pkt_get_crc(p), 4);
	CU_ASSERT_STRING_EQUAL(pkt_get_payload(p), c);
	
	pkt_del(p);
}

/* Test de real_address si les arguments sont corrects
 *
 */
void test_real_address()
{
	struct sockaddr_in6 *rval = malloc(sizeof(struct sockaddr_in6));
	const char *address = "::1"; //adresse non valide??
	const char *result = real_address(address, rval);
	CU_ASSERT_PTR_EQUAL(result, NULL);
	free(rval);
}

/* Test de real_address si les arguments sont incorrects
 *
 */
void test_real_address1()
{
	struct sockaddr_in6 *rval = NULL;
	const char *address = "1";
	const char *result = real_address(address, rval);
	CU_ASSERT_PTR_NOT_EQUAL(result, NULL);
}

/* Test de create_socket si les arguments sont corrects
 *
 */
void test_create_socket1()
{
	struct sockaddr_in6 *addr = malloc(sizeof(struct sockaddr_in6));
	struct sockaddr_in6 *addr1 = malloc(sizeof(struct sockaddr_in6));
	real_address("::1", addr);
	real_address("::1", addr1);
	struct sockaddr_in6 *src = addr;
	int src_port = 12345;
	struct sockaddr_in6 *dest_addr = addr1;
	int dst_port = 12345;
	int result = create_socket(src, src_port, dest_addr, dst_port);
	CU_ASSERT_NOT_EQUAL(result, -1);
	free(addr);
	free(addr1);
}

/* Test de create_socket si les arguments ne sont pas corrects
 *
 */
void test_create_socket2()
{
	struct sockaddr_in6 *addr = malloc(sizeof(struct sockaddr_in6));
	struct sockaddr_in6 *addr1 = malloc(sizeof(struct sockaddr_in6));
	real_address("vvv", addr);
	real_address("vvv", addr1);
	struct sockaddr_in6 *src = addr;
	int src_port = 12345;
	struct sockaddr_in6 *dest_addr = addr1;
	int dst_port = 12345;
	int result = create_socket(src, src_port, dest_addr, dst_port);
	CU_ASSERT_EQUAL(result, -1);
	free(addr);
	free(addr1);
}



int main()
{
	CU_pSuite pSuite = NULL;
	
	if (CUE_SUCCESS != CU_initialize_registry())
	{
		return CU_get_error();
	}
	
	pSuite = CU_add_suite("Suite de tests", NULL, NULL);
	if(NULL == pSuite)
	{
		CU_cleanup_registry();
		return CU_get_error();
	}
	
	if((NULL == CU_add_test(pSuite,"test de l'initiation d'un packet",testPkt_new)) ||
	   (NULL == CU_add_test(pSuite,"test des get et set", testPkt_getsSets)) ||
	   (NULL == CU_add_test(pSuite, "test de encode", testPkt_encode)) ||
	   (NULL == CU_add_test(pSuite, "test de decode", testPkt_decode)) ||
	   (NULL == CU_add_test(pSuite,"test de real_address",test_real_address)) ||
	   (NULL == CU_add_test(pSuite,"2e test de real_address",test_real_address1)) ||
	   (NULL == CU_add_test(pSuite,"test de create_socket",test_create_socket1)) ||
	   (NULL == CU_add_test(pSuite,"2e test de create_socket avec bugs",test_create_socket2))
	   )
	{
		CU_cleanup_registry();
		return CU_get_error();
		
	}
	
	
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}