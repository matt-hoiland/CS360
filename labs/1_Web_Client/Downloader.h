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

#include "HttpMessage.h"

using namespace std;

/**
 * Use 1 socket for each request, regardless of 1.0 or 1.1
 */

class Downloader {
private:
	string hostname;
	unsigned int port;
	string url;
	bool debug;
	unsigned int count;

	int server;

	bool supressBody() {
		return count > 1;
	}

	bool create() {
		struct sockaddr_in serverAddress;

		struct hostent* hostEntry;
		hostEntry = gethostbyname(hostname.c_str());
		if (!hostEntry) {
			cout << "Err: Couldn't resolve the hostname: " << hostname << "." << endl;
			return false;
		}

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		memcpy(&serverAddress.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

		server = socket(PF_INET, SOCK_STREAM, 0);
		if (!server) {
			cout << "Err: Could not allocate a fresh socket" << endl;
			return false;
		}

		if (connect(server, (const struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
			perror("Address");
			return false;
		}

		return true;
	}

	bool download() {
		HttpResponse response;
		if (sendRequest() && retrieveResponse(response)) {
			if (!supressBody() && debug) {
				cout << response.toString() << endl;
			} else if (!supressBody()) {
				cout << response.getBody() << endl;
			}
			return true;
		}
		return false;
	}

	bool sendRequest() {
		stringstream reqStream;
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
					cout << "An error occured while sending the request:" << endl;
					cout << reqStream.str() << endl;
					return false;
				}
			} else if (sent == 0) {
				cout << "Server side closed the socket while sending the request" << endl;
				return false;
			}

			remaining -= sent;
			request += sent;
		}

		return true;
	}

	unsigned int bufflen;
	char* buffer;

	bool retrieveResponse(HttpResponse& response) {
		string headers, remainder;
		if (!retrieveHeaders(headers, remainder)) {
			return false;
		}

		response = HttpResponse(headers);

		if (!retrieveBody(remainder, response.getContentLength())) {
			return false;
		}

		response.setBody(remainder);

		return true;
	}

	bool retrieveBody(string& body, int contentLength) {
		contentLength -= body.length();

		while (contentLength > 0) {
			int received = recv(server, buffer, bufflen, 0);
			if (received < 0) {
				if (errno == EINTR) {
					continue;
				} else {
					cout << "An error occurred while reading the response" << endl;
					return false;
				}
			} else if (received == 0) {
				cout << "Server side closed the socket while reading the response:" << endl;
				return false;
			}
			body.append(buffer, received);
			contentLength -= received;
		}

		return true;
	}

	bool retrieveHeaders(string& headers, string& remainder) {
		headers = "";
		unsigned int splitind;
		while ((splitind = headers.find("\r\n\r\n")) == string::npos) {

			int received = recv(server, buffer, bufflen, 0);
			if (received < 0) {
				if (errno == EINTR) {
					continue;
				} else {
					cout << "An error occurred while reading the response" << endl;
					return false;
				}
			} else if (received == 0) {
				cout << "Server side closed the socket while reading the response:" << endl;
				cout << headers << endl;
				return false;
			}
			headers.append(buffer, received);
		}

		remainder = headers.substr(splitind + 4);
		headers = headers.substr(0, splitind);
		return true;
	}

	void closeSocket() {
		close(server);
	}

public:
	Downloader(string hostname, unsigned int port, string url, bool debug, int count) :
		hostname(hostname), port(port), url(url), debug(debug), count(count), server(-1) {
		bufflen = 1024;
		buffer = new char[bufflen];
	}

	void run() {
		int successes = 0;
		for (unsigned int i = 0; i < count; i++) {
			if (create() && download()) {
				closeSocket();
				successes++;
			}
		}

		if (supressBody()) {
			cout << successes << " out of " << count << " attempts successful!" << endl;
		}
	}
};


#endif /* DOWNLOADER_H_ */
