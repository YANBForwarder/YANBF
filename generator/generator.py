#!/usr/bin/env python3

"""
YANBF
Copyright © 2022-present lifehackerhansol

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

import argparse
import subprocess
import io
import os
import requests
import unicodedata

from libscrc import modbus
from PIL import Image
from struct import unpack
from binascii import hexlify
from bannergif import bannergif


parser = argparse.ArgumentParser(description="YANBF Generator")
parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
parser.add_argument("-o", "--output", metavar="output.cia", type=str, nargs=1, help="output CIA")
parser.add_argument("-b", "--boxart", metavar="boxart.png", type=str, nargs=1, help="Custom banner box art")

args = parser.parse_args()

cmdarg = ""
if os.name != 'nt':
    cmdarg = "./"


def die():
    files = ['data/icon.png',
             'data/banner.bin',
             'data/output.smdh',
             'data/banner.png']
    for file in files:
        try:
            os.remove(file)
        except FileNotFoundError:
            continue
    exit()


path = args.input[0]
if not os.path.exists(os.path.abspath(path)):
    print("Failed to open ROM. Is the path valid?")
    die()
else:
    print("Extracting icon...")
    im = bannergif(path)
    print("Resizing icon...")
    im.putpalette(b"\xFF\xFF\xFF" + im.palette.palette[3:])
    im = im.convert('RGB')
    im = im.resize((48, 48), resample=Image.LINEAR)
    im.save('data/icon.png')

    # get banner title
    print("Extracting game metadata...")
    rom = open(args.input[0], "rb")
    rom.seek(0x68, 0)
    banneraddrle = rom.read(4)
    banneraddr = unpack("<I", banneraddrle)[0]
    rom.seek(banneraddr, 0)
    bannerversion = unpack("<H", rom.read(2))[0] & 3
    langnum = 6
    if bannerversion == 2:
        langnum = 7
    elif bannerversion == 3:
        langnum = 8

    # crc checking
    rom.seek(banneraddr + 0x2)
    crc083F = unpack("<H", rom.read(2))[0]
    rom.seek(banneraddr + 0x20)
    data = rom.read(0x840 - 0x20)
    calcrc = modbus(data)
    if crc083F != calcrc:
        print(f"Banner version {bannerversion}. Banner CRC does not match. If this is a ROM hack, please contact the ROM hack developer.")
        die()
    if bannerversion >= 2:
        rom.seek(banneraddr + 0x4)
        crc093F = unpack("<H", rom.read(2))[0]
        rom.seek(banneraddr + 0x20)
        data = rom.read(0x940 - 0x20)
        calcrc = modbus(data)
        if crc093F == calcrc:
            if bannerversion == 3:
                rom.seek(banneraddr + 0x6)
                crc0A3F = unpack("<H", rom.read(2))[0]
                rom.seek(banneraddr + 0x20)
                data = rom.read(0xA40 - 0x20)
                calcrc = modbus(data)
                if crc0A3F != calcrc:
                    langnum = 7
        else:
            langnum = 6
    title = []
    for x in range(langnum):
        offset = 0x240 + (0x100 * x)
        rom.seek(banneraddr + offset, 0)
        print(len(title))
        title.append(str(rom.read(0x100), "utf-16-le"))
        title[x] = title[x].split('\0', 1)[0]
    jpn_title = title[0].split("\n")
    eng_title = title[1].split("\n")
    fra_title = title[2].split("\n")
    ger_title = title[3].split("\n")
    ita_title = title[4].split("\n")
    spa_title = title[5].split("\n")
    chn_title = None
    kor_title = None
    if langnum >= 7:
        chn_title = title[6].split("\n")
        if len(chn_title) == 1 or chn_title[0][0] == "\uffff":
            chn_title = None
    if langnum >= 8:
        kor_title = title[7].split("\n")
        if len(kor_title) == 1 or kor_title[0][0] == "\uffff":
            kor_title = None
    rom.seek(0xC, 0)
    gamecode = str(rom.read(0x4), "ascii")
    rom.close()

    print("Creating SMDH...")
    bannertoolarg = f'{cmdarg}bannertool makesmdh -i "data/icon.png" '
    haspublisher = False
    if len(jpn_title) == 3:
        haspublisher = True
    if haspublisher:
        bannertoolarg += f'-s "{eng_title[0]} {eng_title[1]}" -js "{jpn_title[0]} {jpn_title[1]}" -fs "{fra_title[0]} {fra_title[1]}" -gs "{fra_title[0]} {ger_title[1]}" -is "{ita_title[0]} {ita_title[1]}" -ss "{spa_title[0]} {spa_title[1]}" '
        bannertoolarg += f'-l "{eng_title[0]} {eng_title[1]}" -jl "{jpn_title[0]} {jpn_title[1]}" -fl "{fra_title[0]} {fra_title[1]}" -gl "{fra_title[0]} {ger_title[1]}" -il "{ita_title[0]} {ita_title[1]}" -sl "{spa_title[0]} {spa_title[1]}" '
        bannertoolarg += f'-p "{eng_title[2]}" -jp "{jpn_title[2]}" -fp "{fra_title[2]}" -gp "{ger_title[2]}" -ip "{ita_title[2]}" -sp "{spa_title[2]}" '
    else:
        bannertoolarg += f'-s "{eng_title[0]}" -js "{jpn_title[0]}" -fs "{fra_title[0]}" -gs "{fra_title[0]}" -is "{ita_title[0]}" -ss "{spa_title[0]}" '
        bannertoolarg += f'-l "{eng_title[0]}" -jl "{jpn_title[0]}" -fl "{fra_title[0]}" -gl "{ger_title[0]}" -il "{ita_title[0]}" -sl "{spa_title[0]}" '
        bannertoolarg += f'-p "{eng_title[1]}" -jp "{jpn_title[1]}" -fp "{fra_title[1]}" -gp "{ger_title[1]}" -ip "{ita_title[1]}" -sp "{spa_title[1]}" '
    if chn_title is not None:
        if haspublisher:
            bannertoolarg += f'-scs "{chn_title[0]} {chn_title[1]}" '
            bannertoolarg += f'-scl "{chn_title[0]} {chn_title[1]}" '
            bannertoolarg += f'-scp "{chn_title[2]}" '
        else:
            bannertoolarg += f'-scs "{chn_title[0]}" -scl "{chn_title[0]}" -scp "{chn_title[1]}" '
    if kor_title is not None:
        if haspublisher:
            bannertoolarg += f'-ks "{kor_title[0]} {kor_title[1]}" '
            bannertoolarg += f'-kl "{kor_title[0]} {kor_title[1]}" '
            bannertoolarg += f'-kp "{kor_title[2]}" '
        else:
            bannertoolarg += f'-ks "{kor_title[0]}" -kl "{kor_title[0]}" -kp "{kor_title[1]}" '
    bannertoolarg += '-o "data/output.smdh"'
    bannertoolrun = subprocess.run(bannertoolarg, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if bannertoolrun.returncode != 0:
        print(bannertoolrun.stdout)
        print(bannertoolrun.stderr)
        die()

    # get boxart for DS, to make banner
    if not args.boxart:
        print("Downloading boxart...")
        ba_region = ""
        if gamecode[3] in ['E', 'T']:
            ba_region = "US"
        elif gamecode[3] == 'K':
            ba_region = "KO"
        elif gamecode[3] == 'J':
            ba_region = "JA"
        elif gamecode[3] == 'D':
            ba_region = "DE"
        elif gamecode[3] == 'F':
            ba_region = "FR"
        elif gamecode[3] == 'H':
            ba_region = "NL"
        elif gamecode[3] == 'I':
            ba_region = "IT"
        elif gamecode[3] == 'R':
            ba_region = "RU"
        elif gamecode[3] == 'S':
            ba_region = "ES"
        elif gamecode[3] == '#':
            ba_region = "HB"
        elif gamecode[3] == 'U':
            ba_region = "AU"
        else:
            ba_region = "EN"
        r = requests.get(f"https://art.gametdb.com/ds/coverM/{ba_region}/{gamecode}.jpg")
        if r.status_code != 200:
            print("Cannot find box art for game. Are you connected to the internet?")
            die()
    else:
        if not os.path.isfile(args.boxart[0]):
            print(f"{args.boxart[0]} does not exist. Is your argument correct?")
            die()
    print("Resizing box art...")
    banner = Image.open(args.boxart[0] if args.boxart else io.BytesIO(r.content))
    width, height = banner.size
    new_height = 128
    new_width = new_height * width // height
    banner = banner.resize((new_width, new_height), resample=Image.ANTIALIAS)
    new_image = Image.new('RGBA', (256, 128), (0, 0, 0, 0))
    upper = (256 - banner.size[0]) // 2
    new_image.paste(banner, (upper, 0))
    new_image.save('data/banner.png', 'PNG')

    print("Creating banner...")
    bannertoolarg = f"{cmdarg}bannertool makebanner -i data/banner.png -a data/dsboot.wav -o data/banner.bin"
    bannertoolrun = subprocess.run(bannertoolarg, shell=True, capture_output=True, universal_newlines=True)
    if bannertoolrun.returncode != 0:
        print(bannertoolrun.stdout)
        print(bannertoolrun.stderr)
        die()

    # CIA generation
    print("Getting filepath...")
    try:
        os.mkdir('romfs')
    except FileExistsError:
        pass
    romfs = open('romfs/path.txt', 'w', encoding="utf8")
    path = unicodedata.normalize("NFC", os.path.abspath(path))
    if os.name == 'nt':
        path = path[2:]
        path = path.replace('\\', '/')
    else:
        temp = path
        orig_dev = os.stat(temp).st_dev
        while temp != '/':
            direc = os.path.dirname(temp)
            if os.stat(direc).st_dev != orig_dev:
                break
            temp = direc
        path = path.replace(temp, "")
    romfs.write(f"sd:{path}")
    romfs.close()

    gamecodehex = hexlify(gamecode.encode()).decode()
    gamecodehex = f"0x{gamecodehex[3:8]}"
    print("Running makerom...")
    makeromarg = f"{cmdarg}makerom -f cia -target t -exefslogo -rsf data/build-cia.rsf -elf data/forwarder.elf -banner data/banner.bin -icon data/output.smdh -DAPP_ROMFS=romfs -major 0 -minor 1 -micro 0 -DAPP_VERSION_MAJOR=0 "
    makeromarg += f"-o {args.output[0] if args.output else 'output.cia'} "
    makeromarg += f'-DAPP_PRODUCT_CODE=CTR-H-{gamecode} -DAPP_TITLE="{eng_title[0]}" -DAPP_UNIQUE_ID={gamecodehex}'
    makeromrun = subprocess.run(makeromarg, shell=True, capture_output=True, universal_newlines=True)
    if makeromrun.returncode != 0:
        print(makeromrun.stdout)
        print(makeromrun.stderr)
        die()
    print("CIA generated.")

    die()
