#include "common.hpp"
#include <rpc>
#include <cstdio>
#include <userspace/PCI>

// TODO: Them locks?

uint32_t getDevice(std::PID client, size_t classID, size_t subclassID, size_t idx) {
	IGNORE(client);

	uint16_t key = 0;
	key |= (classID & 0xFF) << 8;
	key |= subclassID & 0xFF;

	if(classes.find(key) == classes.end())
		return std::PCI::BAD_DEVICE;

	if(classes[key].size() <= idx)
		return std::PCI::BAD_DEVICE;

	return classes[key][idx];
}

uint32_t getBAR(std::PID client, size_t addr, size_t idx) {
	IGNORE(client);

	if(descriptors.find(addr) == descriptors.end())
		return std::PCI::BAD_DEVICE;
	if(idx > 5)
		return std::PCI::BAD_BAR;

	return descriptors[addr].bar[idx];
}

size_t doMSI(std::PID client, size_t addr, size_t vector) {
	IGNORE(client);

	if(descriptors.find(addr) == descriptors.end())
		return std::PCI::BAD_DEVICE;

	return enableMSI(addr, vector);
}

void exportProcedures() {
	std::exportProcedure((void*)getDevice, 3);
	std::exportProcedure((void*)getBAR, 2);
	std::exportProcedure((void*)doMSI, 2);
}
