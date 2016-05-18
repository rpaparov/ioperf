#include <vector>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include "main.h"


/**
 * @brief Read 1 GB from disk, then report on reading speed.
 * @param filename Input file path.
 * @param chunkSize Size of the small chunks of read data, in bytes.
 */
void startReader(std::string filename, unsigned int chunkSize, bool verify)
{
	std::vector<unsigned char> buffer(chunkSize);
	std::vector<unsigned char> refBuffer(chunkSize);

	for (size_t i = 0; i < chunkSize; i++) {
		refBuffer[i] = i % DATA_MODULO;
	}

	auto start = std::chrono::system_clock::now();
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		std::cerr << "ERROR opening input file [" << filename << "]" << std::endl;
		return;
	}
	uint64_t readBytes = 0;
	uint64_t foundErrors = 0;
	bool doLoop = true;
	while (doLoop) {
		size_t n = fread(buffer.data(), sizeof(unsigned char), buffer.size(), fp);
		readBytes += n;
		if (verify) {
			foundErrors += !std::equal(buffer.begin(), buffer.begin() + n, refBuffer.begin());
		}
		if (n < buffer.size()) {
			doLoop = false;
		}
	}
	fclose(fp);
	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double>(diff).count();
	double readMB = readBytes / (1000 * 1000);
	double rate = readMB / duration;
	std::string job = (verify) ? "verified" : "read";
	std::cout << job << " " << readMB << " MB in " << duration << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s" << std::endl;

	if (foundErrors > 0) {
		std::cerr << "found " << foundErrors << " errors" << std::endl;
	}
}
