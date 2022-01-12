#include "perGameSettings.h"

#include <nds.h>
#include <dirent.h>

constexpr std::array<const char *, 3> offOnLabels = {"Default", "Off", "On"};
constexpr std::array<const char *, 10> languageLabels = {"Default", "System", "Japanese", "English", "French", "German", "Italian", "Spanish", "Chinese", "Korean"};
constexpr std::array<const char *, 9> regionLabels = {"Default", "Per-game", "System", "Japan", "USA", "Europe", "Australia", "China", "Korea"};
constexpr std::array<const char *, 4> runInLabels = {"Default", "DS Mode", "Auto", "DSi Mode"};
constexpr std::array<const char *, 3> cpuLabels = {"Default", "67 MHz (NTR)", "133 MHz (TWL)"};
constexpr std::array<const char *, 3> vramLabels = {"Default", "DS Mode", "DSi Mode"};
constexpr std::array<const char *, 4> expandLabels = {"Default", "No", "Yes", "Yes+512 KB"};
constexpr std::array<const char *, 3> bootstrapLabels = {"Default", "Release", "Nightly"};

GameSettings::GameSettings(const std::string &filename) : filepath("sd:/_nds/ntr-forwarder/gamesettings/" + filename + ".ini"), ini(filepath) {
	language = ini.GetInt("GAMESETTINGS", "LANGUAGE", language);
	region = ini.GetInt("GAMESETTINGS", "REGION", region);
	saveNo = ini.GetInt("GAMESETTINGS", "SAVE_NUMBER", saveNo);
	dsiMode = ini.GetInt("GAMESETTINGS", "DSI_MODE", dsiMode);
	boostCpu = ini.GetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
	boostVram = ini.GetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
	cardReadDMA = ini.GetInt("GAMESETTINGS", "CARD_READ_DMA", cardReadDMA);
	asyncCardRead = ini.GetInt("GAMESETTINGS", "ASYNC_CARD_READ", asyncCardRead);
	swiHaltHook = ini.GetInt("GAMESETTINGS", "SWI_HALT_HOOK", swiHaltHook);
	expandRomSpace = ini.GetInt("GAMESETTINGS", "EXTENDED_MEMORY", expandRomSpace);
	bootstrapFile = ini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", bootstrapFile);
}

void GameSettings::save() {
	ini.SetInt("GAMESETTINGS", "LANGUAGE", language);
	ini.SetInt("GAMESETTINGS", "REGION", region);
	ini.SetInt("GAMESETTINGS", "SAVE_NUMBER", saveNo);
	ini.SetInt("GAMESETTINGS", "DSI_MODE", dsiMode);
	ini.SetInt("GAMESETTINGS", "BOOST_CPU", boostCpu);
	ini.SetInt("GAMESETTINGS", "BOOST_VRAM", boostVram);
	ini.SetInt("GAMESETTINGS", "CARD_READ_DMA", cardReadDMA);
	ini.SetInt("GAMESETTINGS", "ASYNC_CARD_READ", asyncCardRead);
	ini.SetInt("GAMESETTINGS", "SWI_HALT_HOOK", swiHaltHook);
	ini.SetInt("GAMESETTINGS", "EXTENDED_MEMORY", expandRomSpace);
	ini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", bootstrapFile);

	// Ensure the folder exists
	if(access("sd:/_nds", F_OK) != 0)
		mkdir("sd:/_nds", 0777);
	if(access("sd:/_nds/ntr-forwarder", F_OK) != 0)
		mkdir("sd:/_nds/ntr-forwarder", 0777);
	if(access("sd:/_nds/ntr-forwarder/gamesettings", F_OK) != 0)
		mkdir("sd:/_nds/ntr-forwarder/gamesettings", 0777);

	ini.SaveIniFile(filepath);
}

