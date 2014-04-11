/**
 * Declares all functions related to the server side of the transfer.
 * @file ServerTransfer.h
 */

#ifndef SERVER_TRANSFER_H
#define SERVER_TRANSFER_H

#include <list>
#include <vector>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <conio.h>
#include <cstdint>
#include "../TransferProps.h"
#include "../Utils.h"
#include "Client.h"

#define UDP_MAXPACKET	65000	/**< The maximum UDP datagram size */

struct ClientStruct
{
	WSAOVERLAPPED fakeOvr;
	Client *client;
};

// Initialise the server and prepare to serve the client
BOOL ServerInitExtendedFuncs();
BOOL ServerInitSocket(LPTransferProps props);
DWORD WINAPI Serve(VOID *pProps);

// Listen for connection attempts (TCP) and data (UDP)
BOOL ListenTCP(LPTransferProps props);
BOOL ListenUDP(LPTransferProps props);


VOID CALLBACK ServerUniSendCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
	LPOVERLAPPED lpOverlapped, DWORD dwFlags);
VOID CALLBACK RecvNameCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped,
	DWORD dwFlags);

// Prepare server for next volley of packets
VOID ServerCleanup(LPTransferProps props);

// Wrappers for the extended functions (AcceptEx and GetAcceptexSockaddrs)
INT OverlappedAccept(SOCKET listenSocket, SOCKET newSock, PVOID lpNameLen, DWORD dwRecvDataLen, LPDWORD lpdwBytesReceived,
						LPOVERLAPPED lpOverlapped);
INT GetAcceptResults(void *resultBuf, uint32_t *nameLen, sockaddr_in *clientInfo);

VOID CALLBACK UnicastFileSend(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped, DWORD dwFlags);
#endif
