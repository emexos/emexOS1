#!/bin/bash

BUILD_FILE=".build"
OUT_FILE="shared/ebuild.h"

# build nummer
if [ -f "$BUILD_FILE" ]; then
    n=$(cat "$BUILD_FILE")
else
    n=0
fi

n=$((n+1))
echo $n > "$BUILD_FILE"

# datum
YEAR=$(date +%y)
MONTH_STR=$(date +%b)

# month mapping
case $MONTH_STR in
    Jan) M="JY" ;;
    Feb) M="FY" ;;
    Mar) M="MH" ;;
    Apr) M="AL" ;;
    May) M="MY" ;;
    Jun) M="JE" ;;
    Jul) M="JY" ;;
    Aug) M="AT" ;;
    Sep) M="SR" ;;
    Oct) M="OR" ;;
    Nov) M="NR" ;;
    Dec) M="DR" ;;
esac

BUILD="${YEAR}${M}${n}"

echo "[BUILDGEN] build = $BUILD"

# header schreiben
cat > "$OUT_FILE" <<EOF
#pragma once
// this file will always be regenerated when building emexOS
// if you want to disable it goto /tools/genbuild.sh
#define ___EMEX_BUILD "$BUILD"
EOF