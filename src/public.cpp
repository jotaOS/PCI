#include "common.hpp"
#include <rpc>
#include <cstdio>
#include <userspace/PCI.hpp>
#include <registry>

uint32_t getDevice(std::PID client, size_t classID, size_t subclassID, size_t idx) {
	if(!std::registry::has(client, "PCI_LIST"))
		return std::PCI::BAD_DEVICE;

	uint16_t key = 0;
	key |= (classID & 0xFF) << 8;
	key |= subclassID & 0xFF;

	if(!classes.has(key))
		return std::PCI::BAD_DEVICE;

	if(classes[key].size() <= idx)
		return std::PCI::BAD_DEVICE;

	return classes[key][idx];
}

uint32_t getBAR(std::PID client, size_t addr, size_t idx) {
	if(!std::registry::has(client, "PCI_FULL"))
		return std::PCI::BAD_DEVICE;
	if(!descriptors.has(addr))
		return std::PCI::BAD_DEVICE;
	if(idx > 5)
		return std::PCI::BAD_BAR;

	return descriptors[addr].bar[idx];
}

size_t doMSI(std::PID client, size_t addr, size_t vector) {
	if(!std::registry::has(client, "PCI_FULL"))
		return std::PCI::BAD_DEVICE;
	if(!descriptors.has(addr))
		return std::PCI::BAD_DEVICE;

	return enableMSI(addr, vector);
}

static inline Address getCommand(size_t addr) {
	Address ret = addr;
	ret.off = COMMAND_OFFSET;
	return ret;
}

size_t enableMMIO(std::PID client, size_t addr) {
	if(!std::registry::has(client, "PCI_FULL"))
		return std::PCI::BAD_DEVICE;
	if(!descriptors.has(addr))
		return std::PCI::BAD_DEVICE;

	Address a = getCommand(addr);
	CommandRegister cmd;
	cmd.raw = read16(a);
	cmd.mmio = 1;
	write16(a, cmd.raw);
	return std::PCI::OK;
}

size_t becomeBusmaster(std::PID client, size_t addr) {
	if(!std::registry::has(client, "PCI_FULL"))
		return std::PCI::BAD_DEVICE;
	if(!descriptors.has(addr))
		return std::PCI::BAD_DEVICE;

	Address a = getCommand(addr);
	CommandRegister cmd;
	cmd.raw = read16(a);
	cmd.busmaster = 1;
	write16(a, cmd.raw);
	return std::PCI::OK;
}

void exportProcedures() {
	std::exportProcedure((void*)getDevice, 3);
	std::exportProcedure((void*)getBAR, 2);
	std::exportProcedure((void*)doMSI, 2);
	std::exportProcedure((void*)enableMMIO, 1);
	std::exportProcedure((void*)becomeBusmaster, 1);
}
