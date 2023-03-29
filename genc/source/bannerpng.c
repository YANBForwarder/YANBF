/* Copyright © 2022 Pk11
 * Copyright © 2022 lifehackerhansol (some funny hack)
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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ext/lodepng.h"
#include "ndsheaderbanner.h"

/**
 * @param input path to the DS ROM
 * @param output path to write to
 * @return true if successful
 */
bool bannerpng(const sNDSBannerExt *banner, const char *output) {
	// Load banner data
	uint8_t palette[0x10][4]; // 16 colors RGBA
	uint8_t bitmap[32 * 32 * 4]; // 32×32 RGBA

	// Read palette
	for(int i = 0; i < 0x10; i++) {
		uint16_t color = banner->palette[i];
		palette[i][0] = lroundf((color & 0x1F) * 255.0f / 31.0f);
		palette[i][1] = lroundf(((color >> 5) & 0x1F) * 255.0f / 31.0f);
		palette[i][2] = lroundf(((color >> 10) & 0x1F) * 255.0f / 31.0f);
		palette[i][3] = i == 0 ? 0x00 : 0xFF;
	}

	// Read bitmap
	uint8_t *icon = banner->icon;
	for(int ty = 0; ty < 4; ty++) for(int tx = 0; tx < 4; tx++) for(int y = 0; y < 8; y++) for(int x = 0; x < 4; x++) {
		uint8_t byte = *(icon++);
		memcpy(bitmap + ((ty * 8 + y) * 32 * 4) + (tx * 8 + x * 2) * 4, palette[byte & 0xF], 4);
		memcpy(bitmap + ((ty * 8 + y) * 32 * 4) + (tx * 8 + x * 2) * 4 + 4, palette[byte >> 4], 4);
	}

	int err = lodepng_encode32_file(output, bitmap, 32, 32);

	return err == 0 ? true : false;
}
