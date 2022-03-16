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

import subprocess
import io
import os
import requests
import unicodedata
import random

from libscrc import modbus
from PIL import Image
from struct import unpack
from binascii import hexlify
from bannergif import bannergif


def getroot(path) -> str:
    root: str = ""
    if os.name == 'nt':
        root = os.path.abspath(path)[:2]
    else:
        temp = path
        orig_dev = os.stat(temp).st_dev
        while temp != '/':
            direc = os.path.dirname(temp)
            if os.stat(direc).st_dev != orig_dev:
                break
            temp = direc
        root = temp
    return root


def collisioncheck(root, path) -> list:
    tidlow = []
    id0 = None
    id1 = None
    id0folders = []
    id1folders = []
    if not os.path.isdir(f"{root}/Nintendo 3DS"):
        return "Failed to find Nintendo 3DS folder. Is the ROM on the SD card?"
    for folder in os.listdir(f"{root}/Nintendo 3DS"):
        if os.path.isdir(f"{root}/Nintendo 3DS/{folder}") and len(folder) == 32:
            id0folders.append(folder)
    if len(id0folders) > 1:
        return "More than one ID0 folder detected. Please remove unnecessary ID0 folders before continuing."
    elif len(id0folders) == 0:
        return "ID0 not found. Is this ROM on the SD card?"
    else:
        id0 = id0folders[0]
    for folder in os.listdir(f"{root}/Nintendo 3DS/{id0}"):
        if os.path.isdir(f"{root}/Nintendo 3DS/{id0}/{folder}") and len(folder) == 32:
            id1folders.append(folder)
    if len(id1folders) > 1:
        return "More than one ID1 folder detected. Please remove unnecessary ID1 folders before continuing."
    elif len(id1folders) == 0:
        return "ID1 not found. Is this ROM on the SD card?"
    else:
        id1 = id1folders[0]
    for name in os.listdir(f"{root}/Nintendo 3DS/{id0}/{id1}/title/00040000"):
        tidlow.append(name)
    for index, value in enumerate(tidlow):
        tidlow[index] = value[1:-2]
    return tidlow


def get_title(path) -> dict:
    # get banner title
    rom = open(path, "rb")
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

    # crc checking, ignore banner version 1
    if bannerversion >= 2:
        rom.seek(banneraddr + 0x4)
        crc093F = unpack("<H", rom.read(2))[0]
        rom.seek(banneraddr + 0x20)
        data = rom.read(0x940 - 0x20)
        calcrc = modbus(data)
        if crc093F != calcrc:
            langnum = 6
        else:
            if bannerversion == 3:
                rom.seek(banneraddr + 0x6)
                crc0A3F = unpack("<H", rom.read(2))[0]
                rom.seek(banneraddr + 0x20)
                data = rom.read(0xA40 - 0x20)
                calcrc = modbus(data)
                if crc0A3F != calcrc:
                    langnum = 7
    title = []
    titles = {
    }
    for x in range(langnum):
        offset = 0x240 + (0x100 * x)
        rom.seek(banneraddr + offset, 0)
        title.append(str(rom.read(0x100), "utf-16-le"))
        title[x] = title[x].split('\0', 1)[0]
    titles['jpn'] = title[0].split('\n')
    titles['eng'] = title[1].split('\n')
    titles['fra'] = title[2].split('\n')
    titles['ger'] = title[3].split('\n')
    titles['ita'] = title[4].split('\n')
    titles['spa'] = title[5].split('\n')
    if langnum >= 7:
        titles['chn'] = title[6].split("\n")
    if langnum >= 8:
        titles['kor'] = title[7].split("\n")
    for lang in titles:
        if len(titles[lang]) == 1:
            titles[lang] = None
    return titles


def getgamecode(path):
    rom = open(path, "rb")
    rom.seek(0xC, 0)
    code = str(rom.read(0x4), "ascii")
    rom.close()
    return code


def makeicon(path):
    im = bannergif(path)
    im.putpalette(b"\xFF\xFF\xFF" + im.palette.palette[3:])
    im = im.convert('RGB')
    im = im.resize((48, 48), resample=Image.LINEAR)
    im.save('data/icon.png')
    return 0


