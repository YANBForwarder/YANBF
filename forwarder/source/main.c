/*
	YANBF
    Copyright (C) 2022-present lifehackerhansol

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

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main() {
	romfsInit();
	nsInit();
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
		NS_RebootToTitle(MEDIATYPE_NAND, tid);
	}

	gfxInitDefault();
	for(u32 i = 0; i < 60; i++) gspWaitForVBlank();
	consoleInit(GFX_TOP, NULL);
	printf("Failed to launch CIA.\n\nPlease reinstall bootstrap.cia from\nYANBF release.\n\nPress START to exit.");
	while (aptMainLoop()) {
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		hidScanInput();

		if (hidKeysDown() & KEY_START) break;
	}
	romfsExit();
	nsExit();
	amExit();
	gfxExit();
	return 0;
}
