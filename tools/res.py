import sys

from PIL import Image


def resize_bmp(input_path, output_path, width, height):
    img = Image.open(input_path)
    img = img.resize((width, height), Image.Resampling.LANCZOS)
    img.save(output_path, format="BMP")


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python resize_bmp.py input.bmp output.bmp width height")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]
    width = int(sys.argv[3])
    height = int(sys.argv[4])

    resize_bmp(input_path, output_path, width, height)
