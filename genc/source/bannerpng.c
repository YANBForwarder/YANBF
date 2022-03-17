/* Copyright © 2022 Pk11
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* To build:
 * gcc bannerpng.c lodepng.c -o bannerpng
 * https://lodev.org/lodepng/
 */

#include "lodepng.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @param input path to the DS ROM
 * @param output path to write to
 * @return true if successful
 */
bool bannerpng(const char *input, const char *output) {
	FILE *rom = fopen(input, "rb");
	if(!rom)
		return false;
	
	// Seek to banner
	fseek(rom, 0x68, SEEK_SET);
	uint32_t bannerOfs;
	fread(&bannerOfs, sizeof(uint32_t), 1, rom);
	fseek(rom, bannerOfs, SEEK_SET);

	// Load banner data
	uint8_t palette[0x10][4]; // 16 colors RGBA
	uint8_t bitmap[32 * 32 * 4]; // 32×32 RGBA

	// Read palette
	fseek(rom, bannerOfs + 0x220, SEEK_SET);
	for(int i = 0; i < 0x10; i++) {
		uint16_t color;
		fread(&color, sizeof(uint16_t), 1, rom);
		palette[i][0] = lroundf((color & 0x1F) * 255.0f / 31.0f);
		palette[i][1] = lroundf(((color >> 5) & 0x1F) * 255.0f / 31.0f);
		palette[i][2] = lroundf(((color >> 10) & 0x1F) * 255.0f / 31.0f);
		palette[i][3] = i == 0 ? 0x00 : 0xFF;
	}

	// Read bitmap
	fseek(rom, bannerOfs + 0x20, SEEK_SET);
	for(int ty = 0; ty < 4; ty++) {
		for(int tx = 0; tx < 4; tx++) {
			for(int y = 0; y < 8; y++) {
				for(int x = 0; x < 4; x++) {
					uint8_t byte = fgetc(rom);
					memcpy(bitmap + ((ty * 8 + y) * 32 * 4) + (tx * 8 + x * 2) * 4, palette[byte & 0xF], 4);
					memcpy(bitmap + ((ty * 8 + y) * 32 * 4) + (tx * 8 + x * 2) * 4 + 4, palette[byte >> 4], 4);
				}
			}
		}
	}

	fclose(rom);

	lodepng_encode32_file(output, bitmap, 32, 32);

	return true;
}
