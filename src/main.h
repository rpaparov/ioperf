#ifndef MAIN_H
#define MAIN_H

#include <getopt.h>
#include <iostream>
#include <string>


const size_t TRANSFER_SIZE = 1000 * 1000 * 1000;
const size_t DATA_MODULO = 100;
const unsigned char STX = 2;
const unsigned char ACK = 6;


struct Args {
	bool success;
	bool server;
	bool client;
	bool writer;
	bool reader;
	bool verify;
	bool udp;
	unsigned int chunkSize;
	unsigned int port;
	float sentGig;
	std::string filename;
	std::string hostname;
	Args() : success(true),
	         server(false), client(false), writer(false), reader(false), verify(false),
	         udp(false), chunkSize(8000), port(7001), sentGig(1),
	         filename(""), hostname("")
	         {}
};


#endif // MAIN_H
