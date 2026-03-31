#!/usr/bin/env bash
set -euo pipefail

USERNAME_FILE="./dsk/username.txt"

if [ ! -f "$USERNAME_FILE" ]; then
	echo "error: $USERNAME_FILE not found"
	exit 1
fi

#SRC="./dsk/rd"
#OUT="./dsk/initrd.cpio"

USERNAME="$(tr -d ' \n' < "$USERNAME_FILE")"

if [ -z "USERNAME" ]; then
	echo "error: username is empty"
	exit 1
fi

# pick cpio binary
CPIO="cpio"
if [ "$(uname -s)" = "Darwin" ] && command -v gcpio &>/dev/null; then
    CPIO="gcpio" # on macos just install it with brew install cpio
fi

make_initrd(){
	local SRC="$1"
	local OUT="$2"

	if [ ! -d "$SRC" ]; then
	    echo "error: $SRC not found"
	    exit 1
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
}

make_initrd "./dsk/rd" "./dsk/initrd.cpio"

TMP="./.tmp_rdh_build"
rm -rf "$TMP"
mkdir -p "$TMP/home/$USERNAME" # adds the home directory before the username

SRC_USER_DIR="./dsk/rdh/user_id"

if [ ! -d "$SRC_USER_DIR" ]; then
	echo "error: $SRC_USER_DIR not found"
	exit 1
fi

cp -r "$SRC_USER_DIR"/. "$TMP/home/$USERNAME/"

(
	cd "$TMP"
	# ignores macOS files
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
) > "./dsk/initrdh.cpio"

echo "done: ./dsk/initrdh.cpio ($(wc -c < ./dsk/initrdh.cpio | tr -d ' ' ) bytes )"

rm -rf "$TMP"