#!/bin/bash

make clean
#make fetchDeps
make
qemu-system-x86_64 \
    -M q35 \
    -cpu qemu64 \
    -m 512 \
    -drive if=pflash,format=raw,readonly=on,file=uefi/OVMF_CODE.fd \
    -drive if=pflash,format=raw,file=uefi/OVMF_VARS.fd \
    -cdrom build/emexOS.iso \
    -device qemu-xhci,id=xhci \
    -device usb-kbd,bus=xhci.0 \
    -serial stdio \
    -monitor telnet::45454,server,nowait \
    2>&1

# usb mouse
# -device usb-mouse,bus=xhci.0 \



# options:
# -d int,cpu_reset \
# -no-reboot \
