#ifndef CLIENTQUEUE_H_
#define CLIENTQUEUE_H_

#include <queue>

#include <pthread.h>

#include "ClientHandler.h"

using namespace std;

/**
 * Thread safe class which manages a queue of active ClientHandler objects
 */
class ClientQueue {
private:
	unsigned int MAX_CONNECTIONS = 100;
	queue<ClientHandler*> clients;
	pthread_mutex_t lock;
	pthread_cond_t notEmpty;
	pthread_cond_t notFull;
	bool debug;
public:
	
	/**
	 * Creates a new ClientQueue with the given settings
	 * 
	 * @param debug whether or not to print debugging information
	 * @param conections explicitly set the number of max connections for this queue
	 */
	ClientQueue(bool debug, unsigned int connections) {
		MAX_CONNECTIONS = connections;
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&notEmpty, NULL);
		pthread_cond_init(&notFull, NULL);
		this->debug = debug;
	}
	~ClientQueue() {
		while (clients.size() > 0) {
			delete clients.front();
			clients.pop();
		}
	}
	
	/**
	 * Thread safe method
	 * Adds the given ClientHandler to the queue.  Waits for the queue to free 
	 * up space if currently full.
	 * 
	 * @param client the ClientHandler to enqueue
	 */
	void enqueue(ClientHandler* client) {
		if (debug) cout << "enqueueing client" << endl;
		pthread_mutex_lock(&lock);
		while (clients.size() == MAX_CONNECTIONS) {
		if (debug) cout << "waiting on free space" << endl;
			pthread_cond_wait(&notFull, &lock);
		}
		if (debug) cout << "about to push client" << endl;
		clients.push(client);
		if (debug) cout << "client pushed" << endl;
		pthread_cond_signal(&notEmpty);
		if (debug) cout << "notEmpty signal sent" << endl;
		pthread_mutex_unlock(&lock);
		if (debug) cout << "queue unlocked" << endl;
	}
	
	/**
	 * Thread safe method
	 * Removes the oldest ClientHandler from the queue and returns it. Waits 
	 * for a client to be added if no clients have been enqueued.
	 * 
	 * @returns a pointer to the ClientHandler that has been waiting the longest
	 */
	ClientHandler* dequeue() {
		if (debug) cout << "dequeuing client" << endl;
		ClientHandler* client;
		pthread_mutex_lock(&lock);
		while (clients.empty()) {
			if (debug) cout << "waiting for  a client to be enqueued" << endl;
			pthread_cond_wait(&notEmpty, &lock);
			if (debug) cout << "waking from sleep" << endl;
		}
		if (debug) cout << "removing client" << endl;
		client = clients.front();
		if (debug) cout << "client fronted" << endl;
		clients.pop();
		if (debug) cout << "client removed" << endl;
		pthread_cond_signal(&notFull);
		pthread_mutex_unlock(&lock);
		return client;
	}
	
	
};

#endif /* CLIENTQUEUE_H_ */
