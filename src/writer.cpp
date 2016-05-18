#include <vector>
#include <iostream>
#include <chrono>
#include "main.h"


/**
 * @brief Write data to disk, then report on writing speed.
 * @param filename Output file path.
 * @param chunkSize Size of the small chunks of written data, in bytes.
 */
void startWriter(std::string filename, unsigned int chunkSize)
{
	const size_t nBlocks = TRANSFER_SIZE / chunkSize;

	std::vector<unsigned char> buffer(chunkSize);
	for (size_t i = 0; i < chunkSize; i++) {
		buffer[i] = i % DATA_MODULO;
	}

	auto start = std::chrono::system_clock::now();
	uint64_t writeBytes = 0;
	FILE* fp = fopen(filename.c_str(), "wb");
	if (!fp) {
		std::cerr << "ERROR opening output file [" << filename << "]" << std::endl;
		return;
	}
	for (unsigned long i = 0; i < nBlocks; i++) {
		writeBytes += fwrite(buffer.data(), sizeof(unsigned char), buffer.size(), fp);
	}
	fclose(fp);
	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double>(diff).count();
	double writeMB = writeBytes / (1000 * 1000);
	double rate = writeMB / duration;
	std::cout << "written " << writeMB << " MB in " << duration << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s" << std::endl;
}
