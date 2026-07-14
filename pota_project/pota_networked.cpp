#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <iomanip>
#include <sstream>

#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Common/BitVector.h>
#include <coproto/Socket/AsioSocket.h>
#include <macoro/sync_wait.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtReceiver.h>

// Include our new Hardware Manager
#include "pota_hardware.h"

using namespace osuCrypto;

const std::string SHARED_PATH = "/mnt/hgfs/CHACS Intership/";
const std::string LOCAL_PATH  = "/home/paneer/Desktop/POTA/pota_project/";

using Block256 = std::array<block, 2>;

std::string blockToHex(const Block256& b) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 1; i >= 0; --i) { 
        auto* data = reinterpret_cast<const uint64_t*>(&b[i]);
        ss << std::setw(16) << data[1] << std::setw(16) << data[0];
    }
    return ss.str();
}

// =========================================================
// SENDER LOGIC
// =========================================================
void run_sender(int t, int l, std::string address) {
    std::cout << "=== [SENDER] POTA SETUP ===" << std::endl;
    std::cout << "Listening for Receiver on: " << address << std::endl;
    auto chl = coproto::asioConnect(address, true); 
    
    PRNG sys_prng(sysRandomSeed());
    IknpOtExtSender otSender;
    macoro::sync_wait(otSender.genBaseOts(sys_prng, chl));
    
    std::ofstream binFile(LOCAL_PATH + "sender_seeds.bin", std::ios::binary);
    
    POTAHardware fpga; // Initialize Hardware connection
    uint32_t current_hw_offset = REG_SEED_BASE;

    uint32_t header[2] = {(uint32_t)t, (uint32_t)l};
    binFile.write(reinterpret_cast<char*>(header), sizeof(header));

    for (int i = 0; i < t; ++i) {
        std::cout << "\n--- Instance " << i << " ---" << std::endl;
        Block256 master_root = {sys_prng.get<block>(), sys_prng.get<block>()};
        
        // 1. Save to File
        binFile.write(reinterpret_cast<char*>(&master_root), sizeof(Block256));
        
        // 2. Push to FPGA directly
        uint32_t* raw_data = (uint32_t*)&master_root;
        for(int k=0; k<8; ++k) { // 256 bits = 8 x 32-bit chunks
            fpga.write_register(current_hw_offset, raw_data[k]);
            current_hw_offset += 0x04;
        }

        std::cout << "[+] Master root pushed to FPGA memory." << std::endl;
        
        Block256 dummy_t = {sys_prng.get<block>(), sys_prng.get<block>()};
        macoro::sync_wait(chl.send(dummy_t));
    }
    binFile.close();
    
    std::cout << "[HW] Sending START signal to FPGA..." << std::endl;
    fpga.write_register(REG_CTRL, 0x00000001); 

    std::cout << "\n=== [SUCCESS] Sender Setup Complete ===" << std::endl;
}

// =========================================================
// RECEIVER LOGIC
// =========================================================
void run_receiver(int t, int l, std::string address) {
    std::cout << "=== [RECEIVER] POTA SETUP ===" << std::endl;
    std::cout << "Connecting to Sender at: " << address << std::endl;
    auto chl = coproto::asioConnect(address, false);
    
    PRNG sys_prng(sysRandomSeed());
    IknpOtExtReceiver otReceiver;
    macoro::sync_wait(otReceiver.genBaseOts(sys_prng, chl));

    std::ofstream binFile(LOCAL_PATH + "receiver_seeds.bin", std::ios::binary);
    
    POTAHardware fpga; // Initialize Hardware connection
    uint32_t current_hw_offset = REG_SEED_BASE;

    uint32_t header[2] = {(uint32_t)t, (uint32_t)l};
    binFile.write(reinterpret_cast<char*>(header), sizeof(header));

    for (int i = 0; i < t; ++i) {
        std::cout << "\n--- Instance " << i << " ---" << std::endl;
        uint32_t alpha = sys_prng.get<uint32_t>() % (1 << l);
        
        binFile.write(reinterpret_cast<char*>(&alpha), sizeof(uint32_t));
        uint32_t pad = 0; binFile.write(reinterpret_cast<char*>(&pad), 4);

        // Push Alpha to FPGA
        fpga.write_register(current_hw_offset, alpha);
        current_hw_offset += 0x04;
        fpga.write_register(current_hw_offset, pad);
        current_hw_offset += 0x04;

        Block256 correction_t;
        macoro::sync_wait(chl.recv(correction_t));

        for(int j = 0; j < l; j++){
            Block256 key = {sys_prng.get<block>(), sys_prng.get<block>()}; // Placeholder
            binFile.write(reinterpret_cast<char*>(&key), sizeof(Block256));
            
            // Push sibling keys to FPGA
            uint32_t* raw_data = (uint32_t*)&key;
            for(int k=0; k<8; ++k) {
                fpga.write_register(current_hw_offset, raw_data[k]);
                current_hw_offset += 0x04;
            }
        }
        binFile.write(reinterpret_cast<char*>(&correction_t), sizeof(Block256));
    }
    binFile.close();
    
    std::cout << "[HW] Sending START signal to FPGA..." << std::endl;
    fpga.write_register(REG_CTRL, 0x00000001);

    std::cout << "\n=== [SUCCESS] Receiver Setup Complete ===" << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cout << "Usage: sudo ./pota_networked <role> <t> <l> <ip:port>\n";
        return 1;
    }
    int role = std::stoi(argv[1]);
    int t = std::stoi(argv[2]); 
    int l = std::stoi(argv[3]); 
    std::string address = argv[4];
    
    if (role == 0) run_sender(t, l, address);
    else if (role == 1) run_receiver(t, l, address);
    
    return 0;
}
