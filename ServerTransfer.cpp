#include "ServerTransfer.h"

// "Global" variables (used only in this file)
static DWORD	recvd	= 0;			// Either the number of bytes or packets received
static WSABUF	wsaBuf;					// Data buffer


DWORD WINAPI Serve(VOID *hwnd)
{
	SOCKADDR_IN		client;
	BOOL			set		= TRUE;
	DWORD			flags	= 0;
	LPTransferProps props	= (LPTransferProps)GetWindowLongPtr((HWND)hwnd, GWLP_TRANSFERPROPS);
	char			buf[UDP_MAXPACKET];

	wsaBuf.buf = buf;
	wsaBuf.len = UDP_MAXPACKET;
	setsockopt(props->socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&set, sizeof(int));

	if (props->nSockType == SOCK_STREAM)
	{
		SOCKET accept;
		if (listen(props->socket, 5) == SOCKET_ERROR)
		{
			MessageBoxPrintf(MB_ICONERROR, TEXT("listen() Failed"), TEXT("listen() failed with socket error %d"), WSAGetLastError());
			return 1;
		}
		if ((accept = WSAAccept(props->socket, NULL, NULL, NULL, NULL)) == SOCKET_ERROR)
		{
			MessageBoxPrintf(MB_ICONERROR, TEXT("WSAAccept Failed"), TEXT("WSAAccept() failed with socket error %d"), WSAGetLastError());
			return 2;
		}
		closesocket(props->socket); // close the listening socket
		props->socket = accept; // assign the new socket to props->socket

		while (1)
		{
			DWORD error, dwSleepRet;
			WSARecv(props->socket, &wsaBuf, 1, NULL, &flags, (LPOVERLAPPED)props,
				TCPRecvCompletion);

			if ((error = WSAGetLastError()) == WSAENOTSOCK)
				break;
			else if (error && error != WSA_IO_PENDING)
			{
				MessageBoxPrintf(MB_ICONERROR, TEXT("WSARecv Error"), TEXT("WSARecv encountered error %d"), error);
				break;
			}

			dwSleepRet = SleepEx(props->dwTimeout, TRUE);
			if (dwSleepRet != WAIT_IO_COMPLETION)
				break; // Something's gone pretty wrong here, since this is TCP
		}
	}
	else
	{
		props->dwTimeout = INFINITE;
		INT client_size = sizeof(client);

		while (1)
		{
			DWORD error, dwSleepRet;
			WSARecvFrom(props->socket, &wsaBuf, 1, NULL, &flags, (sockaddr *)&client, &client_size, (LPOVERLAPPED)props,
				UDPRecvCompletion);
			
			if ((error = WSAGetLastError()) == WSAENOTSOCK)
				break;
			else if (error && error != WSA_IO_PENDING)
			{
				MessageBoxPrintf(MB_ICONERROR, TEXT("WSARecvFrom Error"), TEXT("WSARecvFrom encountered error %d"), error);
				break;
			}
			
			dwSleepRet = SleepEx(props->dwTimeout, TRUE);
			if(dwSleepRet != WAIT_IO_COMPLETION)
				break; // We've lost some packets; just exit the loop
		}
	}

	closesocket(props->socket);
	MessageBoxPrintf(MB_ICONERROR, TEXT("Received"), TEXT("Received %d packets."), recvd);
	DrawTextPrintf((HWND)hwnd, TEXT("Received %d packets."), recvd);
	
	LogTransferInfo("ReceiveLog.txt", props, recvd, GetWindowLongPtr((HWND)hwnd, GWLP_HOSTMODE));

	recvd = 0;
	props->startTime = 0;
	props->endTime = 0;
	props->nPacketSize = 0;
	props->socket = INVALID_SOCKET;
	return 0;
}


BOOL ServerInitSocket(LPTransferProps props)
{
	SOCKET s = WSASocket(AF_INET, props->nSockType, 0, NULL, NULL, WSA_FLAG_OVERLAPPED);

	props->paddr_in->sin_addr.s_addr = htonl(INADDR_ANY);

	if (s == INVALID_SOCKET)
	{
		MessageBoxPrintf(MB_ICONERROR, TEXT("WSASocket Failed"), TEXT("Could not create socket, error %d"), WSAGetLastError());
		return FALSE;
	}

	if (bind(s, (sockaddr *)props->paddr_in, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		MessageBoxPrintf(MB_ICONERROR, TEXT("bind Failed"), TEXT("Could not bind socket, error %d"), WSAGetLastError());
		return FALSE;
	}
	DWORD error = WSAGetLastError();
	props->socket = s;
	return TRUE;
}


VOID CALLBACK UDPRecvCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
	LPOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	LPTransferProps props = (LPTransferProps)lpOverlapped;
	DWORD numExpected;
	if (dwErrorCode != 0)
	{
		MessageBoxPrintf(MB_ICONERROR, TEXT("UDP Recv Error"), TEXT("Error receiving UDP packet; error code %d"), dwErrorCode);
		closesocket(props->socket);
		return;
	}
	time(&props->endTime);
	++recvd;

	props->nNumToSend = ((DWORD *)wsaBuf.buf)[0]; // Hopefully this doesn't get corrupted and happen to equal recvd...
	if (recvd == props->nNumToSend)
	{
		closesocket(props->socket);
		return;
	}

	// This is the first packet
	if (props->dwTimeout == INFINITE)
	{
		props->dwTimeout = COMM_TIMEOUT;
		time(&props->startTime);
	}
}


VOID CALLBACK TCPRecvCompletion(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered,
	LPOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	LPTransferProps props = (LPTransferProps)lpOverlapped;
	if (dwErrorCode != 0)
	{
		MessageBoxPrintf(MB_ICONERROR, TEXT("TCP Recv Error"), TEXT("Error receiving TCP packet; error code %d"), dwErrorCode);
		closesocket(props->socket);
		return;
	}

	if (props->nPacketSize == 0)
	{
		props->nNumToSend	= ((DWORD *)wsaBuf.buf)[0]; // extract the original number to send
		props->nPacketSize	= ((DWORD *)wsaBuf.buf)[1]; // extract the original packet size
	}

	if (dwNumberOfBytesTransfered == 0)
	{
		time(&props->endTime);
		closesocket(props->socket);
		return;
	}
	recvd += dwNumberOfBytesTransfered;
}