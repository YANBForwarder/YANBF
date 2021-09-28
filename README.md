CTR-NDSForwarder
=======

nds-bootstrap forwarder template that is installed as a 3DS homebrew, to allow for more than 40 forwarders.

To build:
  1. figure out a way to put `make_cia` in PATH. Will find a better way to deal with this soon™
  1. edit `romFS:/path.txt` to any file path in `sd:/`
  1. run `make dist`
  1. install both CIAs
  1. extract sdcard.nds to `sdmc:/_nds/CTR-NDSForwarder/sdcard.nds`
  1. voila

Current issues:
  - `make_cia` needs to be in PATH. I should probably not do that
  - code is very messy and will make you cry
  - no way to install more than one forwarder at the moment
  - no actual way to grab cheats at the moment, or rather, I have no idea how to adapt the existing code to do so
  - no icon or banner. That's next up

Credits:
  - [devkitPro](https://devkitpro.org) for their toolchain, the app-launcher 3DS example code, and BootStrap
  - [RocketRobz](https://github.com/RocketRobz) for the forwarder source code
  - [Universal-Team members](https://github.com/Universal-Team) for dealing with my terrible questions
  - probably more to come soon because this code isn't even complete yet