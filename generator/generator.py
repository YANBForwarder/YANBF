import argparse
import subprocess

from PIL import Image
from bannergif import bannergif

iconsize = (48, 48)

parser = argparse.ArgumentParser(description="CTR-NDSForwarder Generator")
parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
parser.add_argument("-o", "--output", metavar="output.gif", type=str, nargs=1, help="output CIA")

args = parser.parse_args()

err = bannergif(args.input[0])
if err != 0:
    print("Failed to open ROM. Is the path valid?")
    exit()
else:
    im = Image.open('output.gif')
    im = im.resize(iconsize)
    im.save('output.png')
    # subprocess.run()