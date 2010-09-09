/*
 * File:   Main.cpp
 * Author: Octavian Sima
 * 
 * Starts server for mobile phone application
 */


#include "Server.h"

int main(int argc, char *argv[]) {
	//if no command line arguments passed, use thise defaults
	int port = 10000;
	bool reverseBytes = true;
	
	if (argc > 1) {
		port = atoi(argv[1]);
	}

	printf("Server started. Listening on port %d\n", port);
	fflush(NULL);

	Server myServer(port, reverseBytes);

	printf("Server, waiting for connection...\n");
	fflush(NULL);
	
	myServer.run();

	return 0;
}

