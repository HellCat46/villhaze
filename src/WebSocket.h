#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <bitset>
#include <cstring>
#include <ostream>
#include <string>
#include <openssl/sha.h>
#include "Parsers/HTTPRequest.h"
#include "Encoder/base64.h"
#include <strings.h>
#include <sys/socket.h>
#include <iostream>
#include <algorithm>

class WebSocket {
	int* sockfd;

	public:
	WebSocket(int* sockfd){
		this->sockfd = sockfd;
	}

	int upgradeToWebSocket(struct Request req){
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
		key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		SHA1((const unsigned char*) (key.c_str()), key.size(), digest);
		res.append("Sec-WebSocket-Accept: ").append(getBase64((char*) digest, sizeof(digest)));
		res.append(eol).append(eol);


		//std::cout<<res;

		
		return write(*sockfd, res.c_str(), res.size());
	}

	void handleWebSocket(){
		char buffer[1024];

		while(true){
			bzero(buffer,1024);
			if(receiveMessage(buffer, sizeof(buffer)) < 0){
				break;
			}

			std::cout<<"Message Received:"<<buffer<<std::endl;


			bzero(buffer,1024);
			if(sendMessage("Message Received", 16) < 0){
				std::cout<<"Failed to send message to the Sender";
			}
		}
	}
	
	// Decodes the Message
	int receiveMessage(char* msg,int len){
		std::string err;
		char data[len+14];
		bzero(data, sizeof(data));

		if(read(*sockfd, data, sizeof(data)) < 0){
			return -1;
		}

		// Checking for message specs using first byte
		if((data[0] & 0b10000000) == 0) err += "Frames are not support. ";
		if((data[0] & 0b01110000) != 0) err += "No Extensions are negotiated upon. ";
		if((data[0] & 0b00000001) == 0)	err += "Only Text Message are supported. ";

		if(err.size() > 0){
			//write(*sockfd, data, data.size()); 
			return -1;
		}



		unsigned long msgLen;
		int strtbytes;
		char msgLenCh[8];


		// Checking size of message and mask status
		switch (data[1] & 0b01111111) {
			case 126: 
				msgLen = std::stoi(std::bitset<8>(data[2]).to_string() + std::bitset<8>(data[3]).to_string(), nullptr, 2);
				strtbytes=4;
				break;
			case 127: {
					std::string bytes;
					for(int idx = 2; idx < 10;idx++){
						bytes.append(std::bitset<8>(data[idx]).to_string());
					}
					msgLen = std::stoi(bytes);
					strtbytes=10;
					break;
				}
			default: msgLen = data[1] & 0b01111111;
				strtbytes=2;
		}
		

		if((data[1] & 0b10000000) != 128){
			err = "Unmasked messages should not be used in Production.";
			if(write(*sockfd, err.c_str(), err.size()) < 0){
				return -1;
			}

			for(int idx = 0; idx < sizeof(data)-strtbytes;idx++){
				msg[idx] = data[idx+strtbytes];
			}
			return 1;
		}

		char masked[4]; 
		for(int idx = 0; idx < 4; idx++){
			masked[idx] = data[idx+strtbytes];
		}

		strtbytes+=4;


		int msgIdx = 0;
		for(int idx= strtbytes; idx < msgLen+strtbytes && idx < sizeof(data);idx++){
			msg[msgIdx] = data[idx] ^ masked[msgIdx%4];
			msgIdx++;
		}

		return 1;
	}

	int sendMessage(char* msg, unsigned long msgLen){
		unsigned char data[14+msgLen]; 


		// Sets Fin, RSV1, RSV2, RSV3 and Opcode
		data[0] = 0b10000001;
		
		// Sets Mask Bit
		data[1] = 0;

		// Set the length of message
		int strtbits;
		if(msgLen < 126){ 
			data[1] += msgLen;

			strtbits = 2;
		}else if(msgLen < 65536){ 
			data[1] += 126;

			std::string len = std::bitset<16>(msgLen).to_string();
			data[2] = std::stoi(len.substr(0,8), nullptr, 2);
			data[3] = std::stoi(len.substr(0,8), nullptr, 2);
			strtbits = 4;
		}else if(msgLen < 18446744073709551615) {
			data[1] += 127;

			std::string len = std::bitset<64>(msgLen).to_string();
			for(int idx = 0; idx < 8;idx++){
				data[1+idx+1] = std::stoi(len.substr(idx*8, 8), nullptr, 2);
			}
			strtbits = 10;
		}else{
			return -1;
		}


		for(int idx=0; idx < msgLen;idx++){
			data[strtbits++] = msg[idx];
		}


		return write(*sockfd, data, strtbits);	
	}
};

#endif
