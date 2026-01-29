#!/bin/bash

FOLDER="dsk/hdd0"
VOLUME_LABEL="EMEXOS"
IMAGE=dsk/disk.img

# Check dependencies
if ! command -v "mcopy" >/dev/null 2>&1; then
    echo "Error: mtools not found. Install with: sudo apt install mtools" >&2
    exit 1
fi

if ! command -v "mkfs.fat" >/dev/null 2>&1; then
    echo "Error: dosfstools not found. Install with: sudo apt install dosfstools" >&2
    exit 1
fi

# Calculate size
if [[ "$OSTYPE" == "darwin"* ]]; then
  SIZE_KB=$(du -sk "$FOLDER" | cut -f1)
else
  SIZE_KB=$(du -s -B1 "$FOLDER" | awk '{print int($1/1024)}')
fi

power_of_two() {
    local n=$1
    local p=1
    while (( p < n )); do
        (( p *= 2 ))
    done
    echo $p
}

SIZE_MB=$(( (SIZE_KB * 12 / 10 + 1023) / 1024 ))
SIZE_MB=$(power_of_two "$SIZE_MB")

if ((SIZE_MB < 64)); then
    SIZE_MB=64
fi

echo "Creating ${SIZE_MB}MB disk image..."

# Remove old image
rm -f "$IMAGE"

# Create empty disk image
dd if=/dev/zero of="$IMAGE" bs=1M count=$SIZE_MB status=none

# Write MBR manually (simplified approach)
# We'll create a simple MBR with one bootable FAT32 partition starting at sector 2048

# MBR Structure:
# 0x000-0x1BD: Boot code (zeros)
# 0x1BE-0x1CD: Partition entry 1
# 0x1CE-0x1DD: Partition entry 2 (unused)
# 0x1DE-0x1ED: Partition entry 3 (unused)
# 0x1EE-0x1FD: Partition entry 4 (unused)
# 0x1FE-0x1FF: Boot signature 0x55 0xAA

TOTAL_SECTORS=$((SIZE_MB * 1024 * 2))
PARTITION_SECTORS=$((TOTAL_SECTORS - 2048))

# Create MBR with Python
python3 <<EOF
import struct

mbr = bytearray(512)

# Partition entry 1 at offset 0x1BE
offset = 0x1BE

# Bootable flag
mbr[offset + 0] = 0x80  # Bootable

# CHS start (not used in LBA, set to 0xFE/0xFF/0xFF)
mbr[offset + 1] = 0xFE
mbr[offset + 2] = 0xFF
mbr[offset + 3] = 0xFF

# Partition type: 0x0C (FAT32 LBA)
mbr[offset + 4] = 0x0C

# CHS end (not used in LBA)
mbr[offset + 5] = 0xFE
mbr[offset + 6] = 0xFF
mbr[offset + 7] = 0xFF

# LBA start (2048 = 0x800)
struct.pack_into('<I', mbr, offset + 8, 2048)

# Partition size in sectors
struct.pack_into('<I', mbr, offset + 12, $PARTITION_SECTORS)

# Boot signature
mbr[0x1FE] = 0x55
mbr[0x1FF] = 0xAA

# Write MBR
with open('$IMAGE', 'r+b') as f:
    f.write(mbr)
EOF

echo "MBR created. Formatting FAT32 partition at sector 2048..."

# Create FAT32 filesystem starting at offset 2048 sectors (1MB)
# We use --offset to tell mkfs.fat where to write within the disk image
mkfs.fat -F 32 -n "$VOLUME_LABEL" -S 512 -s 8 -R 32 --offset 2048 "$IMAGE" >/dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "Error: Failed to create FAT32 filesystem"
    exit 1
fi

echo "Copying files to FAT32 partition..."

# Create temporary mtools config to access partition at offset
MTOOLSRC=$(mktemp)
echo "drive x: file=\"$(pwd)/$IMAGE\" offset=1048576" > "$MTOOLSRC"

# Copy files using mtools
MTOOLSRC="$MTOOLSRC" mcopy -i x: -s "$FOLDER"/* :: 2>/dev/null

if [ $? -ne 0 ]; then
    echo "Warning: Some files may not have been copied"
fi

# Cleanup
rm -f "$MTOOLSRC"

echo ""
echo "✓ Disk image created successfully: $IMAGE"
echo "✓ Size: ${SIZE_MB}MB"
echo "✓ MBR with FAT32 partition at LBA 2048"
echo "✓ Partition type: 0x0C (FAT32 LBA)"
echo ""
