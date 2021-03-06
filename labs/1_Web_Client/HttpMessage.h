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
#include <vector>

using namespace std;

class HttpResponse {
private:
	string protocol;
	int responseCode;
	string responseMessage;

	map<string, vector<string> > headers;

	string body;

	void parseProtocolLine(string line) {
		protocol = line.substr(0, line.find(" "));
		line = line.substr(line.find(" ") + 1);
		responseCode = atoi(line.substr(0, line.find(" ")).c_str());
		line = line.substr(line.find(" ") + 1);
		responseMessage = line;

	}

	void parseHeaderLine(string headerLine) {
		int colon = headerLine.find(":");
		string key = headerLine.substr(0, colon);
		string list = headerLine.substr(colon + 1);
		headers[key] = split(list);
	}

	vector<string> split(string values) {
		vector<string> v;

		unsigned int semi;
		string s;
		while ((semi = values.find(";")) != string::npos) {
			v.push_back(trim(values.substr(0, semi)));
			values = values.substr(semi + 1);
		}
		v.push_back(trim(values));

		return v;
	}

	string ltrim(string s) {
		unsigned int i = 0;
		while (i < s.length() && isspace(s[i])) i++;
		return s.substr(i);
	}

	string rtrim(string s) {
		int i = s.length() - 1;
		while (i >= 0 && isspace(s[i])) i--;
		return s.substr(0, i + 1);
	}

	string trim(string s) { return ltrim(rtrim(s)); }

	void parseHeaders(string headers) {
		istringstream in(headers);

		string input;
		getline(in, input);
		parseProtocolLine(input);

		while (getline(in, input) && trim(input) != "") {
			parseHeaderLine(input);
		}
	}

public:
	HttpResponse() {
		protocol = "HTTP/1.0";
		responseCode = 418;
		responseMessage = "I'm a little teapot";

		headers["Content-Length"] = { "0" };

		body = "";
	}

	HttpResponse(string headers) {
		parseHeaders(headers);
	}

	HttpResponse(string protocol, int responseCode, string responseMessage) {
		this->protocol = protocol;
		this->responseCode = responseCode;
		this->responseMessage = responseMessage;

		body = "";
	}

	string toString() {
		ostringstream out;
		out << protocol << " " << responseCode << " " << responseMessage << endl;
		for (pair <string, vector<string> > p : headers) {
			out << p.first << ": ";
			for (unsigned int i = 0; i < p.second.size(); i++) {
				out << p.second[i];
				if (i != p.second.size() - 1) out << "; ";
			}
			out << endl;
		}
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

	void addHeader(string key, string value) {
		if (headers.count(key) == 0) {
			headers[key] = vector<string>();
		}
		headers[key].push_back(value);
	}

	string getBody() {
		return body;
	}

	void setBody(string body) {
		this->body = body;
	}
};



#endif /* HTTPMESSAGE_H_ */
