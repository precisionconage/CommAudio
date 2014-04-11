#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include <string>
#include <list>
#include <memory>
#include <Winsock2.h>
#include "../bass.h"

class Client
{
public:
	std::list <std::shared_ptr<WSABUF> > bufs;

	// Getter functions
	std::string getName() const;
	SOCKADDR_IN getAddr() const;
	SOCKET      getSock() const;
	SOCKET      getUdpSock() const;

	// Setter functions; uses the builder pattern, so can be called in succession
	Client& setName(const std::string& newName);
	Client& setAddr(const SOCKADDR_IN& newAddr);
	Client& setSock(const SOCKET& newSock);
	Client& setUdpSock(const SOCKET& udpSock);

	// Stream management functions
	HSTREAM getStreamHandle() const;
	bool isInProgress() const;
	void cancelStream();
	void startStream(std::string fileName, QWORD startOffset);

private:
	std::string	  name_;
	SOCKADDR_IN   addr_;
	SOCKET		  tcpSock_;
	SOCKET        udpSock_;
	HSTREAM	      stream_;
	std::string   fileName_;
	bool          transferCancelled_;
};

struct ClientStruct
{
	WSAOVERLAPPED fakeOvr;
	Client *client;
};


#endif