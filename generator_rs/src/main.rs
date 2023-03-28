/*
    Copyright (C) 2022-2023 lifehackerhansol

    SPDX-License-Identifier: MIT
 */

use std::fs::File;
use std::env;
mod imageconvert;
mod ndsheaderbanner;

fn main() {
    println!("hi");
    // i miss argc/argv
    let argv: Vec<String> = env::args().collect();
    imageconvert::resize_banner(&argv[1]);
}
