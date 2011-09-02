#ifndef OFXAMFSERVERH
#define OFXAMFSERVERH

#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/Runnable.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/NObserver.h"
#include "Poco/AutoPtr.h"

#include <vector>
#include <string>

#include "ofxFlashPolicy.h"
#include "ofxAMFConnection.h"
#include "ofMain.h" // for debugging

using Poco::Net::ReadableNotification;
using Poco::NObserver;
using Poco::Thread;
using Poco::Mutex;
using Poco::Runnable;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::SocketReactor;
using Poco::Net::SocketAcceptor;
using std::string;
using std::vector;

template<typename T>
class ofxAMFSocketAcceptor;

class ofxAMFServer {
public:
	ofxAMFServer();
	~ofxAMFServer();
	void setup(string host, int port);
	void addPolicy(string domain, string port);
	void removeClient(ofxAMFConnection* con);
	void addClient(ofxAMFConnection* con);
	bool start();
private: 
	string server_host;
	int server_port;
	ServerSocket* socket;
	SocketReactor* reactor;
	SocketAddress* address;
	ofxAMFSocketAcceptor<ofxAMFConnection>* acceptor;
	vector<ofxFlashPolicy> policies;	
	vector<ofxAMFConnection*> clients;
	Thread thread;
};


template<typename T>
class ofxAMFSocketAcceptor : public SocketAcceptor<T> {
public:
	ofxAMFSocketAcceptor(ServerSocket& socket, SocketReactor& reactor, ofxAMFServer* amfServer)
		:SocketAcceptor<T>(socket,reactor)
		,amf_server(amfServer)
	{
	}
	
	ofxAMFServer* amf_server;
protected:
	virtual T* createServiceHandler(StreamSocket& sock) {
		T* obj = new T(sock, *(SocketAcceptor<T>::reactor()));
		obj->setup(amf_server);
		amf_server->addClient(obj);
		return obj;
	}
};
#endif