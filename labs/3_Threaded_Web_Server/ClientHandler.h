/*
 * ClientHandler.h
 *
 *  Created on: Jan 26, 2016
 *      Author: matt
 */

#ifndef CLIENTHANDLER_H_
#define CLIENTHANDLER_H_

#include <sstream>

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "DirectoryListing.h"
#include "HttpMessage.h"

#define BUFFER_SIZE 0x1000

using namespace std;

class ClientHandler {
private:
	int client;
	string root;
	bool debug;

	char* buffer;

	bool tryRetrieveRequest(HttpRequest& request) {
		string lines;
		memset(buffer, 0, BUFFER_SIZE + 1);

		while (lines.find("\r\n\r\n") == string::npos &&
				lines.find("\n\n") == string::npos) {
			cout << "recvin'!" << endl;
			int nread = recv(client, buffer, BUFFER_SIZE, 0);
			if (nread < 0) {
				if (errno == EINTR) {
					if (debug) cout << "Interrupted while reading from client socket" << endl;
					continue;
				} else {
					if (debug) perror("tryRetrieveRequest");
					close(client);
					return false; // bad stuffs happened, leaving
				}
			} else if (nread == 0) {
				if (debug) cout << "socket closed prematurely" << endl;
				close(client);
				return false;
			}

			lines.append(buffer, nread);
		}
		cout << "About to parse stuffs!" << endl;
		return HttpMessageParser::tryParseRequestPreamble(lines, request);
	}

	string getExtension(string path) {
		if (path.find(".") != string::npos) {
			return (*(MH::split(path, ".").end() - 1));
		}
		return "";
	}

	void sendNotFound(string path) {
		string body = "<DOCTYPE html>\n<html>\n<head>\n    <title>404: Not Found</title>\n</head>\n<body>\n    <h1>404: Not Found</h1>\n    <p>The resource you have requested was not located</p>\n</body>\n</html>";

		HttpResponse response("HTTP/1.1", 404, "Not Found");
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Content-Length", MH::iota(body.length()));
		response.setBody(body);

		sendResponse(response);
	}

	void sendForbidden(string path) {
		string body = "<DOCTYPE html>\n<html>\n<head>\n    <title>403: Forbidden</title>\n</head>\n<body>\n    <h1>403: Forbidden</h1>\n    <p>The resource you have requested is not publicly visible.</p>\n</body>\n</html>";

		HttpResponse response("HTTP/1.1", 403, "Forbidden");
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Content-Length", MH::iota(body.length()));
		response.setBody(body);

		sendResponse(response);
	}

	void sendBadRequest(string path) {
		string body = "<DOCTYPE html>\n<html>\n<head>\n    <title>400: Bad request</title>\n</head>\n<body>\n    <h1>400: Bad request</h1>\n    <p>The request you made was unviable.</p>\n</body>\n</html>";

		HttpResponse response("HTTP/1.1", 400, "Bad Request");
		response.addHeader("Content-type", "text/html");
		response.addHeader("Content-Length", MH::iota(body.length()));
		response.setBody(body);

		sendResponse(response);
	}

	void sendTextFile(string path) {
		struct stat filestat;

		if (stat(path.c_str(), &filestat)) {
			return;
		}
		int size = filestat.st_size;
		char* buffer = new char[size];
		memset(buffer, 0, size);

		FILE* fp = fopen(path.c_str(), "r");
		fread(buffer, size, 1, fp);
		fclose(fp);

		string body;
		body.append(buffer, size);
		delete[] buffer;

		HttpResponse response("HTTP/1.1", 200, "OK");
		ostringstream mime;
		mime << "text/" << (getExtension(path) == "html" ? "html" : "plain");
		response.addHeader("Content-Type", mime.str());
		response.addHeader("Content-Length", MH::iota(body.length()));
		response.setBody(body);

		sendResponse(response);
	}

	void sendImageFile(string path) {
		struct stat filestat;
		if (stat(path.c_str(), &filestat)) {
			sendBadRequest(path);
		}

		char pBuffer[BUFFER_SIZE];
        memset(pBuffer,0,sizeof(pBuffer));
		sprintf(pBuffer,
				"HTTP/1.1 200 OK\r\n\
				 Content-Type: image/$s\r\n\
				 Content-Length: %d\
				 \r\n\r\n",
				 getExtension(path).c_str(),
				 filestat.st_size);
		write(client,pBuffer, strlen(pBuffer));

		size_t fl = filestat.st_size;
		FILE* fp = fopen(path.c_str(), "r");
		char* buffer = new char[fl + 1];
		fread(buffer, fl, 1, fp);

		write(client, buffer, fl);

		delete[] buffer;
		fclose(fp);
	}

	void sendDirectoryListing(string path) {
		DirectoryListing listing(root, path);
		string body = listing.toString();

		HttpResponse response("HTTP/1.1", 200, "OK");
		response.addHeader("Content-Type", "text/html");
		response.addHeader("Content-Length", MH::iota(body.length()));
		response.setBody(body);

		sendResponse(response);
	}

	void sendResponse(HttpResponse& response) {
		int sent = 0;
	    const char* data = response.toString().c_str();
	    int remaining = response.toString().length();

	    while (remaining > 0) {
	        if ((sent = send(client, data, remaining, 0)) < 0) {
	            if (errno == EINTR) {
	                if (debug) cout << "The socket was interrupted while sending response, continuing.";
	                continue;
	            } else {
	                if (debug) perror("write");
	                close(client);
	                return;
	            }
	        } else if (sent == 0) {
	            if (debug) cout << "The socket was closed while sending response" << endl;
	        	close(client);
	            return;
	        }
	        remaining -= sent;
	        data += sent;
	    }
	}

public:
	ClientHandler(int client, string root, bool debug) : client(client), root(root), debug(debug) {
		buffer = new char[BUFFER_SIZE + 1];
	}

	void handle() {
		cout << "handled!" << endl;
		HttpRequest request;
		if (tryRetrieveRequest(request)) {
			cout << request.toString();
		} else {
			cout << "request couldn't be retrieved" << endl;
			close(client);
			return;
		}

		struct stat filestats;
		string path = root + request.getPath();

		if (stat(path.c_str(), &filestats)) {
			sendNotFound(path);

		} else if ((filestats.st_mode & S_IROTH) == 0) {
			sendForbidden(path);

		} else if (S_ISDIR(filestats.st_mode)) {
			struct stat indexstats;
			if (stat((path + "/index.html").c_str(), &indexstats)) {
				sendDirectoryListing(request.getPath());
			} else {
				sendTextFile(path + "/index.html");
			}

		} else if (S_ISREG(filestats.st_mode)) {
			cout << request.getPath();
			string extension = getExtension(request.getPath());
			cout << "EXTENSION: " << extension << endl;
			if (extension == "jpg" || extension == "gif") {
				sendImageFile(path);
			} else {
				sendTextFile(path);
			}
		} else {
			sendBadRequest(request.getPath());
		}
		close(client);
	}
};

#endif /* CLIENTHANDLER_H_ */

