#include <syscalls>

extern "C" void _start() {
	if(std::resolve("term") == 0x2)
		std::exit(0x42);
	else
		std::exit(0x0);
}
