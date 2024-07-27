#include "unistd.h"
#include "iostream"
#include <algorithm>
#include <bitset>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include "strings.h"
#include <openssl/sha.h>
void handleRequest(int*);
struct Request parseRequest(std::string data);
void handleWebSocket(int* sockfd);
int getStringLength(char* str);
void upgradeToWebSocket(int* sockfd, struct Request req);
void printRequest(struct Request request);
std::string getSHA1(std::string str);
std::string getBase64(std::string str);
char rfc4648Convertor(int indx);

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
		bzero(buffer, sizeof(buffer));

		if(read(*sockfd, buffer, sizeof(buffer)) < 0){
			std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
			return;
		}		
		
	
		struct Request request = parseRequest(buffer);
		printRequest(request);
		//std::cout<<"ReqStart\n\n\n"<<request.body<<"\n\n\nReqEnd"<<std::endl;

		if(request.headers.find("Sec-WebSocket-Key") != request.headers.end()){
			upgradeToWebSocket(sockfd,request);
		}else{
			if(write(*sockfd, "HTTP/1.1 200 OK\n\n", 17) < 0){
				std::cout<<"Unable to read input steam of File Descriptor"<<*sockfd<<". Closing the connection"<<std::endl;
				return;
			}
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

		std::cout<<header<<" |=| "<<value<<std::endl;
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

void upgradeToWebSocket(int* sockfd, struct Request req){
	
	std::string eol = "\r\n", res;
	res.append("HTTP/1.1 101 Switching Protocols").append(eol);
	res.append("Connection: Upgrade").append(eol);
	res.append("Upgrade: websocket").append(eol);


	std::string key = req.headers.find("Sec-WebSocket-Key")->second; 

	// Trimming Whitespaces from Start and End of String
	key.erase(key.begin(), std::find_if(key.begin(), key.end(), [](unsigned char ch) { return  !std::isspace(ch);}));
	key.erase(std::find_if(key.rbegin(), key.rend(), [](unsigned char ch) { return !std::isspace(ch);}).base(), key.end());

	//std::string value = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	//std::cout<<"\nToken: "<<value<<std::endl;
	//std::string sha1Hashes = getSHA1(value);
	//std::cout<<"SHA1: "<<sha1Hashes<<std::endl;
	//std::string encoded = getBase64(sha1Hashes);
	//std::cout<<"Base64: "<<encoded<<std::endl;

	
	res.append("Sec-WebSocket-Accept: ").append(getBase64(getSHA1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11")));
	res.append(eol).append(eol);


	std::cout<<res;

	write(*sockfd, res.c_str(), res.size());
}

void handleWebSocket(int* sockfd){
	char buffer[1024];
	bzero(buffer,1024);

	while(true){
		if(read(*sockfd, buffer, sizeof(buffer)) < 0) break;

		std::cout<<"Message Received:"<<buffer<<std::endl;

		if(write(*sockfd, "Message Received", 16) < 0){
			std::cout<<"Failed to send message to the Sender";
		}
	}

}

int getStringLength(char* str){
	int len = 0;
	for(len=0; str[len] != 0 && len < sizeof(str); len++);

	return len;
}


std::string getSHA1(std::string str){
	unsigned char digest[SHA_DIGEST_LENGTH];
//	char baseString[str.size()];

//	strcpy(baseString, str.c_str());

	SHA1((const unsigned char*) (str.c_str()), str.size(),(unsigned char*) &digest);

	char hashes[SHA_DIGEST_LENGTH*2+1];

	for(int idx=0; idx < SHA_DIGEST_LENGTH; idx++)
		sprintf(&hashes[idx*2], "%02x", (unsigned int) digest[idx]);

	return hashes;
}

std::string getBase64(std::string str){
	std::string tbytes, enString = "";
	char bytes[7], input[str.size()];
	strcpy(input, str.c_str());

	int bindx =0, remain_bits = 0;


	// Create sextets from the bits of data and convert into base64
	for(int idx = 0; idx < str.size(); idx++){
		int iidx =0;
		if(remain_bits > 0){
			//std::cout<<tbytes;
			iidx = 8-remain_bits;
			while (iidx < 8 && bindx < 6) {
				bytes[bindx++] = tbytes[iidx++];
			}

			remain_bits = 8-iidx;
		}

		if(bindx < 6){
			tbytes = std::bitset<8>(input[idx]).to_string();
			//std::cout<<" "<<input[idx]<<"="<<tbytes<<" "<<"\n";


			iidx= 0;
			int rem= 8-bindx;
			while (iidx < rem && bindx < 6) {
				bytes[bindx++] = tbytes[iidx++];
				
			}

			remain_bits = 8-iidx;
		}else {
			idx--;
		}


		if(bindx == 6){
			bytes[6] = '\0';
			int sextet = std::stoi(bytes, nullptr, 2);
			//std::cout<<bytes<<" "<<sextet<<" "<<rfc4648Convertor(sextet)<<std::endl<<"\n\n";
			enString.insert(enString.size(), 1, rfc4648Convertor(sextet));
			bindx =0;
		}
	}

	// Use the Remaining Bits;
	while(remain_bits != 0) {
		int iidx = 8 - remain_bits;
		while(bindx < 6){
			if(remain_bits !=0){
				bytes[bindx++] = tbytes[iidx++];
				remain_bits--;
			}else bytes[bindx++] = '0';
		}


		bytes[bindx] = '\0';
		int sextets = std::stoi(bytes, nullptr, 2);
		enString.insert(enString.size(), 1, rfc4648Convertor(sextets));
	}
	
	
	while (enString.size() % 4 != 0) {
		enString.append("=");
	}

	return enString;
}


// Function take an integer value and convert into alphabet 
// according to the RFC 4648 Table used for Base64 Encoding
char rfc4648Convertor(int indx){
	if(indx<26){
		return 65+indx;
	}else if(indx<52){
		return 97+(indx-26);
	}else if(indx<62){
		return indx-4;
	}else if(indx == 62) return '+';
	else if (indx == 63) return '/';
	else return -1;
}
