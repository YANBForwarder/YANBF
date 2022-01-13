#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main() {
	romfsInit();
	gfxInitDefault();
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
			fclose(file);
		}
	}
 
    Result res = 0;

    if(R_SUCCEEDED(res = APT_PrepareToDoApplicationJump(0, 0x0004800546574452, 0))) {
        u8 param[0x300];
        u8 hmac[0x20];

        res = APT_DoApplicationJump(param, sizeof(param), hmac);
    }

	consoleInit(GFX_TOP, NULL);
	printf("Failed to launch CIA.\nPlease reinstall bootstrap.cia from CTR-NDSForwarder release.\nPress START to exit.");
	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}
	gfxExit();
	return 0;
}
