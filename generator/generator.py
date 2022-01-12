import argparse
import subprocess

from PIL import Image
from struct import unpack
from bannergif import bannergif

iconsize = (48, 48)

parser = argparse.ArgumentParser(description="CTR-NDSForwarder Generator")
parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
parser.add_argument("-o", "--output", metavar="output.cia", type=str, nargs=1, help="output CIA")

args = parser.parse_args()

err = bannergif(args.input[0])
if err != 0:
    print("Failed to open ROM. Is the path valid?")
    exit()
else:
    im = Image.open('output.gif')
    im = im.resize(iconsize)
    im.save('output.png')

    # get banner title
    rom = open(args.input[0], "rb")
    rom.seek(0x68, 0)
    banneraddrle = rom.read(4)
    banneraddr = unpack("<I", banneraddrle)[0]
    rom.seek(banneraddr, 0)
    title = []
    haspublisher = False
    for x in range(8):
        offset = 0x240 + (0x100 * x)
        rom.seek(banneraddr + offset, 0)
        title.append(str(rom.read(0x100), "utf-16-le"))
        title[x] = title[x].split('\0', 1)[0]
    jpn_title = title[0].split("\n")
    eng_title = title[1].split("\n")
    fra_title = title[2].split("\n")
    ger_title = title[3].split("\n")
    ita_title = title[4].split("\n")
    spa_title = title[5].split("\n")
    chn_title = title[6].split("\n")
    kor_title = title[7].split("\n")
    if chn_title[0][0] == "\uffff":
        chn_title = None
    if kor_title[0][0] == "\uffff":
        kor_title = None
    bannertoolarg = 'bannertool makesmdh -i "output.png" '
    bannertoolarg += f'-js "{jpn_title[0]}" -es "{eng_title[0]}" -fs "{fra_title[0]}" -gs "{ger_title[0]}" -is "{ita_title[0]}" -ss "{spa_title[0]}" -ds "{eng_title[0]}" -ps "{eng_title[0]}" -rs "{eng_title[0]}" -tcs "{eng_title[0]}" -jl "{jpn_title[1]}" -el "{eng_title[1]}" -fl "{fra_title[1]}" -gl "{ger_title[1]}" -il "{ita_title[1]}" -sl "{spa_title[1]}" -dl "{eng_title[1]}" -pl "{eng_title[1]}" -rl "{eng_title[1]}" -tcl "{eng_title[1]}" '
    if len(jpn_title) == 3:
        haspublisher = True
    if haspublisher:
        bannertoolarg += f'-jp "{jpn_title[2]}" -ep "{eng_title[2]}" -fp "{fra_title[2]}" -gp "{ger_title[2]}" -ip "{ita_title[2]}" -sp "{spa_title[2]}" -dp "{eng_title[2]}" -pp "{eng_title[2]}" -rp "{eng_title[2]}" -tcp "{eng_title[2]}" '
    else:
        bannertoolarg += '-p "nds-bootstrap Forwarder" '
    if chn_title is not None:
        bannertoolarg += f'-scs "{chn_title[0]}" -scl "{chn_title[1]}"'
        if haspublisher:
            bannertoolarg += f' -scp "{chn_title[2]}" '
    else:
        bannertoolarg += f'-scs "{eng_title[0]}" -scl "{eng_title[1]}"'
        if haspublisher:
            bannertoolarg += f' -scp "{eng_title[2]}" '
    if kor_title is not None:
        bannertoolarg += f'-ks "{kor_title[0]}" -kl "{kor_title[1]}" '
        if haspublisher:
            bannertoolarg += f'-kp "{kor_title[2]}" '
    else:
        bannertoolarg += f'-ks "{eng_title[0]}" -kl "{eng_title[1]}" '
        if haspublisher:
            bannertoolarg += f' -kp "{eng_title[2]}" '
    bannertoolarg += '-o "output.smdh"'
    subprocess.call(bannertoolarg, shell=True)

