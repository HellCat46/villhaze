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
struct Request* parseRequest(char* data);
void handleWebSocket(int* sockfd);
int getStringLength(char* str);


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

		// Parsing the Incoming Request
		struct Request* request = parseRequest(buffer);
		std::cout<<"Request Protocol: "<<request->protocol<<std::endl;
		std::cout<<"Request Route: "<<request->route<<std::endl;
		std::cout<<"Request Method: "<<request->method<<std::endl;
		std::cout<<"Header Name: ";
		for(auto header : request->headers){
			std::cout<<header.first<<"="<<header.second<<std::endl;
		}
		std::cout<<"\nRequest body: "<<request->body<<std::endl;

		if(write(*sockfd, "HTTP/1.1 200 OK\n\n", 17) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
			return;
		}
		

		close(*sockfd);
		std::cout<<"Connection Closed. File Descriptor:"<<*sockfd<<std::endl;
}

struct Request* parseRequest(char* data){
	char line[8192];
	int lineidx =0;
	struct Request* request = new struct Request();

	int linecount = 0, isBody = 0;
	for(int idx =0; data[idx] != 0; idx++){
		//std::cout<<(int) data[idx]<<std::endl;
		if(data[idx] == 13) continue;

		if(data[idx] == 10){
			line[lineidx++] = '\0';
			std::string li = line;
			//std::cout<<line<<"\t Len:"<<getStringLength(line)<<std::endl;
			
			
			if(isBody == 1){
				request->body.append(li).append("\n");
			}else if(linecount == 0){
				std::string delimitor = " ";

				request->method = li.substr(0, li.find(delimitor));
				li.erase(0, li.find(delimitor)+1);
				request->route = li.substr(0, li.find(delimitor));
				li.erase(0, li.find(delimitor)+1);
				request->protocol = li.substr(0, li.find(delimitor));
			}else {
				std::string delimitor = ":";

				std::string header = li.substr(0, li.find(delimitor));
				li.erase(0, li.find(delimitor)+1);
				std::string value = li;

				request->headers[header] = value;
			}
			linecount++;

			if(data[idx+1] != 0){
			bzero(&line, sizeof(line));
			lineidx = 0;
			}

			if(data[idx+1] == 10 || data[idx+1] == 13){
				isBody = 1;
			}
		}

		line[lineidx++] = data[idx];
	}

	if(lineidx == 0) return request;

	line[lineidx++] = '\0';
	std::string li = line;
	if(isBody != 1){
		std::string delimitor = ":";
		std::string header = li.substr(0, li.find(delimitor));
		li.erase(0, li.find(delimitor)+1);
		std::string value = li;
		request->headers[header] = value;
	}else {
		request->body.append(li);
	}
	

	return request;
}

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
