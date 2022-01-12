#ifndef PER_GAME_SETTINGS_H
#define PER_GAME_SETTINGS_H

#include <string>

#include "inifile.h"

class GameSettings {
	std::string filepath;
	CIniFile ini;

public:
	GameSettings(const std::string &filename);

	int language = -2;
	int region = -3;
	int saveNo = 0;
	int dsiMode = -1;
	int boostCpu = -1;
	int boostVram = -1;
	int cardReadDMA = -1;
	int asyncCardRead = -1;
	int swiHaltHook = -1;
	int expandRomSpace = -1;
	int bootstrapFile = -1;

	void save(void);

	void menu(void);

};

#endif // PER_GAME_SETTINGS_H
