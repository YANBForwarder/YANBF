CTR-NDSForwarder
=======

nds-bootstrap forwarder template that is installed as a 3DS homebrew, to allow for more than 40 forwarders.

How to use:
  1. Download release zip
  1. Extract release zip
  1. Copy all files in `for SD card root` to root of SD card
  1. Install all CIAs in `cia` folder
  1. Get Python 3 for your PC
  1. `pip install Pillow requests`
  1. `python3 generator.py <path to ROM on SD>`

To build:
  1. figure out a way to put `make_cia` in PATH. Will find a better way to deal with this soon™
  1. edit `romFS:/path.txt` to any file path in `sd:/`
  1. run `make dist`
  1. install both CIAs
  1. extract sdcard.nds to `sdmc:/_nds/CTR-NDSForwarder/sdcard.nds`
  1. voila

Current issues:
  - `make_cia` needs to be in PATH. I should probably not do that
    - UNIX requires even more to be in path, such as `makerom` and `bannertool`
  - code is very messy and will make you cry
  - no actual way to grab cheats at the moment, or rather, I have no idea how to adapt the existing code to do so
  - no banner. That's next up

Credits:
  - [devkitPro](https://devkitpro.org) for their toolchain and BootStrap
  - [RocketRobz](https://github.com/RocketRobz/NTR_Forwarder) for the forwarder source code
  - [Universal-Team members](https://github.com/Universal-Team) for dealing with my terrible questions and Epicpkmn11 for testing
  - [aspargas2](https://github.com/aspargas2) for basic python:tm:
  - probably more to come soon because this code isn't even complete yet

License:

Bootstrap / SD template:
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

The rest:
```
MIT License

bannergif.py
Copyright (C) 2022 Pk11

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

