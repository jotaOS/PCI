#include "common.hpp"
#include <cstdio>
#include <userspace/PCI>

static inline bool hasCapabilities(const Descriptor0& desc) {
	return desc.common.status & (1 << 4);
}

static size_t solveMSI(Address addr, uint8_t vector) {
	MSICapability cap;
	portcpy(&cap, addr, sizeof(MSICapability));

	// MSI is a substitute for legacy IRQs through IOAPIC.
	// When a PCI device wants to send an IRQ, it write something to an address.
	// This write is intercepted by the chipset, and with some of the bits
	//   encoded in the address and the data, it knows which LAPIC to send it to.

	// Address to write to on IRQ
	MSIAddress msiaddr;
	msiaddr.baseAddress = MSI_ADDR_BASE_ADDRESS;
	msiaddr.destID = 0; // Always BSP, because I say so
	Address addraddr = addr;
	addraddr.off += MSI_ADDR_OFFSET;
	write32(addraddr, msiaddr.raw);

	// Data to write
	MSIData msidata;
	msidata.vector = vector;
	msidata.deliveryMode = 0;
	Address addrdata = addr;
	if(cap.msgControl & (1 << 7))
		addrdata.off += MSI_DATA64_OFFSET; // 64-bit
	else
		addrdata.off += MSI_DATA32_OFFSET; // 32-bit
	write32(addrdata, msidata.raw);

	// Finally, enable MSI
	MSIControl msictl;
	msictl.raw = cap.msgControl;
	msictl.enable = 1;
	msictl.mme = 0; // Only 1 IRQ
	Address addrctl = addr;
	addrctl.off += MSI_CONTROL_OFFSET;
	write16(addrctl, msictl.raw);

	return std::PCI::OK;
}

size_t enableMSI(Address addr, uint8_t vector) {
	addr.off = 0; // Just in case
	const Descriptor0& desc = descriptors[addr.get()];

	if(!hasCapabilities(desc))
		return std::PCI::NO_CAPABILITIES;

	size_t msi = 0, msix = 0;

	addr.off = desc.capabilities;
	while(true) {
		uint16_t word = read16(addr);
		switch(word & 0xFF) {
		case MSI_CAPABILITY_ID:
			msi = addr.off;
			break;
		case MSIX_CAPABILITY_ID:
			msix = addr.off;
			break;
		}

		addr.off = word >> 8;
		if(!addr.off)
			break;
	}

	if(!msi && !msix) {
		return std::PCI::NO_MSI_OR_MSIX;
	} else if(msi) {
		addr.off = msi;
		return solveMSI(addr, vector);
	} else {
		return std::PCI::MSIX_UNSUPPORTED;
		addr.off = msix;
	}
}
