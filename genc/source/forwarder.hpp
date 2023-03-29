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

#include <filesystem>
#include "ndsheaderbanner.h"

namespace fs = std::filesystem;

class Forwarder {
    private:
        std::string root;
        std::vector<std::string> tidlow;
        sNDSHeaderExt* header;
        sNDSBannerExt* banner;
        unsigned char* cgfx_banner_input;
    public:
        fs::path ROMpath;

        Forwarder(const std::string path);

        virtual ~Forwarder(){
            free(header);
            free(banner);
        }
        
        // funny debugging
        std::string getroot();

        bool collisioncheck();
        bool gettitle();
        bool geticon();
};
