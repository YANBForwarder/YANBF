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
#include <filesystem>
#include <unistd.h>

#include "argparse.hpp"
#include "bannerpng.h"
#include "core.hpp"

#define APPVERSION "1.5.0"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    std::cout << "YANBF Generator v" << APPVERSION << std::endl;

    argparse::ArgumentParser parser("generator", APPVERSION);
    parser.add_argument("input").help("DS ROM path").required();
    parser.add_argument("-o", "--output").help("Output path");
    parser.add_argument("-b", "--boxart").help("Custom boxart path");
    parser.add_argument("-s", "--sound").help("Custom icon sound path");
    parser.add_argument("-r", "--randomize").help("Randomize UniqueID").default_value(false).implicit_value(true);
    try{
        parser.parse_args(argc, argv);
    } catch(const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    Forwarder forwarder(parser.get<std::string>("input"));

    try {
        forwarder.collisioncheck();
    } catch(const std::invalid_argument& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    forwarder.gettitle();
    if(!forwarder.geticon()) {
        std::cerr << "Failed to save icon." << std::endl;
        return 1;
    };
    return 0;
}
