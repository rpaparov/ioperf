#include <vector>
#include <iostream>
#include <chrono>
#include <algorithm>
#include "main.h"


/**
 * @brief Read 1 GB from disk, then report on reading speed.
 * @param filename Input file path.
 * @param chunkSize Size of the small chunks of read data, in bytes.
 */
void startReader(std::string filename, unsigned int chunkSize, bool verify)
{

	const size_t nBlocks = TRANSFER_SIZE / chunkSize;
	std::vector<unsigned char> v(chunkSize, 0);
	std::vector<unsigned char> vv(chunkSize);

	for (size_t i = 0; i < chunkSize; i++) {
		vv[i] = i % DATA_MODULO;
	}

	auto start = std::chrono::system_clock::now();
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		std::cerr << "ERROR opening input file [" << filename << "]" << std::endl;
		return;
	}
	size_t readBytes = 0;
	size_t foundErrors = 0;
	for (unsigned long i = 0; i < nBlocks; i++) {
		size_t n = fread(v.data(), sizeof(unsigned char), v.size(), fp);
		readBytes += n;
		if (verify) {
			foundErrors += !std::equal(v.begin(), v.begin() + n, vv.begin());
		}
	}
	fclose(fp);
	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double>(diff).count();
	float readMB = readBytes / (1000 * 1000);
	float rate = readMB / duration;
	std::string job = (verify) ? "verified" : "read";
	std::cout << job << " " << readMB << " Mbytes in " << duration << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s" << std::endl;

	if (foundErrors > 0) {
		std::cerr << "found " << foundErrors << " errors" << std::endl;
	}
}
