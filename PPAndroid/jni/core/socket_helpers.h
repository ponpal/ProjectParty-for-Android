/*
 * socket_helpers.h
 *
 *  Created on: 12 jul 2014
 *      Author: Lukas
 */

#ifndef SOCKET_HELPERS_H_
#define SOCKET_HELPERS_H_

void socketSetBlocking(int socket, bool value);
void socketSendTimeout(int socket, uint32_t msecs);
void socketRecvTimeout(int socket, uint32_t mescs);
bool socketConnectTimeout(int socket, uint32_t ip, uint16_t port, uint32_t msecs);


#endif /* SOCKET_HELPERS_H_ */
