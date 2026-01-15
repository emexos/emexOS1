import os
import sys

from PIL import Image

if len(sys.argv) != 3:
    # input.bmp output.bmp
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]

with Image.open(input_path) as img:
    img.save(output_path, format="BMP")
