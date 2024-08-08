#include "unistd.h"
#include "iostream"
#include <cmath>
#include <cstdio>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include "strings.h"
#include <thread>
#include "vector"
#include "mutex"
#include "arpa/inet.h"
#include "Parsers/HTTPRequest.h"
#include "WebSocket.h"
void handleRequest(const int*, int);
int getStringLength(const char*);
void printRequest(const Request*);
void freeThread(int id);
std::string getAddr(sockaddr_in* addr );


struct WSThread {
	int id;
	std::thread thread;

	WSThread() {
		id = -1;
	}

	WSThread(const int id, const int* fd) {
	 	this->id = id;
	 	thread = std::thread(handleRequest, fd, id );
	}
	//
	// WSThread(const WSThread& wst) {
	// 	id = wst.id;
	// }
	//
	// WSThread& operator=(const WSThread&) {
	// 	return *this;
	// }
};
WSThread* threads;
int tidx = 0, tlen =0;
std::mutex mutex;



void eer(char* msg){
	perror(msg);
	exit(1);
}

[[noreturn]] int main(int argc, char *argv[]){
	WSThread threadsArr[3]; //[std::thread::hardware_concurrency()*62];
	threads = threadsArr;
	tlen = sizeof(threadsArr)/sizeof(WSThread);

	if(argc < 2){
		std::cout<<"Port not Provided";
		exit(1);
	}


	int sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd < 0){
		std::cout<<"Unable to open socket";
		exit(1);
	}
	
	sockaddr_in serv_addr{}, cli_addr{};
	bzero(&serv_addr, sizeof(serv_addr));
	int portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd,(sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
		std::cout<<"Unable to bind to ip and port";
		exit(1);
	}

	listen(sockfd, 5);
	std::cout<<"Listening on port "<<portno<<std::endl;

	unsigned int clilen = sizeof(cli_addr);


	while(true) {
		int newsockfd = accept(sockfd, (sockaddr*) &cli_addr, &clilen);
		if(newsockfd == -1) continue;



		if(tidx < tlen) {
			//std::cout << "No new Thread"<<std::endl;
			threads[tidx] = WSThread(tidx, &newsockfd);
			tidx++;
		}
		else {
			//std::cout << "Reusing Thread"<<std::endl;
			int idx = 0;
			for (; idx < tidx; idx++) {
				if(threads[idx].id == -1) {
					threads[idx].id = idx;

					threads[idx].thread.join();
					threads[idx].thread = std::thread(handleRequest,&newsockfd, idx);
					break;
				}
			}
			if(idx == tidx) {
				close(newsockfd);
			}
		}
	}
}

void handleRequest(const int* newsockfd,int threadId){
	int sockfd = *newsockfd;

	if(sockfd < 0){
		std::cout<<"Failed to accept a connection";
		freeThread(threadId);
		return;
	}
		std::cout<<"\n\n\nConnection Opened. File Descriptor:"<<sockfd<<std::endl;

		char buffer[1024];
		bzero(buffer, sizeof(buffer));

		if(read(sockfd, buffer, sizeof(buffer)) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<sockfd<<". Closing the connection"<<std::endl;
			freeThread(threadId);
			return;
		}		
		
	
		Request request = parseRequest(buffer);
		//printRequest(&request);
		//std::cout<<"ReqStart\n\n\n"<<request.body<<"\n\n\nReqEnd"<<std::endl;

		if(request.headers.find("Sec-WebSocket-Key") != request.headers.end()){

			const WebSocket ws(&sockfd);

			if(ws.upgradeToWebSocket(&request) < 0) {
				std::cout<<"Failed to Upgrade the connection for File Descriptor"<<sockfd<<". Closing the connection"<<std::endl;
				freeThread(threadId);
				return;
			}
			ws.handleWebSocket();
		}else{
			if(write(sockfd, "HTTP/1.1 400 Bad Request\n\n", 17) < 0){
				std::cout<<"Unable to read input steam of File Descriptor"<<sockfd<<". Closing the connection"<<std::endl;
				freeThread(threadId);
				return;
			}
		}
		

		close(sockfd);
		std::cout<<"Connection Closed. File Descriptor:"<<sockfd<<std::endl;
		freeThread(threadId);
}

std::string getAddr(sockaddr_in* addr ) {
	char addrStr[INET_ADDRSTRLEN > INET6_ADDRSTRLEN ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN];
	switch (addr->sin_family) {
		case AF_INET : {
			inet_ntop(AF_INET, &addr->sin_addr,addrStr, INET_ADDRSTRLEN);
			break;
		}
		case AF_INET6: {
			auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(addr);
			inet_ntop(AF_INET6, &ipv6->sin6_addr, addrStr, INET6_ADDRSTRLEN);
			break;
		}
	}
	return addrStr;
}

void freeThread(const int id) {
	mutex.lock();


	for(int idx = 0; idx < tlen; idx++) {
		if(threads[idx].id == id) {
			threads[idx].id = -1;
			break;
		}
	}
	mutex.unlock();
}

void printRequest(const Request* request){
	std::cout<<"Request Protocol: "<<request->protocol<<std::endl;
	std::cout<<"Request Route: "<<request->route<<std::endl;
	std::cout<<"Request Method: "<<request->method<<std::endl;
	std::cout<<"Header Name: "<<std::endl;
	for(const auto&[header, value] : request->headers){
		std::cout<<header<<" |=| "<<value<<std::endl;
	}
	std::cout<<"\n\nRequest body: "<<request->body<<std::endl;
}

int getStringLength(const char* str){
	int len;
	for(len=0; str[len] != 0 && len < sizeof(str); len++){}

	return len;
}


