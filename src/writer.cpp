#include <vector>
#include <iostream>
#include <chrono>
#include "main.h"


/**
 * @brief Write 1 GB to disk, then report on writing speed.
 * @param filename Output file path.
 * @param chunkSize Size of the small chunks of written data, in bytes.
 */
void startWriter(std::string filename, unsigned int chunkSize)
{
	const size_t nBlocks = TRANSFER_SIZE / chunkSize;

	std::vector<unsigned char> v(chunkSize);
	for (size_t i = 0; i < chunkSize; i++) {
		v[i] = i % DATA_MODULO;
	}

	auto start = std::chrono::system_clock::now();
	size_t writeBytes = 0;
	FILE* fp = fopen(filename.c_str(), "wb");
	if (!fp) {
		std::cerr << "ERROR opening output file [" << filename << "]" << std::endl;
		return;
	}
	for (unsigned long i = 0; i < nBlocks; i++) {
		writeBytes += fwrite(v.data(), sizeof(unsigned char), v.size(), fp);
	}
	fclose(fp);
	auto end = std::chrono::system_clock::now();

	auto diff = end - start;
	double duration = std::chrono::duration<double, std::milli>(diff).count();
	float writeMB = writeBytes / (1000 * 1000);
	float rate = writeMB / (duration / 1000.0);
	std::cout << "written " << writeMB << " Mbytes in " << duration / 1000 << " s, "
	          << rate << " MB/s, " << rate * 8 << " Mb/s" << std::endl;
}
