#!/usr/bin/env python3
import sys

def elf_to_bin(elf_file, bin_file):
    with open(elf_file, 'rb') as f:
        elf_data = f.read()
    
    # Find the program headers
    e_phoff = int.from_bytes(elf_data[32:36], 'little')
    e_phentsize = int.from_bytes(elf_data[42:44], 'little')
    e_phnum = int.from_bytes(elf_data[44:46], 'little')
    
    binary = bytearray()
    
    for i in range(e_phnum):
        ph_offset = e_phoff + i * e_phentsize
        p_type = int.from_bytes(elf_data[ph_offset:ph_offset+4], 'little')
        
        if p_type == 1:  # PT_LOAD
            p_offset = int.from_bytes(elf_data[ph_offset+4:ph_offset+8], 'little')
            p_filesz = int.from_bytes(elf_data[ph_offset+32:ph_offset+36], 'little')
            binary.extend(elf_data[p_offset:p_offset+p_filesz])
    
    with open(bin_file, 'wb') as f:
        f.write(binary)
    print(f"✅ Converted {elf_file} to {bin_file}")

if __name__ == "__main__":
    elf_to_bin("BMS_Project.elf", "BMS_Project.bin")