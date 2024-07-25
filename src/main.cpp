#include "unistd.h"
#include "iostream"
#include <cstdlib>
#include <cstring>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include "strings.h"
void handleRequest(int*);
struct Request parseRequest(std::string data);
void handleWebSocket(int* sockfd);
int getStringLength(char* str);
void upgradeToWebSocket(int* sockfd);
void printRequest(struct Request request);

struct Request {
	std::string method, route, protocol, body;
	std::map<std::string, std::string> headers;
};



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

		if(read(*sockfd, buffer, sizeof(buffer)) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
			return;
		}

		for(auto buf: buffer){
			std::cout<<(int) buf<<std::endl;
		}
		
	
		struct Request request = parseRequest(buffer);
		//printRequest(request);
		std::cout<<"ReqStart\n\n\n"<<request.body<<"\n\n\nReqEnd"<<std::endl;

		if(write(*sockfd, "HTTP/1.1 200 OK\n\n", 17) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
			return;
		}
		

		close(*sockfd);
		std::cout<<"Connection Closed. File Descriptor:"<<*sockfd<<std::endl;
}

struct Request parseRequest(std::string req){
	struct Request request;
	std::string delim = "\r\n";
	

	// Parses Important Data from First Line
	request.method = req.substr(0, req.find(" "));
	req.erase(0, req.find(" ")+1);
	request.route = req.substr(0, req.find(" "));
	req.erase(0, req.find(" ")+1);
	request.protocol = req.substr(0, req.find(delim));
	req.erase(0, req.find(delim)+delim.size());

	// Parses Headers and Add them to Map
	while(true){
		std::string header = req.substr(0, req.find(":"));
		req.erase(0, req.find(":")+1);
		std::string value = req.substr(0, req.find(delim));
		req.erase(0, req.find(delim)+delim.size());
		request.headers[header] = value;

		//std::cout<<header<<" |=| "<<value<<std::endl;
		if(req.find("\r\n") == 0) break;
	}

	// Parses Body
	req.erase(0, req.find(delim)+delim.length());
	req.append("\0");

	//std::cout<<req<<req.size()<<std::endl;
	request.body = req;


	return request;
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

void upgradeToWebSocket(int* sockfd){}

void handleWebSocket(int* sockfd){
	char buffer[1024];

	while(read(*sockfd, buffer, sizeof(buffer))){
	}

}

int getStringLength(char* str){
	int len = 0;
	for(len=0; str[len] != 0 && len < sizeof(str); len++);

	return len;
}