void GameSettings::menu() {
	consoleDemoInit();

	u16 held;
	int cursorPosition = 0;
	while(1) {
		consoleClear();
		iprintf("ntr-forwarder\n\n");
		iprintf("  Language: %s\n", languageLabels[language + 2]);
		iprintf("  Region: %s\n", regionLabels[region + 3]);
		iprintf("  Save Number: %d\n", saveNo);
		iprintf("  Run in: %s\n", runInLabels[dsiMode + 1]);
		iprintf("  ARM9 CPU Speed: %s\n", cpuLabels[boostCpu + 1]);
		iprintf("  VRAM Mode: %s\n", vramLabels[boostVram + 1]);
		iprintf("  Card Read DMA: %s\n", offOnLabels[cardReadDMA + 1]);
		iprintf("  Async Card Read: %s\n", offOnLabels[asyncCardRead + 1]);
		iprintf("  SWI Halt Hook: %s\n", offOnLabels[swiHaltHook + 1]);
		iprintf("  Expand ROM in RAM: %s\n", expandLabels[expandRomSpace + 1]);
		iprintf("  Bootstrap File: %s\n", bootstrapLabels[bootstrapFile + 1]);
		iprintf("\nPress <B> to cancel,\n<START> to save\n");

		// Print cursor
		iprintf("\x1b[%d;0H>", 2 + cursorPosition);

		do {
			scanKeys();
			held = keysDownRepeat();

			swiWaitForVBlank();
		} while(!held);

		if(held & KEY_UP) {
			cursorPosition--;
			if(cursorPosition < 0)
				cursorPosition = 10;
		} else if(held & KEY_DOWN) {
			cursorPosition++;
			if(cursorPosition > 10)
				cursorPosition = 0;
		} else if(held & (KEY_LEFT | KEY_A)) {
			switch(cursorPosition) {
				case 0:
					language--;
					if(language < -2) language = 7;
					break;
				case 1:
					region--;
					if(region < -3) region = 5;
					break;
				case 2:
					saveNo--;
					if(saveNo < 0) saveNo = 9;
					break;
				case 3:
					dsiMode--;
					if(dsiMode < -1) dsiMode = 2;
					break;
				case 4:
					boostCpu--;
					if(boostCpu < -1) boostCpu = 1;
					break;
				case 5:
					boostVram--;
					if(boostVram < -1) boostVram = 1;
					break;
				case 6:
					cardReadDMA--;
					if(cardReadDMA < -1) cardReadDMA = 1;
					break;
				case 7:
					asyncCardRead--;
					if(asyncCardRead < -1) asyncCardRead = 1;
					break;
				case 8:
					swiHaltHook--;
					if(swiHaltHook < -1) swiHaltHook = 1;
					break;
				case 9:
					expandRomSpace--;
					if(expandRomSpace < -1) expandRomSpace = 2;
					break;
				case 10:
					bootstrapFile--;
					if(bootstrapFile < -1) bootstrapFile = 1;
					break;
			}
		} else if(held & KEY_RIGHT) {
			switch(cursorPosition) {
				case 0:
					language++;
					if(language > 7) language = -2;
					break;
				case 1:
					region++;
					if(region > 5) region = -3;
					break;
				case 2:
					saveNo++;
					if(saveNo > 9) saveNo = 0;
					break;
				case 3:
					dsiMode++;
					if(dsiMode > 2) dsiMode = -1;
					break;
				case 4:
					boostCpu++;
					if(boostCpu > 1) boostCpu = -1;
					break;
				case 5:
					boostVram++;
					if(boostVram > 1) boostVram = -1;
					break;
				case 6:
					cardReadDMA++;
					if(cardReadDMA > 1) cardReadDMA = -1;
					break;
				case 7:
					asyncCardRead++;
					if(asyncCardRead > 1) asyncCardRead = -1;
					break;
				case 8:
					swiHaltHook++;
					if(swiHaltHook > 1) swiHaltHook = -1;
					break;
				case 9:
					expandRomSpace++;
					if(expandRomSpace > 2) expandRomSpace = -1;
					break;
				case 10:
					bootstrapFile++;
					if(bootstrapFile > 1) bootstrapFile = -1;
					break;
			}
		} else if(held & KEY_B) {
			return;
		} else if(held & KEY_START) {
			save();
			return;
		}
	}
}
