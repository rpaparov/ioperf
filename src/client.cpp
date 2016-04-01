#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "main.h"


/**
 * @brief Send data over a TCP or UDP connection, and then quit.
 * @param port TCP port number.
 * @param chunkSize Size of the small chunks of written data, in bytes.
 * @param isUdp Set to true to use UDP instead of TCP.
 */
void startClient(std::string host, unsigned int port, unsigned int chunkSize, double sentGig, bool isUdp)
{
	int type = (isUdp) ? SOCK_DGRAM : SOCK_STREAM;
	int sockfd = socket(AF_INET, type, 0);
	if (sockfd < 0) {
		std::cerr << "ERROR opening socket" << std::endl;
		return;
	}

	struct hostent* server = gethostbyname(host.c_str());
	if (server == NULL) {
		std::cerr << "ERROR, no such host" << std::endl;
		return;
	}

	struct sockaddr_in serv_addr;
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "ERROR connecting" << std::endl;
		return;
	}

	const size_t nBlocks = sentGig * (TRANSFER_SIZE / chunkSize);
	std::vector<unsigned char> buffer(chunkSize);
	
	for (size_t i = 0; i < chunkSize; i++) {
		buffer[i] = i % DATA_MODULO;
	}

	auto start = std::chrono::system_clock::now();
	
	// first let server know a transmission is about to start
	if (isUdp) {
		ssize_t n = write(sockfd, &STX, 1);
		if (n < 0) {
			std::cerr << "ERROR starting transmission, n=" << n << std::endl;
		} else if (n != 1) {
			std::cerr << "ERROR could not send STX, n=" << n << std::endl;
		}
	}

	size_t writeBytes = 0;
	for (size_t i = 0; i < nBlocks; i++) {
		ssize_t n = write(sockfd, buffer.data(), chunkSize);
		if (n < 0) {
			std::cerr << "ERROR writing to socket, n=" << n << ", errno=" << errno << std::endl;
			break;
		} else if (n != (ssize_t)chunkSize) {
			std::cerr << "ERROR could not write full buffer, n=" << n << std::endl;
		}
		writeBytes += n;
		
		if (isUdp) {
			unsigned char bufferIn;
			ssize_t m = read(sockfd, &bufferIn, 1);
			if ((m != 1) or (bufferIn != ACK)) {
				std::cerr << "ERROR could not read acknowledge, n=" << m << std::endl;
			}
		}
	}
	
	// hack for the server to identify the end of transmission, one last write of size 0
	if (isUdp) {
		ssize_t n = write(sockfd, buffer.data(), 0);
		if (n != 0) {
			std::cerr << "ERROR writing final 0 for UDP closure, n=" << n << std::endl;
		}
	}

	close(sockfd);

	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double, std::milli>(diff).count();
	float writeMB = writeBytes / (1000 * 1000);
	float rate = writeMB / (duration / 1000.0);
	unsigned long lostBytes = (sentGig * TRANSFER_SIZE) - writeBytes;
	std::cout << "written " << writeMB << " Mbytes in "
	          << duration / 1000 << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s"
	          << ", lost " << lostBytes << std::endl;
}


