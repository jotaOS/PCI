#include "common.hpp"

uint32_t read32(Address addr) {
	_setaddr(addr);
	return _read();
}

uint16_t read16(Address addr) {
	uint32_t ret = read32(addr);
	if(addr.off & 2)
		ret >>= 16; // Higher half
	return ret;
}

void write32(Address addr, uint32_t val) {
	_setaddr(addr);
	_write32(val);
}

void write16(Address addr, uint16_t val) {
	// Read 32 bits
	uint32_t aux = read32(addr);
	uint32_t val32 = val;
	// Prepare write32
	if(addr.off & 2) {
		// Higher half
		aux &= ~0xFFFF0000;
		aux |= val32 << 16;
	} else {
		// Lower half
		aux &= ~0x0000FFFF;
		aux |= val32;
	}
	// Do it
	_write32(val);
}

// Copy words from a device descriptor
void portcpy(void* dst, Address addr, size_t sz) {
	uint32_t* dst32 = (uint32_t*)dst;

	while(sz) {
		// Copying two words (4 bytes) at a time
		*(dst32++) = read32(addr);
		addr.off += 4;
		sz -= 4;
	}
}

static inline uint16_t getVendorID(Address addr) {
	addr.off = VENDOR_OFFSET;
	return read16(addr);
}

static inline uint8_t getHeaderType(Address addr) {
	addr.off = HEADER_TYPE_OFFSET;
	return read16(addr);
}



// Descriptors for connected devices; key is address
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
