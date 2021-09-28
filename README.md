CTR-NDSForwarder
=======

nds-bootstrap forwarder template that is installed as a 3DS homebrew, to allow for more than 40 forwarders.

Requires [this SD pack](https://www.dropbox.com/s/k5uaa4jzbtkgm0z/DS%20Game%20Forwarder%20pack%20%283DS%20SD%20Card%29.7z?dl=0) provided by [RocketRobz](https://github.com/RocketRobz). Extract files accordingly.


To build:
  1. figure out a way to put `make_cia` in PATH. Will find a better way to deal with this soon™
  1. edit `romFS:/path.txt` to any file path in `sd:/`
  1. run `make`
  1. install both CIAs
  1. voila

Current issues:
  - can't seem to pass argv to the forwarder `sdcard.nds` for whatever reason. It hangs on a white screen from that point.
  - `make_cia` needs to be in PATH. I should probably not do that
  - code is very messy and will make you cry
  - no way to install more than one forwarder at the moment, but I need to solve the problem of the program actually booting first before getting there
  - how do you program DS homebrew

Credits:
  - [devkitPro](https://devkitpro.org) for their toolchain, the app-launcher 3DS example code, and BootStrap
  - [RocketRobz](https://github.com/RocketRobz) for the forwarder source code
  - [Universal-Team members](https://github.com/Universal-Team) for dealing with my terrible questions
  - probably more to come soon because this code isn't even complete yet