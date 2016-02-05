/*
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
