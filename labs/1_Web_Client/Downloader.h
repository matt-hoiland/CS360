/*
 * Downloader.h
 *
 *  Created on: Jan 19, 2016
 *      Author: matt
 */

#ifndef DOWNLOADER_H_
#define DOWNLOADER_H_

#include <iostream>
#include <sstream>

#include <errno.h>      //nope
#include <netdb.h>      //yup
#include <stdio.h>      //yup
#include <stdlib.h>     //yup
#include <string.h>     //yup
#include <sys/types.h>  //yup
#include <sys/socket.h> //yup
#include <unistd.h>     //yup

class Downloader {
private:
	std::string hostname;
	unsigned int port;
	std::string url;
	bool debug;
	unsigned int count;

	int server;

	bool supressBody() {
		return count > 1;
	}

	void create() {
		struct sockaddr_in serverAddress;

		struct hostent* hostEntry;
		hostEntry = gethostbyname(hostname.c_str());
		if (!hostEntry) {
			std::cout << "Err: Couldn't resolve the hostname: " << hostname << "." << std::endl;
			exit(-1);
		}
		std::cout << hostEntry->h_length << std::endl;

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		memcpy(&serverAddress.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

		server = socket(PF_INET, SOCK_STREAM, 0);
		if (!server) {
			std::cout << "Err: Could not allocate a fresh socket" << std::endl;
			exit(-1);
		}

		if (connect(server, (const struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
			std::cout << "Err: Couldn't connect to the address" << std::endl;
			exit(-1);
		}
		std::cout << "So far so good!" << std::endl;
	}

	void download() {
		std::string response;
		if (sendRequest() && retrieveResponse(response)) {
			std::cout << response << std::endl;
		}
	}

	bool sendRequest() {
		std::stringstream reqStream;
		reqStream << "GET " << url << " HTTP/1.0\n\n";
		const char* request = reqStream.str().c_str();

		int remaining = reqStream.str().length();
		int sent;

		while (remaining > 0) {
			if ((sent = send(server, request, remaining, 0)) < 0) {
				if (errno == EINTR) {
					// C'etait interrompu, essaies une fois de plus
					continue;
				} else {
					std::cout << "An error occured while sending the request:" << std::endl;
					std::cout << reqStream.str() << std::endl;
					return false;
				}
			} else if (sent == 0) {
				std::cout << "Server side closed the socket while sending the request" << std::endl;
				return false;
			}

			remaining -= sent;
			request += sent;
		}

		return true;
	}

	unsigned int bufflen;
	char* buffer;

	bool retrieveResponse(std::string& response) {
		response = "";
		while (response.find("\n\n") == std::string::npos) {
			int received = recv(server, buffer, bufflen, 0);
			if (received < 0) {
				if (errno == EINTR) {
					continue;
				} else {
					std::cout << "An error occurred while reading the response" << std::endl;
					return false;
				}
			} else if (received == 0) {
				std::cout << "Server side closed the socket while reading the response:" << std::endl;
				std::cout << response << std::endl;
				return false;
			}
			response.append(buffer, received);
		}
		return true;
	}

	void closeSocket() {
		close(server);
	}

public:
	Downloader(std::string hostname, unsigned int port, std::string url, bool debug, int count) :
		hostname(hostname), port(port), url(url), debug(debug), count(count), server(-1) {
		bufflen = 1024;
		buffer = new char[bufflen];
	}

	void run() {
		create();
		for (unsigned int i = 0; i < count; i++) {
			download();
		}
		closeSocket();
	}
};


#endif /* DOWNLOADER_H_ */
