#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include "main.h"


/**
 * @brief Listen for a TCP connection, accept data, and then quit.
 * @param port TCP port number.
 * @param filename If not empty (""), then output input data to file.
 * @param chunkSize Size of the small chunks of read data, in bytes.
 */
void startServerTcp(unsigned int port, std::string filename, unsigned int chunkSize)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "ERROR opening TCP socket" << std::endl;
		return;
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
		std::cerr << "ERROR while setting socket options" << std::endl;
		return;
	}

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	bzero((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "ERROR on TCP binding" << std::endl;
		return;
	}

	listen(sockfd, 5);
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
	if (newsockfd < 0) {
		std::cerr << "ERROR on TCP accept" << std::endl;
		return;
	}

	std::cerr << "accepted connection, client is " << inet_ntoa(cli_addr.sin_addr) << ":"
	          << ntohs(cli_addr.sin_port) << std::endl;

	auto start = std::chrono::system_clock::now();

	FILE* fp = NULL;
	if (!filename.empty()) {
		fp = fopen(filename.c_str(), "wb");
		if (!fp) {
			std::cerr << "ERROR opening output file [" << filename << "]" << std::endl;
			return;
		}
	}

	std::vector<unsigned char> buffer(chunkSize);
	uint64_t readBytes = 0;
	uint64_t writeBytes = 0;

	bool doLoop = true;
	while (doLoop) {
		ssize_t n = read(newsockfd, buffer.data(), chunkSize);
		if (n < 0) {
			std::cerr << "ERROR reading from TCP socket" << std::endl;
			break;
		} else if (n == 0) {
			doLoop = false;
		}
		readBytes += n;
		if (fp) {
			size_t m = fwrite(buffer.data(), sizeof(unsigned char), n, fp);
			if (m != (size_t)n) {
				std::cerr << "ERROR writing to file" << std::endl;
				doLoop = false;
			}
			writeBytes += m;
		}
	}

	if (fp != NULL) {
		fclose(fp);
	}
	close(newsockfd);
	close(sockfd);
	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double>(diff).count();
	double readMB = readBytes / (1000 * 1000);
	double writeMB = writeBytes / (1000 * 1000);
	double rate = readMB / duration;
	std::cout << "read/written " << readMB << "/" << writeMB << " MB in " << duration << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s" << std::endl;
}


/**
 * @brief Listen for a UDP connection, accept data, and then quit.
 * @param port UDP port number.
 * @param filename If not empty (""), then output input data to file.
 * @param chunkSize Size of the small chunks of read data, in bytes.
 */
void startServerUdp(unsigned int port, std::string filename, unsigned int chunkSize)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		std::cerr << "ERROR opening UDP socket" << std::endl;
		return;
	}
	
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
		std::cerr << "ERROR while setting socket options" << std::endl;
		return;
	}

	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;

	socklen_t serv_addr_size = sizeof(serv_addr);
	socklen_t cli_addr_size = sizeof(cli_addr);

	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*)&serv_addr, serv_addr_size) < 0) {
		std::cerr << "ERROR on UDP binding" << std::endl;
		return;
	}

	std::vector<unsigned char> buffer(chunkSize);
	size_t readBytes = 0;
	size_t writeBytes = 0;
	bool doLoop = true;
	
	// wait until STX is sent
	ssize_t p = recvfrom(sockfd, buffer.data(), 1, 0, (struct sockaddr*)&cli_addr, &cli_addr_size);
	if ((p != 1) or (buffer[0] != STX)) {
		std::cerr << "ERROR reading STX, n=" << p << ", errno=" << errno << std::endl;
		doLoop = false;
	}// else if 
	//std::cerr << "Reading STX succesfull" << std::endl;
	auto start = std::chrono::system_clock::now();

	FILE* fp = NULL;
	if (!filename.empty()) {
		fp = fopen(filename.c_str(), "wb");
		if (!fp) {
			std::cerr << "ERROR opening output file [" << filename << "]" << std::endl;
			return;
		}
	}

	while (doLoop) {
		//ssize_t n = recvfrom(sockfd, buffer.data(), chunkSize, 0, (struct sockaddr*)&cli_addr, &cli_addr_size);
		ssize_t n = read(sockfd, buffer.data(), chunkSize);

		if (n < 0) {
			std::cerr << "ERROR reading from UDP socket, n=" << n << ", errno=" << errno << std::endl;
			break;
		} else if (n == 0) {
			doLoop = false;
		}
		readBytes += n;
		if (fp) {
			size_t m = fwrite(buffer.data(), sizeof(unsigned char), n, fp);
			if (m != (size_t)n) {
				std::cerr << "ERROR writing to file" << std::endl;
				doLoop = false;
			}
			writeBytes += m;
		}
		ssize_t m = sendto(sockfd, &ACK, 1, 0, (struct sockaddr*)&cli_addr, cli_addr_size);
		//ssize_t m = write(sockfd, &ACK, 1);
		if (m != 1) {
			std::cerr << "ERROR could not send acknowledge, n=" << m << ", errno=" << errno << std::endl;
		}
	}

	if (fp != NULL) {
		fclose(fp);
	}
	close(sockfd);

	auto end = std::chrono::system_clock::now();
	auto diff = end - start;

	double duration = std::chrono::duration<double, std::milli>(diff).count();
	float readMB = readBytes / (1000 * 1000);
	float writeMB = writeBytes / (1000 * 1000);
	float rate = readMB / (duration / 1000.0);
	unsigned long lostReadBytes = TRANSFER_SIZE - readBytes;
	unsigned long lostWriteBytes = TRANSFER_SIZE - writeBytes;
	
	float lostReadPercent  = (100.0 * lostReadBytes) / TRANSFER_SIZE;
	float lostWritePercent = (100.0 * lostWriteBytes) / TRANSFER_SIZE;

	std::cout << "read/written " << readMB << "/" << writeMB << " Mbytes in "
	          << duration / 1000 << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s"
	          << ", lostRead " << lostReadBytes << " (" << lostReadPercent << "%)"
	          << ", lostWrite " << lostWriteBytes << " (" << lostWritePercent << "%)"
	          << ", client " << inet_ntoa(cli_addr.sin_addr) << ":"
	          << ntohs(cli_addr.sin_port) << std::endl;
}
