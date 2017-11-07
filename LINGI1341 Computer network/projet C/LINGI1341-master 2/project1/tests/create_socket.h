//
//  create_socket.h
//  Chat
//
//  Created by Jsday on 28/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#ifndef create_socket_h
#define create_socket_h

#include <netdb.h> //network address and service

/* Creates a socket and initialize it
 * @source_addr: if !NULL, the source address that should be bound to this socket
 * @src_port: if >0, the port on which the socket is listening
 * @dest_addr: if !NULL, the destination address to which the socket should send data
 * @dst_port: if >0, the destination port to which the socket should be connected
 * @return: a file descriptor number representing the socket,
 *         or -1 in case of error (explanation will be printed on stderr)
 */
int create_socket(struct sockaddr_in6 *source_addr,
                  int src_port,
                  struct sockaddr_in6 *dest_addr,
                  int dst_port);

#endif /* create_socket_h */
