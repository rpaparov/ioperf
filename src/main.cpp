#include <unistd.h>
#include "main.h"

void startServerTcp(unsigned int port, std::string filename, unsigned int chunkSize);
void startServerUdp(unsigned int port, std::string filename, unsigned int chunkSize);
void startClient(std::string host, unsigned int port, unsigned int chunkSize, double sentGig, bool isUdp);
void startWriter(std::string filename, unsigned int chunkSize);
void startReader(std::string filename, unsigned int chunkSize, bool verify);
void printUsage(char* prog);
Args readArguments(int argc, char** argv);


int main(int argc, char** argv)
{
	Args args = readArguments(argc, argv);

	if (not args.success) {
		printUsage(argv[0]);
		return 1;
	}

	if (args.server) {
		while (true) {
			if (args.udp) {
				startServerUdp(args.port, args.filename, args.chunkSize);
			} else {
				startServerTcp(args.port, args.filename, args.chunkSize);
			}
			usleep(1000);
		}
	} else if (args.client) {
		startClient(args.hostname, args.port, args.chunkSize, args.sentGig, args.udp);
	} else if (args.writer) {
		startWriter(args.filename, args.chunkSize);
	} else {
		startReader(args.filename, args.chunkSize, args.verify);
	}

	return 0;
}


void printUsage(char* prog)
{
	std::string opt = "[-s|-c HOST|-w|-r] [-p PORT] [-n BYTES] [-g GBYTES] [-f PATH] [-hv]";
	std::cerr << "Usage: " << prog << " " << opt << std::endl;
	std::cerr << "Test TCP/UDP and/or disk transfer rate." << std::endl;
	std::cerr << std::endl;
	std::cerr << "Choose only one from:" << std::endl;
	std::cerr << "  -s            start in server mode" << std::endl;
	std::cerr << "  -c HOST       connect client to HOST server" << std::endl;
	std::cerr << "  -w            only write to disk" << std::endl;
	std::cerr << "  -r            only read from disk" << std::endl;
	std::cerr << std::endl;
	std::cerr << "General options:" << std::endl;
	std::cerr << "  -h            show this help and exit" << std::endl;
	std::cerr << "  -n BYTES      size of the transferred/read/written data" << std::endl;
	std::cerr << "                chunks, in bytes" << std::endl;
	std::cerr << "  -g GIGABYTES  size of total tranfered data, in GB (default 1)" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Server, writer and reader options:" << std::endl;
	std::cerr << "  -f PATH       path to file, mandatory for writer and" << std::endl;
	std::cerr << "                reader, optional for server" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Client and server options:" << std::endl;
	std::cerr << "  -p PORT       port used for TCP/UDP communication" << std::endl;
	std::cerr << "  -u            use UDP instead of TCP" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Reader-only options:" << std::endl;
	std::cerr << "  -v            validate data (slower)" << std::endl;
}


Args readArguments(int argc, char** argv)
{
	Args args;

	int opt;
	while ((opt = getopt(argc, argv, "hvswruc:n:f:p:g:")) != -1) {
		switch (opt) {
			case 's':
				args.server = true;
				break;
			case 'c':
				args.client = true;
				args.hostname = optarg;
				break;
			case 'w':
				args.writer = true;
				break;
			case 'r':
				args.reader = true;
				break;
			case 'v':
				args.verify = true;
				break;
			case 'u':
				args.udp = true;
				break;
			case 'n':
				args.chunkSize = atoi(optarg);
				break;
			case 'p':
				args.port = atoi(optarg);
				break;
			case 'f':
				args.filename = optarg;
				break;
			case 'g':
				args.sentGig = atof(optarg);
				break;
			default:
				args.success = false;
		}
	}

	if ((args.server + args.client + args.writer + args.reader) != 1) {
		args.success = false;
	}

	if (args.writer and args.filename == "") {
		args.success = false;
	}

		if (args.reader and args.filename == "") {
		args.success = false;
	}

	return args;
}
