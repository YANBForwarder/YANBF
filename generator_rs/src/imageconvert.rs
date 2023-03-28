/*
    Copyright (C) 2022-2023 lifehackerhansol

    SPDX-License-Identifier: MIT
 */

use std::fs::File;
use image::{ColorType, GenericImage, RgbaImage, imageops};

pub fn resize_banner(path: &String) {
    let mut banner = image::open(path).unwrap();
    let new_height = 128;
    let new_width = new_height * banner.width() / banner.height();
    println!("{}", banner.width());
    banner = banner.resize(new_width, new_height, imageops::FilterType::Triangle);
    let mut new_image = RgbaImage::new(256, 128);
    println!("{}", banner.width());
    let upper = (256 - banner.width()) / 2;
    new_image.copy_from(&banner, upper, 0);
    image::save_buffer("end.png", &new_image, 256, 128, ColorType::Rgba8);
}
