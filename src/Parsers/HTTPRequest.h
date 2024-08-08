#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <map>


struct Request {
	std::string method, route, protocol, body;
	std::map<std::string, std::string> headers;
};

Request parseRequest(std::string req){
	Request request;
	const std::string delim = "\r\n";
	

	// Parses Important Data from First Line
	request.method = req.substr(0, req.find(' '));
	req.erase(0, req.find(' ')+1);
	request.route = req.substr(0, req.find(' '));
	req.erase(0, req.find(' ')+1);
	request.protocol = req.substr(0, req.find(delim));
	req.erase(0, req.find(delim)+delim.size());

	// Parses Headers and Add them to Map
	while(true){
		std::string header = req.substr(0, req.find(':'));
		req.erase(0, req.find(':')+1);

		request.headers[header] = req.substr(0, req.find(delim));
		req.erase(0, req.find(delim)+delim.size());

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

#endif
