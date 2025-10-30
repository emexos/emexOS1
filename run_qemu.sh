#!/bin/bash

ISO="$1"

if [ -z "$ISO" ]; then
    echo "Error: No ISO filename provided"
    exit 1
fi

echo "  [RUN] $ISO"

qemu-system-x86_64 \
    -cdrom "$ISO" \
    -m 256M \
    \
    # -enable-kvm \
    \
    # -device qemu-xhci,id=xhci \
    # -device usb-kbd \
    # -device usb-mouse \
    \
2>/dev/null || \
qemu-system-x86_64 \
    -cdrom "$ISO" \
    -m 256M \
    \
    # -device qemu-xhci,id=xhci \
    # -device usb-kbd \
    # -device usb-mouse \
