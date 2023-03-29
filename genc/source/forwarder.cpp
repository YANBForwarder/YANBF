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

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <unistd.h>

#include "bannerpng.h"
#include "core.hpp"
#include "imageconvert.hpp"

namespace fs = std::filesystem;


Forwarder::Forwarder(const std::string path){
    ROMpath = path;
    root = ROMpath.root_path().string();
    header = (sNDSHeaderExt*)malloc(sizeof(sNDSHeaderExt));
    banner = (sNDSBannerExt*)malloc(sizeof(sNDSBannerExt));
}

// funny debugging
std::string Forwarder::getroot() {
    return root;
}

bool Forwarder::collisioncheck() {
    std::vector<fs::path> id0folders;
    std::vector<fs::path> id1folders;
    fs::path n3dsfolder = root + "Nintendo 3DS";
    if (!fs::exists(n3dsfolder)) throw std::invalid_argument("Failed to find Nintendo 3DS folder. Is the ROM on the SD card?\n");
    for (const auto& dirEntry : fs::directory_iterator(n3dsfolder)) {
        if(fs::is_directory(dirEntry.path())) {
            std::string foldername = dirEntry.path().filename().string();
            if (foldername.size() == 32) id0folders.push_back(dirEntry.path());
        }
    }
    if(id0folders.size() != 1) throw std::invalid_argument("No ID0 folder, or more than one ID0 folder detected.\n");
    for (const auto& dirEntry : fs::directory_iterator(id0folders[0])) {
        if(fs::is_directory(dirEntry.path())) {
            std::string foldername = dirEntry.path().filename().string();
            if (foldername.size() == 32) id1folders.push_back(dirEntry.path());
        }
    }
    if(id1folders.size() != 1) throw std::invalid_argument("No ID1 folder, or more than one ID1 folder detected.\n");
    fs::path titlefolder = id1folders[0].string() + "/title/00040000";
    for (const auto& dirEntry : fs::directory_iterator(titlefolder)) if(fs::is_directory(dirEntry.path())) tidlow.push_back(dirEntry.path().filename().string());
    for(int i=0;i<tidlow.size();i++) tidlow[i] = tidlow[i].substr(1, tidlow[i].size()-3);

    return true;
}

bool Forwarder::gettitle() {
    FILE* rom = fopen(ROMpath.string().c_str(), "rb");
	fread(header, sizeof(sNDSHeaderExt), 1, rom);
    fseek(rom, header->bannerOffset, SEEK_SET);
    fread(banner, sizeof(sNDSBannerExt), 1, rom);
    fclose(rom);
    return true;
}

bool Forwarder::geticon() {
    return bannerpng(banner, "data/output.png");
}

bool Forwarder::getBannerImage() {
    cgfx_banner_input = convertBanner();
    return true;
}