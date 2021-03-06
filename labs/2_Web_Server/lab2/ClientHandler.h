2_Web_Server/DirectoryListing.h                                                                     0000664 0001750 0001750 00000004131 12652321442 015507  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   /*
 * DirectoryListing.h
 *
 *  Created on: Jan 27, 2016
 *      Author: matt
 */

#ifndef DIRECTORYLISTING_H_
#define DIRECTORYLISTING_H_

#include <sstream>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

class DirectoryListing {
private:
	struct fileInfo {
		bool dir;
		string name;
		int size;
		string date;
	};
	string path;
	string root;
	vector<fileInfo> files;

public:
	DirectoryListing(string root, string path) {
		this->path = path;
		this->root = root;

        DIR* dirp;
        struct dirent* dp;

        dirp = opendir((root + path).c_str());
        while ((dp = readdir(dirp)) != NULL) {
            struct stat filestat;
            string name = dp->d_name;
            cout << dp->d_name << endl;
            string filepath = root + path + (path[path.size() - 1] == '/' ? "" : "/") + name;
            cout << filepath << endl;
            if (stat(filepath.c_str(), &filestat)) {
            	cout << "continue..." << endl;
            	continue;
            }
            cout << "here!" << endl;
            struct fileInfo info;
            info.dir = S_ISDIR(filestat.st_mode);
            info.name = name;
            info.size = filestat.st_size;
            files.push_back(info);
        }
        closedir(dirp);
	}

	string toString() {
		ostringstream out;
		out << "<DOCTYPE html>\n<html>\n<head>\n";
		out << "<title>Index of " << path << "</title>\n";
		out << "<style type=\"text/css\">table {border-collapse: collapse;} td {border-bottom: 1px solid black; padding: 3px 10px;}</style>\n";
		out << "</head>\n</body>\n";
		out << "<h1>Index of " << path << "</h1>\n";
		out << "<table>\n";
		out << "<tr><th>Dir</th><th>Name</th><th>size</th></tr>\n";
		for (struct fileInfo info : files) {
			out << "<tr><td>" << (info.dir ? "dir" : "")<< "</td>";
			out << "<td><a href=\"" << path << (path[path.size() - 1] == '/' ? "" : "/") << info.name << "\">" << info.name << "</a></td>";
			out << "<td>" << info.size << "</td></tr>\n";
		}
		out << "</table>\n</body>\n</html>\n";
		return out.str();
	}
};


#endif /* DIRECTORYLISTING_H_ */
                                                                                                                                                                                                                                                                                                                                                                                                                                       2_Web_Server/HttpMessage.h                                                                          0000664 0001750 0001750 00000011214 12652304646 014444  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   /*
 * HttpMessage.h
 *
 *  Created on: Jan 20, 2016
 *      Author: matt
 */

#ifndef HTTPMESSAGE_H_
#define HTTPMESSAGE_H_

#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "mh_utils.h"

using namespace std;

class Headers {
private:
	map<string, vector<string> > headers;

public:
	Headers() {}
	~Headers() {}

	void set(map<string, vector<string> > other) {
		this->headers = other;
	}

	map<string, vector<string> > get() {
		return headers;
	}

	vector<string>& operator[](string key) {
		return headers[key];
	}

	int count(string key) {
		return headers.count(key);
	}

	string toString() {
		ostringstream out;
		for (pair <string, vector<string> > p : headers) {
			out << p.first << ": ";
			for (unsigned int i = 0; i < p.second.size(); i++) {
				out << p.second[i];
				if (i != p.second.size() - 1) out << "; ";
			}
			out << endl;
		}
		return out.str();
	}
};



class HttpResponse {
private:
	string protocol;
	int responseCode;
	string responseMessage;
	Headers headers;
	string body;

public:
	HttpResponse() {
		protocol = "HTTP/1.0";
		responseCode = 418;
		responseMessage = "I'm a little teapot";

		headers["Content-Length"] = { "0" };

		body = "";
	}

	HttpResponse(string protocol, int responseCode, string responseMessage) : HttpResponse() {
		this->protocol = protocol;
		this->responseCode = responseCode;
		this->responseMessage = responseMessage;
	}

	string toString() {
		ostringstream out;
		out << protocol << " " << responseCode << " " << responseMessage << endl;
		out << headers.toString();
		out << endl;
		out << body;
		return out.str();
	}

	int getContentLength() {
		if (headers.count("Content-Length")){
			return atoi(headers["Content-Length"][0].c_str());
		}
		return 0;
	}

	const string& getBody() const {
		return body;
	}

	void setBody(const string& body) {
		this->body = body;
	}

	const Headers& getHeaders() const {
		return headers;
	}

	void setHeaders(Headers headers) {
		this->headers.set(headers.get());
	}

	const string& getProtocol() const {
		return protocol;
	}

	int getResponseCode() const {
		return responseCode;
	}

	const string& getResponseMessage() const {
		return responseMessage;
	}

