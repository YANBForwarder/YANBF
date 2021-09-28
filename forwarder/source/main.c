#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main()
{
	romfsInit();
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	mkdir("sdmc:/_nds/CTR-NDSForwarder", 0777);

	printf("CTR-mode NDS Forwarder\n");
	printf("Loading...");
	
	char *line = NULL;
	size_t length = 0;

	FILE *file = fopen("romfs:/path.txt", "r");
	if(file) {
		while(__getline(&line, &length, file) != -1) {
			FILE *path = fopen("sdmc:/_nds/CTR-NDSForwarder/path.txt", "w");
			fputs(line, path);
			fputs("\n", path);
			printf("%s\n", line);
			fclose(path);
			fclose(file);
		}
	}
	// Used to debug above function actually working
	FILE *path = fopen("sdmc:/_nds/CTR-NDSForwarder/path.txt", "r");
	if(path) {
		while(__getline(&line, &length, path) != -1) {
			printf("%s", line);
		}
	}
	fclose(path);
	
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();

		aptSetChainloader(0x0004800546574452, 0); // Bootstrap Title ID
		break;
	}

	gfxExit();
	return 0;
}
