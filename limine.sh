#!/bin/bash
set -euo pipefail

mkdir -p third_party
rm -rf third_party/limine

git clone https://codeberg.org/Limine/Limine.git --branch=v10.x-binary --depth=1 third_party/limine
