/*
 * Connection.h
 * Author: Professor Karim Sonh
 * UCSC
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include "Thread.h"
#include "TCPSocket.h"
#include "Interface.h"
// A class representing a single connection form a client
class Connection : public Thread{
	private:
		TCPSocket * tcpSocket; // TCP Socket for communication with client
		Connection * next_connection; // A way to build a linked list of connections for the garbage collector to be able to track them
		Interface * threadInterface;
	public:
		Connection(TCPSocket * p_tcpSocket, Interface * p_threadInterface); // Constructor: Set client connected TCP socket
		void * threadMainBody (void * arg); // Main thread body that serves the connection
		void setNextConnection(Connection * connection); // Set the next pointer: used by the Garbage Collector
		Connection * getNextConnection (); // Get a pointer to the next connection
		~Connection(); // Destructor
};

#endif