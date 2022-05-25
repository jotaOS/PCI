#include <syscalls>

extern "C" void _start() {
	PID term = std::resolve("term");
	if(term != 0x2)
		std::exit(0x1);

	bochsbrk();
	std::rpc(term, 0, 'X');
	std::rpc(term, 1, 13, 6);

	while(true);
}
