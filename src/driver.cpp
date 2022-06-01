#include "common.hpp"
#include <unordered_map>
#include <cstdio>

static uint16_t readWord(Address addr) {
	_write(addr);
	uint32_t ret = _read();
	ret >>= (addr.off & 2) * 8;
	return ret;
}

static inline uint32_t readTwoWords(Address addr) {
	_write(addr);
	return _read();
}

static void portcpy(void* dst, Address addr, size_t sz) {
	uint32_t* dst32 = (uint32_t*)dst;

	while(sz) {
		// Copying two words (4 bytes) at a time
		*(dst32++) = readTwoWords(addr);
		addr.off += 4;
		sz -= 4;
	}
}

static inline uint16_t getVendorID(Address addr) {
	addr.off = VENDOR_OFFSET;
	return readWord(addr);
}

static inline uint8_t getHeaderType(Address addr) {
	addr.off = HEADER_TYPE_OFFSET;
	return readWord(addr);
}



std::unordered_map<uint32_t, Descriptor0> descriptors;

static Descriptor0 readDescriptor(Address addr) {
	// Get the CommonDescriptor
	auto headerType = getHeaderType(addr) & ~(1 << 7);
	if(headerType != 0x0) {
		// Ignoring like a boss
		// This will have class=0
		return Descriptor0();
	}

	Descriptor0 desc;
	portcpy(&desc, addr, sizeof(Descriptor0));
	return desc;
}

static void checkDevice(uint8_t bus, uint8_t slot) {
	Address addr(bus, slot, 0);
	if(getVendorID(addr) == BAD_VENDOR)
		return; // Nothing over here

	// Function 0 is surely a device
	descriptors[addr.get()] = readDescriptor(addr);

	// More?
	auto headerType = getHeaderType(addr);
	if(headerType & (1 << 7)) {
		// Multi-function device, check all of them
		for(addr.func=1; addr.func<8; ++addr.func) {
			if(getVendorID(addr) != BAD_VENDOR) {
				// Nice
				descriptors[addr.get()] = readDescriptor(addr);
			}
		}
	}
}

void probe() {
	for(size_t bus=0; bus<256; ++bus)
		for(size_t slot=0; slot<32; ++slot)
			checkDevice(bus, slot);
}
