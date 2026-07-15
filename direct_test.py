import os, mmap, struct

BASE_ADDRESS = 0xA0000000  

print("Opening physical memory...")
f = os.open("/dev/mem", os.O_RDWR | os.O_SYNC)
mm = mmap.mmap(f, 4096, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, offset=BASE_ADDRESS)

def write_reg(offset, value):
    mm[offset:offset+4] = struct.pack('<I', value)

print("Configuring the POTA Core registers...")
write_reg(0x04, 16)       # num_keys = 16
write_reg(0x08, 4)        # num_layers = 4
write_reg(0x0C, 0xAA)     # inst0_alpha = 0xAA

# Write dummy seed
for offset in range(0x90, 0xB0, 4):
    write_reg(offset, 0x11111111)

print("Triggering init_pulse!")
write_reg(0x00, 0x01)    # Start the IP!

mm.close()
os.close(f)
print("Done! Look at Vivado!")