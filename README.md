Yet Another nds-bootstrap Forwarder
=======

YANBF (Yet Another nds-bootstrap Forwarder) is a forwarder structure for nds-bootstrap.

## Features:
- "forwarders" are referred to placeholder icons that will shortcut to a particular application (in this case, [nds-bootstrap](https://github.com/DS-Homebrew/nds-bootstrap))
    - Skip the [second menu](https://github.com/DS-Homebrew/TWiLightMenu) and just directly launch
- Launches from 3DS-mode.
    - The other implementation by @RocketRobz and @Olmectron, https://gbatemp.net/threads/nds-forwarder-cias-for-your-home-menu.426174/, is DSiWare, which means you are limited to only 40 titles. This works great on the DSi, it does not work great on the 3DS.
    - This means you can now install as many icons as you want until you max out the HOME menu icon space!

## How it works:
- A pre-existing forwarder template exists: https://gbatemp.net/threads/nds-forwarder-cias-for-your-home-menu.426174/
    - This uses a DSiWare template, copies the icon and title and the ROM path, and then installed as a CIA. The CIA, when launched, will boot `sdcard.nds` which will do all the nds-bootstrap setup.
    - I instead opted to create an `sdcard.nds` bootloader, and then a 3DS-mode app that launches said bootloader
        - you can't launch `.nds` files from 3DS-mode. So a bootloader is required.
        - the bootloader does the same thing as the DSiWare template, in actuality

## How to use:
1. Install Python 3 for your PC: http://python.org/downloads
1. Go to https://github.com/lifehackerhansol/YANBF/releases, and download the appropriate zip file for your PC OS
1. Go to https://github.com/RocketRobz/NTR_Forwarder/releases, and download the zip file that is not the source code
1. Extract the zip files
1. From the DS Game Forwarder Pack, copy everything in the for SD card root folder to your SD card root
1. From YANBF, copy bootstrap.cia to your SD card root
1. Download the latest TWiLight Menu++ version [here](https://github.com/DS-Homebrew/TWiLightMenu/releases). Only get the `3DS.7z` file.
1. In the 7z file, go to `_nds`, `TWiLightMenu`, and `extras`.
1. Drag the `apfix.pck` and `widescreen.pck` file to `sd:/_nds/ntr-forwarder/`.
1. Open a terminal or command prompt and cd inside the generator folder
1. Run pip install -r requirements.txt
1. Run python3 generator.py <path to ROM on your SD card>
    - Make sure to replace <path to ROM on your SD card> with the full path to your ROM
    - this ROM MUST BE ON YOUR SD CARD! It will fail to generate properly otherwise!
1. An output.cia will be generated. Copy this to your SD card
1. Boot your 3DS and install both CIAs using FBI

## Updating files:
### Using Universal-Updater
1. Open Universal-Updater on your 3DS
    - You can download it here: https://github.com/Universal-Team/Universal-Updater/releases
1. Navigate to `YANBF`, then install the YANBF Forwarder Pack

### Manually
1. Re-follow steps 2-9 in the "How to use" section

## To build:
  1. edit `romFS:/path.txt` to any file path in `sd:/`. Do NOT end with newline, this kills it.
  1. run `make dist`
  1. install both CIAs
  1. voila

## Current issues:
  - no idea. report if you see one

## Credits:
  - [devkitPro](https://devkitpro.org) for their toolchain
  - [RocketRobz](https://github.com/RocketRobz/NTR_Forwarder) for the TWLNAND bootstrap code
  - [Epicpkmn11](https://github.com/Epicpkmn11) for bannergif.py and testing
  - [Olmectron](https://github.com/Olmectron/Simple-Web-App-GUI-for-YANBF-Generator) for the GUI wrapper

## License:

### Bootstrap:
```
 Copyright (C) 2010  Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
```

### The rest:
```
MIT License

bannergif.py
Copyright (C) 2021 Pk11

generator.py and CIA template
Copyright (C) 2022-present lifehackerhansol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
