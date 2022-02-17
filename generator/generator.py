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
import os

import core

def execute(error):
    if isinstance(error, int):
        return
    else:
        print(error)
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

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="YANBF Generator")
    parser.add_argument("input", metavar="input.nds", type=str, nargs=1, help="DS ROM path")
    parser.add_argument("-o", "--output", metavar="output.cia", type=str, nargs=1, help="output CIA")
    parser.add_argument("-b", "--boxart", metavar="boxart.png", type=str, nargs=1, help="Custom banner box art")
    parser.add_argument("-r", "--randomize", action='store_true', help="Randomize UniqueID")

    args = parser.parse_args() 
    path = None
    boxart = None
    output = None
    randomize = False
    if args.boxart:
        boxart = args.boxart[0]
    if args.input:
        path = args.input[0]
    if args.output:
        output = args.output[0]
    if args.randomize:
        randomize = True
    if not os.path.exists(os.path.abspath(path)):
        die("Failed to open ROM. Is the path valid?")
    cmdarg = ""
    if os.name != 'nt':
        cmdarg = "./"
    print("Extracting and resizing icon...")
    core.makeicon(path)
    print("Getting ROM titles...")
    title = core.get_title(path)
    print("Creating SMDH...")
    execute(core.makesmdh(cmdarg, path, title))
    print("Downloading boxart...")
    execute(core.downloadboxart(path, boxart))
    print("Creating banner...")
    execute(core.makebanner(cmdarg, path))
    print("Getting filepath...")
    execute(core.makeromfs(path))
    print("Running makerom...")
    execute(core.makecia(cmdarg, path, title, output, randomize))
