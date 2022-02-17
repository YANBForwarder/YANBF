/*
YANBF
Copyright (C) 2022-present lifehackerhansol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main() {
	romfsInit();
	gfxInitDefault();
	amInit();
	mkdir("sdmc:/_nds", 0777);
	mkdir("sdmc:/_nds/ntr-forwarder", 0777);
	
	char *line = NULL;
	size_t length = 0;

	FILE *file = fopen("romfs:/path.txt", "r");
	if(file) {
		while(__getline(&line, &length, file) != -1) {
			FILE *path = fopen("sdmc:/_nds/ntr-forwarder/path.txt", "w");
			fputs(line, path);
			fclose(path);
		}
	}
	fclose(file);
	AM_TitleEntry bootstrap;
	u64 tid = 0x0004800546574452;
	if(R_SUCCEEDED(AM_GetTitleInfo(MEDIATYPE_NAND, 1, &tid, &bootstrap))) {
		if(R_SUCCEEDED(APT_PrepareToDoApplicationJump(0, 0x0004800546574452, 0))) {
			u8 param[0x300];
			u8 hmac[0x20];
			APT_DoApplicationJump(param, sizeof(param), hmac);
		}
	}

	consoleInit(GFX_TOP, NULL);
	printf("Failed to launch CIA.\n\nPlease reinstall bootstrap.cia from\nYANBF release.\n\nPress START to exit.");
	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		if (hidKeysDown() & KEY_START) break;
	}
	amExit();
	gfxExit();
	return 0;
}
