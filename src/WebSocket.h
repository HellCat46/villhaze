#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <openssl/sha.h>
#include "Parsers/HTTPRequest.h"
#include "Encoder/base64.h"
#include <sys/socket.h>
#include <iostream>
#include <algorithm>

class WebSocket {
	int* sockfd;

	public:
	WebSocket(int* sockfd){
		this->sockfd = sockfd;
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


		
		unsigned char digest[SHA_DIGEST_LENGTH];
		getSHA1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", digest);
		res.append("Sec-WebSocket-Accept: ").append(getBase64((char*) digest, sizeof(digest)));
		res.append(eol).append(eol);


		//std::cout<<res;

		
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
	private:
		
	void getSHA1(std::string str, unsigned char* digest){

		SHA1((const unsigned char*) (str.c_str()), str.size(), digest);
		
	}
};

#endif
