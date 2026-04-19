from PIL import Image

img = Image.open("bg.bmp")

# half size should be good ig?
new_width = img.width // 2
new_height = img.height // 2

resized_img = img.resize((new_width, new_height))

resized_img.save("bg_small.bmp")

print("Image resized successfully!")
