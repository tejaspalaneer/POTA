#pragma once
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define POTA_BASE_ADDR 0xA0000000
#define MAP_SIZE       0x10000 
#define REG_CTRL       0x00
#define REG_STATUS     0x04
#define REG_SEED_BASE  0x08 

class POTAHardware {
private:
    int mem_fd;
    void *mapped_base;
    volatile uint32_t *pota_hw;

public:
    POTAHardware() {
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (mem_fd == -1) {
            perror("[HW Error] Failed to open /dev/mem. Did you run with sudo?");
            exit(EXIT_FAILURE);
        }
        mapped_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, POTA_BASE_ADDR);
        if (mapped_base == MAP_FAILED) {
            perror("[HW Error] Failed to map memory");
            close(mem_fd);
            exit(EXIT_FAILURE);
        }
        pota_hw = (volatile uint32_t *)mapped_base;
        std::cout << "[HW] FPGA Memory Mapped Successfully at 0x" << std::hex << POTA_BASE_ADDR << std::dec << "\n";
    }

    ~POTAHardware() {
        munmap(mapped_base, MAP_SIZE);
        close(mem_fd);
    }

    void write_register(uint32_t offset, uint32_t value) {
        pota_hw[offset / 4] = value; 
    }

    uint32_t read_register(uint32_t offset) {
        return pota_hw[offset / 4];
    }
};
