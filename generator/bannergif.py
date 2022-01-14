"""
Copyright © 2021 Pk11

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""

from PIL import Image
import struct

def bannergif(path=""):
    if path == "":
        return -1
    rom = open(path, "rb")
    # Seek to banner
    rom.seek(0x68)
    rom.seek(struct.unpack("<I", rom.read(4))[0])

    # Load banner data
    bitmaps = []
    palettes = []
    animation = []
    if struct.unpack("<H", rom.read(2))[0] == 0x103:  # DSi (animated)
        # Read frame bitmaps
        rom.seek(0x123E, 1)
        for _ in range(8):
            bitmap = [0] * 32 * 32
            for ty in range(4):
                for tx in range(4):
                    for y in range(8):
                        for x in range(4):
                            byte = struct.unpack("B", rom.read(1))[0]
                            bitmap[((ty * 8 + y) * 32) + tx * 8 + x * 2] = byte & 0xF
                            bitmap[((ty * 8 + y) * 32) + tx * 8 + x * 2 + 1] = byte >> 4
            bitmaps.append(bitmap)

        # Read palettes
        for _ in range(8):
            palette = [0] * 256 * 3  # Pillow wants a 256 color palette with RGB separated
            for i in range(0x10):
                color = struct.unpack("<H", rom.read(2))[0]
                palette[i * 3] = round((color & 0x1F) * 255 / 31)
                palette[i * 3 + 1] = round(((color >> 5) & 0x1F) * 255 / 31)
                palette[i * 3 + 2] = round(((color >> 10) & 0x1F) * 255 / 31)
            palettes.append(palette)

        # Read animation sequence
        for i in range(0x40):
            value = struct.unpack("<H", rom.read(2))[0]
            animation.append({
                "vflip": True if value & (1 << 15) else False,
                "hflip": True if value & (1 << 14) else False,
                "palette": (value >> 11) >> 7,
                "bitmap": (value >> 8) & 7,
                "duration": value & 0xFF
            })
    else:  # DS
        # Read bitmap
        rom.seek(0x1E, 1)
        bitmap = [0] * 32 * 32
        for ty in range(4):
            for tx in range(4):
                for y in range(8):
                    for x in range(4):
                        byte = struct.unpack("B", rom.read(1))[0]
                        bitmap[((ty * 8 + y) * 32) + tx * 8 + x * 2] = byte & 0xF
                        bitmap[((ty * 8 + y) * 32) + tx * 8 + x * 2 + 1] = byte >> 4
        bitmaps.append(bitmap)

        # Read palette
        palette = [0] * 256 * 3  # Pillow wants a 256 color palette with RGB separated
        for i in range(0x10):
            color = struct.unpack("<H", rom.read(2))[0]
            palette[i * 3] = round((color & 0x1F) * 255 / 31)
            palette[i * 3 + 1] = round(((color >> 5) & 0x1F) * 255 / 31)
            palette[i * 3 + 2] = round(((color >> 10) & 0x1F) * 255 / 31)
        palettes.append(palette)

        # No animation, just show the first frame as there's only one
        animation = [{
            "vflip": False,
            "hflip": False,
            "palette": 0,
            "bitmap": 0,
            "duration": 1
        }]

    # Convert to Pillow image
    images = []
    delays = []
    for i, frame in enumerate(animation):
        # Animation ends when the animation u16 is 0, since it's split here
        # for ease of use checking just the duration should be fine
        if frame["duration"] == 0:
            break

        # 32x32 Paletted image
        img = Image.frombytes("P", (32, 32), bytes(bitmaps[frame["bitmap"]]))
        img.putpalette(palettes[frame["palette"]])

        # Flip the image if needed
        if(frame["hflip"]):
            img = img.transpose(Image.FLIP_LEFT_RIGHT)
        if(frame["vflip"]):
            img = img.transpose(Image.FLIP_TOP_BOTTOM)

        # Add it to the output list
        images.append(img)
        # The 'duration' is in frames (1/60th of a second), Pillow wants
        # miliseconds. (though GIFs are actually centiseconds)
        # Also, basically all GIF viewers will wait longer if the delay is
        # under 20 miliseconds, so use that as the minimum. If you want
        # more accurate timing you'll need a different output format.
        delays.append(max(frame["duration"] * 1000 // 60, 20))

    # Save output image
    return images[0]