	void addHeader(string key, string value) {
		headers[key].push_back(value);
	}
};

class HttpRequest {
private:
	string method;
	string path;
	string protocol;

	Headers headers;

public:
	HttpRequest() {
		method = "GET";
		path = "/";
		protocol = "HTTP/1.1";
	}

	HttpRequest(string method, string path, string protocol) :
		method(method), path(path), protocol(protocol) {}

	~HttpRequest() {}

	string toString() {
		ostringstream out;
		out << method << " " << path << " " << protocol << endl;
		out << headers.toString() << endl;
		return out.str();
	}

	const Headers& getHeaders() const {
		return headers;
	}

	void setHeaders(Headers headers) {
		this->headers.set(headers.get());
	}

	const string& getMethod() const {
		return method;
	}

	const string& getPath() const {
		return path;
	}

	const string& getProtocol() const {
		return protocol;
	}
};

class HttpMessageParser {
private:
	static void parseProtocolLineResponse(string line, HttpResponse& response) {
		string protocol = line.substr(0, line.find(" "));
		line = line.substr(line.find(" ") + 1);
		int responseCode = atoi(line.substr(0, line.find(" ")).c_str());
		line = line.substr(line.find(" ") + 1);
		string responseMessage = line;
		response = HttpResponse(protocol, responseCode, responseMessage);
	}

	static void parseProtocolLineRequest(string line, HttpRequest& request) {
		istringstream in(line);
		string method, path, protocol;
		in >> method >> path >> protocol;
		request = HttpRequest(method, path, protocol);
	}

	static void parseHeaderLine(string headerLine, Headers& headers) {
		int colon = headerLine.find(":");
		string key = headerLine.substr(0, colon);
		string list = headerLine.substr(colon + 1);
		headers[key] = MH::split(list, ";");
	}

	static Headers parseHeaders(istringstream& in) {
		Headers headers;
		string input;

		while (getline(in, input) && MH::trim(input) != "") {
			parseHeaderLine(input, headers);
		}

		return headers;
	}

public:
	static bool tryParseResponsePreamble(string lines, HttpResponse& response) {
		istringstream in(lines);
		string line;
		if(!getline(in, line)) {
			return false;
		}
		parseProtocolLineResponse(line, response);
		response.setHeaders(parseHeaders(in));
		return true;
	}

	static bool tryParseRequestPreamble(string lines, HttpRequest& request) {
		istringstream in(lines);
		string line;
		if(!getline(in, line)) {
			return false;
		}
		parseProtocolLineRequest(line, request);
		request.setHeaders(parseHeaders(in));
		return true;
	}
};

#endif /* HTTPMESSAGE_H_ */
                                                                                                                                                                                                                                                                                                                                                                                    2_Web_Server/mh_utils.h                                                                             0000664 0001750 0001750 00000000633 12652305053 014040  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   /*
 * mh_utils.h
 *
 *  Created on: Jan 27, 2016
 *      Author: matt
 */

#ifndef MH_UTILS_H_
#define MH_UTILS_H_

#include <sstream>
#include <string>
#include <vector>

#include <cctype>

namespace MH {
std::vector<std::string> split(std::string, std::string);
std::string ltrim(std::string);
std::string rtrim(std::string);
std::string trim(std::string);
std::string iota(int);
}


#endif /* MH_UTILS_H_ */
                                                                                                     2_Web_Server/Server.h                                                                               0000664 0001750 0001750 00000003467 12652336150 013474  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   /*
 * Server.h
 *
 *  Created on: Jan 26, 2016
 *      Author: matt
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "ClientHandler.h"

using namespace std;

class Server {
private:
	bool debug;
	unsigned int port;
	const char* dir;

	int server;
	bool serving;

	void create() {
		struct sockaddr_in serverAddress;

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(port);
		serverAddress.sin_addr.s_addr = INADDR_ANY;

		server = socket(PF_INET, SOCK_STREAM, 0);
		if (!server) {
			perror("socket");
			exit(-1);
		}

		int reuse = 1;
		if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
			perror("setsockopt");
			exit(-1);
		}

		if (bind(server, (const struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
			perror("bind");
			exit(-1);
		}

		if (listen(server, SOMAXCONN) < 0) {
			perror("listen");
			exit(-1);
		}

		if (debug) cout << "Socket successfully created! Ready to server!" << endl;
	}

	void serve() {
		while (serving) {
			struct sockaddr_in clientAddress;
			socklen_t clientLength = sizeof(clientAddress);

			int client = accept(server, (struct sockaddr*) &clientAddress, &clientLength);
			if (!client) {
				perror("accept");
				//exit(-1);
			}
			if (debug) cout << "Client accepted!" << endl;
			ClientHandler(client, dir, debug).handle();
			close(client);
		}

		closeServer();
	}

	void closeServer() {

	}

public:
	Server(bool debug, unsigned int port, const char* dir) :
		debug(debug), port(port), dir(dir), server(0), serving(true) {}
	~Server() {}

	void run() {
		create();
		serve();
	}

	void stop() {

	}
};



#endif /* SERVER_H_ */
                                                                                                                                                                                                         2_Web_Server/main.cpp                                                                               0000664 0001750 0001750 00000005363 12652336150 013502  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   #include <iostream>
