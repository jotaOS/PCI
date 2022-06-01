#ifndef COMMON_HPP
#define COMMON_HPP

#include <driver>
#include <unordered_map>

// PCI ports
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

struct Address {
	uint8_t bus;
	uint8_t slot;
	uint8_t func;
	uint8_t off;

	Address() = default;
	Address(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off=0)
		: bus(bus), slot(slot), func(func), off(off)
	{}

	inline uint32_t get() const {
		uint32_t ret = 1 << 31;
		ret |= bus << 16;
		ret |= slot << 11;
		ret |= func << 8;
		ret |= off & 0xFC;
		return ret;
	}
};

// Wrappers
inline void _write(Address addr) { std::out32(PCI_CONFIG_ADDRESS, addr.get()); }
inline uint32_t _read() { return std::in32(PCI_CONFIG_DATA); }

struct CommonDescriptor {
	uint16_t vendor, device;
	uint16_t command, status;
	uint8_t revision, prog, subclassCode, classCode = 0;
	uint8_t cls, latency, headerType, BIST;
} __attribute__((packed));

#define VENDOR_OFFSET 0
#define HEADER_TYPE_OFFSET 14

#define BAD_VENDOR 0xFFFF

struct Descriptor0 {
	CommonDescriptor common;
	uint32_t bar[6];
	uint32_t ccis; // Cardbus CIS
	uint16_t subvendor, subsystem;
	uint32_t rom; // Expasion ROM base address
	uint8_t capabilities;
	uint8_t reserved0;
	uint16_t reserved1;
	uint32_t reserved2;
	uint8_t intLine, intPin, minGrant, maxLatency;
} __attribute__((packed));

// Actions
void probe();

extern std::unordered_map<uint32_t, Descriptor0> descriptors;

#endif
