#!/usr/bin/env python3
"""FAT32 disk inspector - run after make install_disk"""
import struct, sys

DISK = "dsk/disk.img"
PART_LBA = 2048

with open(DISK, 'rb') as f:
    # ---- BPB ----
    f.seek(PART_LBA * 512)
    bpb = f.read(512)

bps = struct.unpack_from('<H', bpb, 11)[0]
spc = struct.unpack_from('<B', bpb, 13)[0]
res = struct.unpack_from('<H', bpb, 14)[0]
nf  = struct.unpack_from('<B', bpb, 16)[0]
fsz = struct.unpack_from('<I', bpb, 36)[0]
rc  = struct.unpack_from('<I', bpb, 44)[0]
hs  = struct.unpack_from('<I', bpb, 28)[0]
sig = struct.unpack_from('<H', bpb, 510)[0]
ftype = bpb[0x52:0x5a].decode('ascii','replace')

print("=== BPB ===")
print(f"  sig=0x{sig:04X}  type='{ftype}'")
print(f"  bytes_per_sector={bps}  sectors_per_cluster={spc}")
print(f"  reserved_sectors={res}  num_fats={nf}  fat_size={fsz}")
print(f"  root_cluster={rc}  hidden_sectors={hs}")

fat_lba  = PART_LBA + res
data_lba = fat_lba + nf * fsz
root_lba = data_lba + (rc - 2) * spc
print(f"  fat_lba={fat_lba}  data_lba={data_lba}  root_dir_lba={root_lba}")

if hs != PART_LBA:
    print(f"  *** ERROR: hidden_sectors={hs} should be {PART_LBA} ***")

# ---- FAT entries 0-8 ----
with open(DISK, 'rb') as f:
    f.seek(fat_lba * 512)
    fat_data = f.read(64)

print("\n=== FAT entries 0-7 ===")
for i in range(8):
    val = struct.unpack_from('<I', fat_data, i*4)[0]
    print(f"  [{i}] 0x{val:08X}")

# ---- Root directory ----
with open(DISK, 'rb') as f:
    f.seek(root_lba * 512)
    root = f.read(spc * 512)

print("\n=== Root directory (SFN entries) ===")
found_bios_sys = False
for i in range(0, len(root), 32):
    e = root[i:i+32]
    if e[0] == 0: break
    if e[0] == 0xe5: continue
    attr = e[11]
    if attr == 0x0f:  # LFN - show name chars
        name_chars = b''
        for off in [1,3,5,7,9, 14,16,18,20,22,24, 28,30]:
            c = e[off:off+2]
            if c != b'\xff\xff' and c != b'\x00\x00':
                name_chars += c
        try:
            nm = name_chars.decode('utf-16-le','replace').replace('\x00','')
            print(f"  LFN: '{nm}'")
        except:
            pass
        continue
    name = e[0:8].decode('ascii','replace').rstrip()
    ext  = e[8:11].decode('ascii','replace').rstrip()
    cls_hi = struct.unpack_from('<H', e, 20)[0]
    cls_lo = struct.unpack_from('<H', e, 26)[0]
    cluster = (cls_hi << 16) | cls_lo
    size = struct.unpack_from('<I', e, 28)[0]
    fname = f"{name}.{ext}" if ext else name
    if 'LIMINE' in name.upper() and 'SYS' in ext.upper():
        found_bios_sys = True
        print(f"  *** {fname} attr=0x{attr:02X} cluster={cluster} size={size} ***")
        # Read first bytes
        if cluster >= 2:
            fdata_lba = data_lba + (cluster-2)*spc
            with open(DISK,'rb') as f:
                f.seek(fdata_lba*512)
                fb = f.read(16)
            print(f"      first bytes: {fb.hex()}")
    else:
        print(f"  {fname} attr=0x{attr:02X} cluster={cluster} size={size}")

if not found_bios_sys:
    print("  *** limine-bios.sys NOT FOUND IN ROOT DIR ***")

# ---- Check /boot dir ----
print("\n=== Checking /boot subdir ===")
boot_cluster = None
for i in range(0, len(root), 32):
    e = root[i:i+32]
    if e[0] == 0: break
    attr = e[11]
    if attr == 0x0f: continue
    name = e[0:8].decode('ascii','replace').rstrip()
    if name.upper() == 'BOOT':
        cls_hi = struct.unpack_from('<H', e, 20)[0]
        cls_lo = struct.unpack_from('<H', e, 26)[0]
        boot_cluster = (cls_hi << 16) | cls_lo
        print(f"  /boot found at cluster={boot_cluster}")
        break

if boot_cluster and boot_cluster >= 2:
    boot_lba = data_lba + (boot_cluster-2)*spc
    with open(DISK,'rb') as f:
        f.seek(boot_lba*512)
        boot_dir = f.read(spc*512)
    for i in range(0,len(boot_dir),32):
        e = boot_dir[i:i+32]
        if e[0] == 0: break
        attr = e[11]
        if attr == 0x0f: continue
        name = e[0:8].decode('ascii','replace').rstrip()
        ext  = e[8:11].decode('ascii','replace').rstrip()
        cls_hi = struct.unpack_from('<H', e, 20)[0]
        cls_lo = struct.unpack_from('<H', e, 26)[0]
        cluster = (cls_hi << 16) | cls_lo
        size = struct.unpack_from('<I', e, 28)[0]
        fname = f"{name}.{ext}" if ext else name
        print(f"  /boot/{fname} attr=0x{attr:02X} cluster={cluster} size={size}")
elif not boot_cluster:
    print("  /boot NOT FOUND")

print("\ndone.")