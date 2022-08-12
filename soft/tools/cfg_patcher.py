#!/usr/bin/env python3

import os
import argparse
from enum import IntFlag
from copy import deepcopy
from ctypes import Structure, c_uint32, c_uint8, Union


CFG_BASE = 0x8008000 - 1024
CF2_MAGIC1 = 0x1e9e10f1
CF2_MAGIC2 = 0x20227a79
SETTINGS_MAGIC_0 = b"\x43\x66\x47\x10"


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

class UF2BlockStructU(Union):
    """Union of an UF2 block and an array of uchar"""
    _fields_ = [
        ("s", UF2BlockStruct),
        ("u", c_uint8 * 512)
    ]


class CF2Struct(Structure):
    """Just an CF2 block for magic compare"""
    _fields_ = [
        ("magic1", c_uint32),
        ("magic2", c_uint32),
        ("used_entries", c_uint32),
        ("total_entries", c_uint32),
    ]


class CF2Entry(Structure):
    """Just a CF2 entrie structure"""
    _fields_ = [
        ("key", c_uint32),
        ("value", c_uint32),
    ]


class CF2EntryU(Union):
    """Union of a CF2Entry and a table of unsigned char"""
    _fields_ = [
        ("s", CF2Entry),
        ("u", c_uint8 * 8)
    ]


class UF2Block():
    """Just an UF2Block class"""
    def __init__(self, buffer):
        """Inits the brand new instance"""
        self.c_struct = UF2BlockStructU.from_buffer_copy(buffer)
        # Check the magics
        assert self.c_struct.s.magicStart0 == 0x0A324655
        assert self.c_struct.s.magicStart1 == 0x9E5D5157

    def get_cf2_offset(self):
        """Returns the CF2 offset if found in the payload"""
        for i in range (0, self.c_struct.s.payloadSize, 4):
            cf2_header = CF2Struct.from_buffer_copy(bytes(self.c_struct.s.data[i:]))
            if (cf2_header.magic1 == CF2_MAGIC1) and \
               (cf2_header.magic2 == CF2_MAGIC2):
                return i
        return None


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
        self.parse_cf2_data()

    def get_first_cf2(self):
        """Returns the first uf2_block index that conains CF2 headers"""
        for i, uf2_block in enumerate(self.chunks):
            offset = uf2_block.get_cf2_offset()
            if offset is not None:
                return i, offset
        return None, None

    def export(self, path):
        """Saves back the current UF2 chunks into UF2 """
        with open(path, "wb") as f:
            for chunk in self.chunks:
                f.write(bytes(chunk.c_struct.u))

    def parse_cf2_data(self):
        """Retrieve CF2 data from this UF2 file"""
        index, offset = self.get_first_cf2()
        print(F"CONFIG found @ index:{index}, offset:{offset}", end="")
        print(" ==> 0x{0:x}".format(self.chunks[index].c_struct.s.targetAddr + offset))
        uf2_first_chunk = self.chunks[index]
        cf2_header = CF2Struct.from_buffer_copy(bytes(uf2_first_chunk.c_struct.s.data[offset:]))

        # Config usually spans over multiple uf2 blocks
        self.cf2_used_entries = cf2_header.used_entries
        self.cf2_total_entries = cf2_header.total_entries
        self.cf2 = {}
        current_index = index
        current_offset = offset + 2 * 8
        current_entries = 0
        for _ in range(self.cf2_used_entries):
            data = self.chunks[current_index].c_struct.s.data[current_offset:current_offset+8]
            entry = CF2Entry.from_buffer_copy(bytes(data))
            self.cf2[entry.key] = {"index": current_index,
                                   "offset": current_offset,
                                   "value": entry.value}
            self.max_index = current_index
            self.max_offset = current_offset
            current_entries += 1
            current_offset += 8
            if current_entries == self.chunks[current_index].c_struct.s.payloadSize:
                current_index += 1
                current_offset = 0

    def __iter__(self):
        """Iterate over self.chunks"""
        return iter(self.chunks)

    def dump_cf2(self):
        """Just dump the CF2 config"""
        print(F"{len(uf2.cf2)} entries found")
        for k, v in uf2.cf2.items():
            print(F"{k}: 0x{v['value']:X} ({v['index']}@{v['offset']})")

    def patch(self, vals):
        """Patch the given uf2 with the patch dict"""
        for k, v in vals.items():
            if k in self.cf2:
                # Just patch
                index = self.cf2[k]["index"]
                offset = self.cf2[k]["offset"]
            else:
                # Add if possible
                if self.cf2_used_entries >= self.cf2_total_entries:
                    raise RuntimeError("No room left in cf2 for new content")
                if self.max_offset == self.chunks[max_index].payloadSize:
                    index = self.max_index + 1
                    seld.max_index = index
                    offset = 0
                else:
                    offset = self.max_offset + 8
                    seld.max_offset = offset
            entry = CF2EntryU()
            entry.s.key = k
            entry.s.value = v
            self.cf2[k] = {"index": index, "offset": offset, "value": v}
            # Write v at the right index/offset
            print(self.chunks[index].c_struct.s.data[offset:offset+8] )
            self.chunks[index].c_struct.s.data[offset:offset+8] = entry.u
            print(self.chunks[index].c_struct.s.data[offset:offset+8] )


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", action="store_true")
    parser.add_argument("uf2_file")
    args = parser.parse_args()

    uf2 = UF2(args.uf2_file)
    uf2.dump_cf2()

    if args.p:
        uf2.patch({100: 0x07})
        uf2.dump_cf2()

        dest_file = os.path.splitext(args.uf2_file)[0] + "_patched.uf2"
        uf2.export(dest_file)
