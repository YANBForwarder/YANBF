/*
    YANBF
    Copyright (C) 2022 lifehackerhansol

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <unistd.h>

#include "bannerpng.h"
#include "core.hpp"

namespace fs = std::filesystem;


Forwarder::Forwarder(const std::string path){
            ROMpath = path;
            root = ROMpath.root_path().string();
        }

        // funny debugging
std::string Forwarder::getroot() {
    return root;
}

bool Forwarder::collisioncheck() {
    std::string id0;
    std::string id1;
    std::vector<std::string> id0folders;
    std::vector<std::string> id1folders;
    std::string n3dsfolder = root + "/Nintendo 3DS";
    if (access(n3dsfolder.c_str(), F_OK) != 0) throw std::invalid_argument("Failed to find Nintendo 3DS folder. Is the ROM on the SD card?\n");
    return true;
}

