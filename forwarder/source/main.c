#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main()
{
	romfsInit();
	gfxInitDefault();
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

	gspWaitForVBlank();
	gfxSwapBuffers();
	aptSetChainloader(0x0004800546574452, 0); // Bootstrap Title ID

	gfxExit();
	return 0;
}