def makesmdh(cmdarg, path, title):
    bannertoolarg = f'{cmdarg}bannertool makesmdh -i "data/icon.png" '

    if len(title['eng']) == 3:
        bannertoolarg += f'-s "{title["eng"][0]} {title["eng"][1]}" -l "{title["eng"][0]} {title["eng"][1]}" -p "{title["eng"][2]}" '
    else:
        bannertoolarg += f'-s "{title["eng"][0]}" -l "{title["eng"][0]}" -p "{title["eng"][1]}" '

    if title['jpn'] is not None:
        if len(title['jpn']) == 3:
            bannertoolarg += f'-js "{title["jpn"][0]} {title["jpn"][1]}" -jl "{title["jpn"][0]} {title["jpn"][1]}" -jp "{title["jpn"][2]}" '
        else:
            bannertoolarg += f'-js "{title["jpn"][0]}" -jl "{title["jpn"][0]}" -jp "{title["jpn"][1]}" '

    if title['fra'] is not None:
        if len(title['fra']) == 3:
            bannertoolarg += f'-fs "{title["fra"][0]} {title["fra"][1]}" -fl "{title["fra"][0]} {title["fra"][1]}" -fp "{title["fra"][2]}" '
        else:
            bannertoolarg += f'-fs "{title["fra"][0]}" -fl "{title["fra"][0]}" -fp "{title["fra"][1]}" '

    if title['ger'] is not None:
        if len(title['ger']) == 3:
            bannertoolarg += f'-gs "{title["ger"][0]} {title["ger"][1]}" -gl "{title["ger"][0]} {title["ger"][1]}" -gp "{title["ger"][2]}" '
        else:
            bannertoolarg += f'-gs "{title["ger"][0]}" -gl "{title["ger"][0]}" -gp "{title["ger"][1]}" '

    if title['ita'] is not None:
        if len(title['ita']) == 3:
            bannertoolarg += f'-is "{title["ita"][0]} {title["ita"][1]}" -il "{title["ita"][0]} {title["ita"][1]}" -ip "{title["ita"][2]}" '
        else:
            bannertoolarg += f'-is "{title["ita"][0]}" -il "{title["ita"][0]}" -ip "{title["ita"][1]}" '

    if title['spa'] is not None:
        if len(title['spa']) == 3:
            bannertoolarg += f'-ss "{title["spa"][0]} {title["spa"][1]}" -sl "{title["spa"][0]} {title["spa"][1]}" -sp "{title["spa"][2]}" '
        else:
            bannertoolarg += f'-ss "{title["spa"][0]}" -sl "{title["spa"][0]}" -sp "{title["spa"][1]}" '

    if 'chn' in title and title['chn'] is not None:
        if len(title['chn']) == 3:
            bannertoolarg += f'-scs "{title["chn"][0]} {title["chn"][1]}" -scl "{title["chn"][0]} {title["chn"][1]}" -scp "{title["chn"][2]}" '
        else:
            bannertoolarg += f'-scs "{title["chn"][0]}" -scl "{title["chn"][0]}" -scp "{title["chn"][1]}" '

    if 'kor' in title and title['kor'] is not None:
        if len(title['kor']) == 3:
            bannertoolarg += f'-ks "{title["kor"][0]} {title["kor"][1]}" -kl "{title["kor"][0]} {title["kor"][1]}" -kp "{title["kor"][2]}" '
        else:
            bannertoolarg += f'-ks "{title["kor"][0]}" -kl "{title["kor"][0]}" -kp "{title["kor"][1]}" '

    bannertoolarg += '-o "data/output.smdh"'
    bannertoolrun = subprocess.run(bannertoolarg, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    if bannertoolrun.returncode != 0:
        return f"{bannertoolrun.stdout}\n{bannertoolrun.stderr}"
    return 0


def downloadfromapi(path, boxart=None, sound=None):
    gamecode = getgamecode(path)
    boxartdl = False
    sounddl = False
    r = requests.get(f"https://yanbf.api.hansol.ca/banner/{gamecode}")
    if r.status_code == 200:
        data = r.json()
        if not boxart:
            if 'image' in data:
                r = requests.get(data['image'])
                if r.status_code == 200:
                    f = open("data/banner.png", 'wb')
                    f.write(r.content)
                    f.close()
                    boxartdl = True
        if not sound:
            if 'sound' in data:
                r = requests.get(data['sound'])
                if r.status_code == 200:
                    f = open("data/customsound.wav", 'wb')
                    f.write(r.content)
                    f.close()
                    sounddl = True
    if boxartdl and sounddl:
        return 1
    elif sounddl:
        return 2
    elif boxartdl:
        return 3
    return 0


def downloadboxart(path, boxart=None):
    # get boxart for DS, to make banner
    gamecode = getgamecode(path)
    if not boxart:
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
            return "Cannot find box art for game. Are you connected to the internet?"
    else:
        if not os.path.isfile(boxart):
            return f"{boxart} does not exist. Is your argument correct?"
    banner = Image.open(boxart if boxart else io.BytesIO(r.content))
    width, height = banner.size
    new_height = 128
    new_width = new_height * width // height
    banner = banner.resize((new_width, new_height), resample=Image.ANTIALIAS)
    new_image = Image.new('RGBA', (256, 128), (0, 0, 0, 0))
    upper = (256 - banner.size[0]) // 2
    new_image.paste(banner, (upper, 0))
    new_image.save('data/banner.png', 'PNG')
    return 0


def makebanner(cmdarg, path, sound):
    if sound:
        if not os.path.isfile(sound):
            return "Sound does not exist. Is your argument correct?"
    else:
        sound = "data/dsboot.wav"
    bannertoolrun = subprocess.run(f"{cmdarg}bannertool makebanner -i data/banner.png -a {sound} -o data/banner.bin", shell=True, capture_output=True, universal_newlines=True)
    if bannertoolrun.returncode != 0:
        return f"{bannertoolrun.stdout}\n{bannertoolrun.stderr}"
    return 0


def makeromfs(root, path):
    # CIA generation
    try:
        os.mkdir('romfs')
    except FileExistsError:
        pass
    romfs = open('romfs/path.txt', 'w', encoding="utf8")
    path = unicodedata.normalize("NFC", os.path.abspath(path))
    path = path.replace(root, "")
    if os.name == 'nt':
        path = path.replace('\\', '/')
    romfs.write(f"sd:{path}")
    romfs.close()
    return 0


def makecia(cmdarg, root, path, title, output=None, randomize=False, tidlow=[]):
    if not output:
        output = f"{root}/cias/{os.path.basename(path)}.cia"
        try:
            os.mkdir(f"{root}/cias")
        except FileExistsError:
            pass
    gamecode = getgamecode(path)
    uniqueid = None
    if randomize:
        uniqueid = hex(random.randint(0x300, 0xF7FFF))[2:]
    else:
        gamecodeint = int(hexlify(gamecode.encode()).decode(), 16)
        uniqueid = hex(gamecodeint ^ ((gamecodeint) >> 27))[3:8]
    while uniqueid in tidlow:
        uniqueid = hex(int(uniqueid, 16) + 1)
    makeromarg = f"{cmdarg}makerom -f cia -target t -exefslogo -rsf data/build-cia.rsf -elf data/forwarder.elf -banner data/banner.bin -icon data/output.smdh -DAPP_ROMFS=romfs -major 1 -minor 5 -micro 0 -DAPP_VERSION_MAJOR=1 "
    if output:
        makeromarg += f'-o "{output}" '
    else:
        makeromarg += '-o "output.cia" '
    makeromarg += f'-DAPP_PRODUCT_CODE=CTR-H-{gamecode} -DAPP_TITLE="{title["eng"][0]}" -DAPP_UNIQUE_ID=0x{uniqueid}'
    makeromrun = subprocess.run(makeromarg, shell=True, capture_output=True, universal_newlines=True)
    if makeromrun.returncode != 0:
        return f"{makeromrun.stdout}\n{makeromrun.stderr}"
    return "CIA generated."
