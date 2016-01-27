/*
 * ClientHandler.h
 *
 *  Created on: Jan 26, 2016
 *      Author: matt
 */

#ifndef CLIENTHANDLER_H_
#define CLIENTHANDLER_H_

using namespace std;

class ClientHandler {
private:
	int client;
	bool debug;

public:
	ClientHandler(int client, bool debug) : client(client), debug(debug) {}

	void handle() {
		cout << "handled!" << endl;
	}
};

#endif /* CLIENTHANDLER_H_ */
