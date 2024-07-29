#include "unistd.h"
#include "iostream"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include "strings.h"
#include <openssl/sha.h>
#include "Parsers/HTTPRequest.h"
#include "WebSocket.h"
void handleRequest(int*);
int getStringLength(char* str);
void printRequest(struct Request request);


void eer(char* msg){
	perror(msg);
	exit(1);
}
int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout<<"Port not Provided";
		exit(1);
	}


	int sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd < 0){
		std::cout<<"Unable to open socket";
		exit(1);
	}
	
	struct sockaddr_in serv_addr, cli_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	int portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0){
		std::cout<<"Unable to bind to ip and port";
		exit(1);
	}

	listen(sockfd, 5);
	std::cout<<"Listening on port "<<portno<<std::endl;

	unsigned int clilen = sizeof(cli_addr);

	while(true) {
		int newsockfd = accept(sockfd, (struct sockaddr*) &serv_addr, &clilen);
		handleRequest(&newsockfd);
	}
}

void handleRequest(int* sockfd){
		if(*sockfd < 0){
			std::cout<<"Failed to accept a connection";
			exit(0);
		}
		std::cout<<"\n\n\nConnection Opened. File Descriptor:"<<*sockfd<<std::endl;

		char buffer[1024];
		bzero(buffer, sizeof(buffer));

		if(read(*sockfd, buffer, sizeof(buffer)) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
			return;
		}		
		
	
		struct Request request = parseRequest(buffer);
		printRequest(request);
		//std::cout<<"ReqStart\n\n\n"<<request.body<<"\n\n\nReqEnd"<<std::endl;

		if(request.headers.find("Sec-WebSocket-Key") != request.headers.end()){

			WebSocket ws = WebSocket(sockfd);
			ws.upgradeToWebSocket(request);
			ws.handleWebSocket();
		}else{
			if(write(*sockfd, "HTTP/1.1 200 OK\n\n", 17) < 0){
				std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
				return;
			}
		}
		

		close(*sockfd);
		std::cout<<"Connection Closed. File Descriptor:"<<*sockfd<<std::endl;
}

void printRequest(struct Request request){
	std::cout<<"Request Protocol: "<<request.protocol<<std::endl;
	std::cout<<"Request Route: "<<request.route<<std::endl;
	std::cout<<"Request Method: "<<request.method<<std::endl;
	std::cout<<"Header Name: "<<std::endl;
	for(auto header : request.headers){
		std::cout<<header.first<<" |=| "<<header.second<<std::endl;
	}
	std::cout<<"\n\nRequest body: "<<request.body<<std::endl;
}



int getStringLength(char* str){
	int len = 0;
	for(len=0; str[len] != 0 && len < sizeof(str); len++);

	return len;
}


