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
	Address(uint32_t got) // "got" as in the past of "get" i don't know at this point
		: bus(got >> 16),
		  slot(got >> 11),
		  func(got >> 8),
		  off(got)
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
inline void _setaddr(Address addr) { std::out32(PCI_CONFIG_ADDRESS, addr.get()); }
inline uint32_t _read() { return std::in32(PCI_CONFIG_DATA); }
inline void _write32(uint32_t val) { std::out32(PCI_CONFIG_DATA, val); }

uint32_t read32(Address addr);
uint16_t read16(Address addr);
void write16(Address addr, uint16_t val);
void write32(Address addr, uint32_t val);

void portcpy(void* dst, Address addr, size_t sz);

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
	uint8_t capabilities, reserved0; uint16_t reserved1;
	uint32_t reserved2;
	uint8_t intLine, intPin, minGrant, maxLatency;
} __attribute__((packed));

// Actions
void probe();

extern std::unordered_map<uint32_t, Descriptor0> descriptors;
extern std::unordered_map<uint16_t, std::vector<uint32_t>> classes;



// Higher level
#define MSI_CAPABILITY_ID  0x05
#define MSIX_CAPABILITY_ID 0x11



struct MSICapability {
	uint8_t id, next; uint16_t msgControl;
	uint32_t msgAddrLo;
	uint32_t msgAddrHi;
	uint16_t msgData, reserved;
	uint32_t mask;
	uint32_t pending;
} __attribute__((packed));

#define MSI_CONTROL_OFFSET 0x02
#define MSI_ADDR_OFFSET 0x04
#define MSI_DATA32_OFFSET 0x08
#define MSI_DATA64_OFFSET 0x0C

union MSIControl {
	struct {
		uint16_t enable : 1;
		uint16_t mmc : 3;
		uint16_t mme : 3;
		uint16_t c64 : 1;
		uint16_t pvm : 1;
		uint16_t reserved : 6;
	};
	uint16_t raw;
};

#define MSI_ADDR_BASE_ADDRESS 0xFEE
union MSIAddress {
	struct {
		uint32_t reserved : 2;
		uint32_t destinationMode : 1;
		uint32_t redirectionHint : 1;
		uint32_t reserved1 : 8;
		uint32_t destID : 8; // Which core?
		uint32_t baseAddress : 12; // Must be MSI_ADDR_BASE_ADDRESS
	} __attribute__((packed));

	uint32_t raw;
};

union MSIData {
	struct{
		uint32_t vector : 8;
		uint32_t deliveryMode : 3;
		uint32_t reserved : 3;
		uint32_t level : 1;
		uint32_t triggerMode : 1;
		uint32_t reserved1 : 16;
	} __attribute__((packed));

	uint32_t raw;
};

size_t enableMSI(Address addr, uint8_t vector);



// Public
void exportProcedures();

#endif
