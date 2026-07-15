import os
import mmap
import struct
import time

# 1. Open the UIO device
f = os.open("/dev/uio0", os.O_RDWR | os.O_SYNC)
mm = mmap.mmap(f, 4096, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, offset=0)

def write_reg(offset, value):
    mm[offset:offset+4] = struct.pack('<I', value)

print("Configuring the POTA Core...")

# 2. Write Configuration Data (Give the state machine something to do!)
write_reg(0x04, 16)       # num_keys = 16 (Offset 8'd1 * 4)
write_reg(0x08, 4)        # num_layers = 4 (Offset 8'd2 * 4)
write_reg(0x0C, 0xAA)     # inst0_alpha = 0xAA (Offset 8'd3 * 4)

# Write a dummy 256-bit seed to the sender (Offsets 0x90 to 0xAC)
write_reg(0x90, 0x11111111)
write_reg(0x94, 0x22222222)
write_reg(0x98, 0x33333333)
write_reg(0x9C, 0x44444444)
write_reg(0xA0, 0x55555555)
write_reg(0xA4, 0x66666666)
write_reg(0xA8, 0x77777777)
write_reg(0xAC, 0x88888888)

print("Triggering init_pulse...")

# 3. Start the IP! (Write 1 to init_pulse at offset 0x00)
write_reg(0x00, 0x01)

print("Command sent! Check Vivado ILA!")

mm.close()
os.close(f)