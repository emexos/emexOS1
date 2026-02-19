#!/usr/bin/env bash
set -euo pipefail


SRC="./dsk/rd"
OUT="./dsk/initrd.cpio"


if [ ! -d "$SRC" ]; then
    echo "error: $SRC not found"
    exit 1
fi


# pick cpio binaryv
CPIO="cpio"
if [ "$(uname -s)" = "Darwin" ] && command -v gcpio &>/dev/null; then
    CPIO="gcpio" # on macos just install it with brew install cpio
fi

(
    cd "$SRC"
    # -H newc = header is newc (the kernel only supports newc now)
    # -R 0:0 = force uid/gid 0
    find . \
        \( -name ".DS_Store" \
        -o -name "._*" \
        -o -name ".Spotlight-V100" \
        -o -name ".Trashes" \
        -o -name ".fseventsd" \
        -o -name ".TemporaryItems" \
        -o -name ".VolumeIcon.icns" \
        -o -name ".AppleDouble" \) -prune -o -print \
    | sort | "$CPIO" -o -H newc -R 0:0
) > "$OUT"

echo "done: $OUT ($(wc -c < "$OUT" | tr -d ' ') bytes)"
