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

#include <string>

#include "ext/JustInterp.h"
#include "ext/lodepng.h"
#include "ext/ujpeg.h"

void bilinearResize(float *data, uint32_t input_width,
                            uint32_t input_height, uint32_t output_width,
                            uint32_t output_height, float *output) {
    float x_ratio, y_ratio;

    if (output_width > 1) {
        x_ratio = ((float)input_width - 1.0) / ((float)output_width - 1.0);
    } else {
        x_ratio = 0;
    }

    if (output_height > 1) {
        y_ratio = ((float)input_height - 1.0) / ((float)output_height - 1.0);
    } else {
        y_ratio = 0;
    }

    for (int i = 0; i < output_height; i++) {
        for (int j = 0; j < output_width; j++) {
            float x_l = floor(x_ratio * (float)j);
            float y_l = floor(y_ratio * (float)i);
            float x_h = ceil(x_ratio * (float)j);
            float y_h = ceil(y_ratio * (float)i);

            float x_weight = (x_ratio * (float)j) - x_l;
            float y_weight = (y_ratio * (float)i) - y_l;

            float a = data[(int)y_l * input_width + (int)x_l];
            float b = data[(int)y_l * input_width + (int)x_h];
            float c = data[(int)y_h * input_width + (int)x_l];
            float d = data[(int)y_h * input_width + (int)x_h];

            float pixel = a * (1.0 - x_weight) * (1.0 - y_weight) +
                          b * x_weight * (1.0 - y_weight) +
                          c * y_weight * (1.0 - x_weight) +
                          d * x_weight * y_weight;

            output[i * output_width + j] = pixel;
        }
    }
}

void convertBanner(std::string srcpath, std::string dstpath) {
	// output image size
	int outWidth = 256;
	int outHeight = 128;
	int inWidth = 0;
	int inHeight = 0;
	unsigned char *input;
	if(srcpath.substr(srcpath.size() - 3).find("jpg") != std::string::npos) {
		uJPEG ujpeg();
		ujpeg.decodeFile(srcpath.c_str());
		input = malloc(ujpeg.getImageSize());
		inWidth = ujpeg.getWidth();
		inHeight = ujpeg.getHeight();
		ujpeg.getImage(&input);
	} else if(srcpath.substr(srcpath.size() - 3).find("png") != std::string::npos) {
		lodepng_decode32_file(input, &inWidth, &inHeight, srcpath.c_str());
	}
	// calculate what to scale to while preserving aspect ratio
	float scale = min((float)outWidth / inWidth, (float)outHeight / inHeight);
	// alloc a buffer that can fit that
	u32 *buffer = malloc((inWidth * scale + inHeight * scale) * sizeof(u32));
	// bilinear scale the image, note this might need to be done per subpixel?
	bilinearResize(input, inWidth, inHeight, inWidth * scale, inHeight * scale, buffer);
	free(input);

	// alloc another buffer the size we want to output
	unsigned char image = malloc(outWidth * outHeight * 4);
	// copy centered to our new buffer
	int x = (outWidth - inWidth * scale) / 2;
	int yBase = (outHeight - inHeight * scale) / 2;
	for(int y = yBase; y < yBase + inHeight * scale; y++) {
		memcpy(image + y * outWidth + x, buffer + y * inWidth * scale, Width * scale);
	}
	free(buffer);

	// encode png
	lodepng_encode32_file(filename, image, width, height);
	free(image);
}
