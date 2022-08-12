#!/usr/bin/env python3

from ctypes import Structure, c_uint32, c_uint8
from enum import IntFlag
import argparse


CFG_BASE = 0x8008000 - 1024
print("CFG_BASE: {0:X}".format(CFG_BASE))


class UF2Flags(IntFlag):
    """Just Flags for UF2 block"""
    NOT_MAIN_FLASH = 0x00000001
    FILE_CONTAINER = 0x00001000
    FAMILY_ID_PRESENT = 0x00002000
    MD5_PRESENT = 0x00004000
    EXTENTION_TAGS_PRESENT = 0x00008000


class UF2BlockStruct(Structure):
    """Just an UF2 block c style structure"""
    _fields_ = [
        ("magicStart0", c_uint32),
        ("magicStart1", c_uint32),
        ("flags", c_uint32),
        ("targetAddr", c_uint32),
        ("payloadSize", c_uint32),
        ("blockNo", c_uint32),
        ("numBlocks", c_uint32),
        ("fileSize", c_uint32),  # or familyID,
        ("data", c_uint8 * 476),
        ("magicEnd", c_uint32),
    ]


class UF2Block():
    """Just an UF2Block class"""
    def __init__(self, buffer):
        """Inits the brand new instance"""
        self.c_struct = UF2BlockStruct.from_buffer_copy(buffer)
        # Check the magics
        assert self.c_struct.magicStart0 == 0x0A324655
        assert self.c_struct.magicStart1 == 0x9E5D5157


class UF2():
    """Just a simple UF2 class"""
    def __init__(self, path):
        """Just init the brand new instance"""
        self.chunks = []
        with open(path, "rb") as uf2:
            while True:
                chunk = uf2.read(512)
                if not chunk:
                    break
                self.chunks.append(UF2Block(chunk))

    def __iter__(self):
        """Iterate over self.chunks"""
        return iter(self.chunks)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("uf2_file")
    args = parser.parse_args()
    uf2 = UF2(args.uf2_file)
    # find a block that writes at CFG_BASE
    cnt = 0
    for uf2_blk in uf2:
        if cnt > 0:
            cnt += 1
        if uf2_blk.c_struct.flags in [UF2Flags.NOT_MAIN_FLASH,
                                      UF2Flags.FILE_CONTAINER]:
            continue
        print("{0:X}".format(uf2_blk.c_struct.targetAddr))
        if uf2_blk.c_struct.targetAddr != CFG_BASE:
            continue
        print("!")
        cnt += 1
    print(cnt)
    print(cnt * 512)
    print((cnt*512)/1024)