#include <sstream>
#include <stdexcept>

#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Server.h"
#include "DirectoryListing.h"

using namespace std;

struct args_t {
	bool debug;
	unsigned int port;
	const char* dir;
} args;

void parse_args(int argc, char** argv, struct args_t* args);
void setUpHandlers();

//Handlers
void mainHandler(int status);
void handleSegFault();
void handleInterupt();
void handlePipe();
void handleHangUp();

int main(int argc, char** argv) {

	parse_args(argc, argv, &args);
	if (args.debug) cout << "Arguments successfully parsed, port: " << args.port << ", dir: " << args.dir << endl;

	setUpHandlers();
	if (args.debug) cout << "Handlers created." << endl;

	Server(args.debug, args.port, args.dir).run();

	return 0;
}

void parse_args(int argc, char** argv, struct args_t* args) {
	if (argc < 3 || argc > 4) {
		cout << "USAGE: <port> <dir> [-d]" << endl;
		exit(-1);
	}

	bool debug = false;
	int option;
	while ((option = getopt(argc, argv, "d")) != -1) {
		switch (option) {
		case 'd' :
			debug = true;
			break;
		default:
			cout << "USAGE: <port> <dir> [-d]" << endl;
			exit(-1);
		}
	}

	int port;
	istringstream num(argv[optind]);
	if (!(num >> port) || port < 0) {
		cout << "USAGE: <port> should be a non-negative number, given: " << argv[optind] << endl;
		exit(-1);
	}

	const char* dir = argv[optind + 1];
	struct stat dirstat;

	if (stat(dir, &dirstat)) {
		cout << "ERROR: failed to stat " << dir << endl;
		exit(-1);
	}
	if (!S_ISDIR(dirstat.st_mode) || !(dirstat.st_mode & S_IROTH)) {
		cout << "USAGE: <dir> should be a public directory, given: " << dir << endl;
		exit(-1);
	}

	args->debug = debug;
	args->port = port;
	args->dir = dir;
}

void setUpHandlers() {
	struct sigaction sigold, signew;

	signew.sa_handler = mainHandler;
	sigaction(SIGSEGV, &signew, &sigold);
	sigaction(SIGINT, &signew, &sigold);
	sigaction(SIGPIPE, &signew, &sigold);
	sigaction(SIGHUP, &signew, &sigold);
}


void mainHandler(int status) {
	cout << "status: " << status << endl;
	switch(status) {
	case SIGSEGV:
		handleSegFault();
		break;
	case SIGINT:
		handleInterupt();
		break;
	case SIGPIPE:
		handlePipe();
		break;
	case SIGHUP:
		handleHangUp();
		break;
	default:
		if (args.debug) cout << "Unknown signal received: " << status << endl;
	}
}

void handleSegFault() {
	if (args.debug) cout << "Segmentation fault caught, shutting down." << endl;
	exit(-1);
}

void handleInterupt() {
	if (args.debug) cout << "Interrupt signal caught, pressing forward." << endl;
}

void handlePipe() {
	if (args.debug) cout << "Pipe signal caught, pressing forward." << endl;
}

void handleHangUp() {
	if (args.debug) cout << "Hang up signal caught, pressing forward." << endl;
}
                                                                                                                                                                                                                                                                             2_Web_Server/mh_utils.cpp                                                                           0000664 0001750 0001750 00000001366 12652336150 014401  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   #include "mh_utils.h"

std::vector<std::string> MH::split(std::string values, std::string regex) {
	std::vector<std::string> v;

	int semi;
	std::string s;
	while ((semi = values.find(regex)) != std::string::npos) {
		v.push_back(trim(values.substr(0, semi)));
		values = values.substr(semi + 1);
	}
	v.push_back(trim(values));

	return v;
}

std::string MH::ltrim(std::string s) {
	unsigned int i = 0;
	while (i < s.length() && isspace(s[i])) i++;
	return s.substr(i);
}

std::string MH::rtrim(std::string s) {
	int i = s.length() - 1;
	while (i >= 0 && isspace(s[i])) i--;
	return s.substr(0, i + 1);
}

std::string MH::trim(std::string s) { return ltrim(rtrim(s)); }

std::string MH::iota(int i) {
	std::ostringstream out;
	out << i;
	return out.str();
}
                                                                                                                                                                                                                                                                          2_Web_Server/Makefile                                                                               0000664 0001750 0001750 00000000101 12652336150 013473  0                                                                                                    ustar   matt                            matt                                                                                                                                                                                                                   all:
	g++ -std=c++11 -o server *.cpp
clean:
	rm server nohup.out
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               