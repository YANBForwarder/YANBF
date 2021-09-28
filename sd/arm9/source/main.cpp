#include <nds.h>
#include <nds/fifocommon.h>
#include <stdio.h>
#include <fat.h>
#include "fat_ext.h"
#include <sys/stat.h>
#include <limits.h>
#include <nds/disc_io.h>

#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "ndsheaderbanner.h"
#include "nds_loader_arm9.h"
#include "nitrofs.h"
#include "tonccpy.h"

#include "inifile.h"
#include "fileCopy.h"

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "donorMap.h"
#include "saveMap.h"
#include "ROMList.h"

using namespace std;

static bool boostCpu = false;
static bool boostVram = false;
static int dsiMode = 0;
static int language = -1;
static int region = -2;
static bool cacheFatTable = false;

static bool bootstrapFile = false;

static bool consoleInited = false;

bool extention(const std::string& filename, const char* ext) {
	if(strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

/**
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed(const char* filename) {
	//if (perGameSettings_boostCpu == -1) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		fseek(f_nds_file, 0xC, SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		game_TID[4] = 0;
		fclose(f_nds_file);

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(game_TID, twlClockExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	//}

	return true;
}

/**
 * Disable card read DMA for a specific game.
 */
bool setCardReadDMA(const char* filename) {
	scanKeys();
	if(keysHeld() & KEY_L) {
		return false;
	}

	//if (perGameSettings_cardReadDMA == -1) {
		FILE *f_nds_file = fopen(filename, "rb");

		char game_TID[5];
		fseek(f_nds_file, 0xC, SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		game_TID[4] = 0;
		fclose(f_nds_file);

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(game_TID, cardReadDMAExcludeList[i], 3) == 0) {
				// Found match
				return false;
			}
		}
	//}

	return true;
}

/**
 * Disable asynch card read for a specific game.
 */
bool setAsyncCardRead(const char* filename) {
	scanKeys();
	if(keysHeld() & KEY_R) {
		return true;
	}

	return false;
}

/**
 * Set donor SDK version for a specific game.
 */
int SetDonorSDK(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, 0xC, SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);
	
	for (auto i : donorMap) {
		if (i.first == 5 && game_TID[0] == 'V')
			return 5;

		if (i.second.find(game_TID) != i.second.cend())
			return i.first;
	}

	return 0;
}

/**
 * Fix AP for some games.
 */
std::string setApFix(const char *filename) {
	bool useTwlmPath = (access("sd:/_nds/TWiLightMenu/extras/apfix.pck", F_OK) == 0);

	char game_TID[5];
	u16 headerCRC16 = 0;

	bool ipsFound = false;
	bool cheatVer = true;
	char ipsPath[256];
	snprintf(ipsPath, sizeof(ipsPath), "sd:/_nds/%s/apfix/%s.ips", (useTwlmPath ? "TWiLightMenu/extras" : "ntr-forwarder"), filename);
	ipsFound = (access(ipsPath, F_OK) == 0);
	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "sd:/_nds/%s/apfix/%s.bin", (useTwlmPath ? "TWiLightMenu/extras" : "ntr-forwarder"), filename);
		ipsFound = (access(ipsPath, F_OK) == 0);
	} else {
		cheatVer = false;
	}

	if (!ipsFound) {
		FILE *f_nds_file = fopen(filename, "rb");

		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, headerCRC16), SEEK_SET);
		fread(&headerCRC16, sizeof(u16), 1, f_nds_file);
		fclose(f_nds_file);
		game_TID[4] = 0;

		snprintf(ipsPath, sizeof(ipsPath), "sd:/_nds/%s/apfix/%s-%X.ips", (useTwlmPath ? "TWiLightMenu/extras" : "ntr-forwarder"), game_TID, headerCRC16);
		ipsFound = (access(ipsPath, F_OK) == 0);
		if (!ipsFound) {
			snprintf(ipsPath, sizeof(ipsPath), "sd:/_nds/%s/apfix/%s-%X.bin", (useTwlmPath ? "TWiLightMenu/extras" : "ntr-forwarder"), game_TID, headerCRC16);
			ipsFound = (access(ipsPath, F_OK) == 0);
		} else {
			cheatVer = false;
		}
	}

	if (ipsFound) {
		return ipsPath;
	}

	useTwlmPath = (access("sd:/_nds/TWiLightMenu", F_OK) == 0);

	FILE *file = fopen(useTwlmPath ? "sd:/_nds/TWiLightMenu/extras/apfix.pck" : "sd:/_nds/ntr-forwarder/apfix.pck", "rb");
	if (file) {
		char buf[5] = {0};
		fread(buf, 1, 4, file);
		if (strcmp(buf, ".PCK") != 0) // Invalid file
			return "";

		u32 fileCount;
		fread(&fileCount, 1, sizeof(fileCount), file);

		u32 offset = 0, size = 0;

		// Try binary search for the game
		int left = 0;
		int right = fileCount;

		while (left <= right) {
			int mid = left + ((right - left) / 2);
			fseek(file, 16 + mid * 16, SEEK_SET);
			fread(buf, 1, 4, file);
			int cmp = strcmp(buf,  game_TID);
			if (cmp == 0) { // TID matches, check CRC
				u16 crc;
				fread(&crc, 1, sizeof(crc), file);

				if (crc == headerCRC16) { // CRC matches
					fread(&offset, 1, sizeof(offset), file);
					fread(&size, 1, sizeof(size), file);
					cheatVer = fgetc(file) & 1;
					break;
				} else if (crc < headerCRC16) {
					left = mid + 1;
				} else {
					right = mid - 1;
				}
			} else if (cmp < 0) {
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if (offset > 0 && size > 0) {
			fseek(file, offset, SEEK_SET);
			u8 *buffer = new u8[size];
			fread(buffer, 1, size, file);

			mkdir("sd:/_nds/nds-bootstrap", 0777);
			snprintf(ipsPath, sizeof(ipsPath), "sd:/_nds/nds-bootstrap/apFix%s", cheatVer ? "Cheat.bin" : ".ips");
			FILE *out = fopen(ipsPath, "wb");
			if(out) {
				fwrite(buffer, 1, size, out);
				fclose(out);
			}
			delete[] buffer;
			fclose(file);
			return ipsPath;
		}

		fclose(file);
	}

	return "";
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause() {
//---------------------------------------------------------------------------------
	iprintf("Press start...\n");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string ndsPath;
std::string romfolder;
std::string filename;
std::string savename;
std::string romFolderNoSlash;
std::string savepath;

std::vector<char*> argarray;

static inline void takeWhileMsg() {
	#ifdef DSI
	iprintf ("If this takes a while, close\n");
	iprintf ("and open the console's lid.\n");
	#else
	iprintf ("If this takes a while,\n");
	iprintf ("press HOME, and press B.\n");
	#endif
	iprintf ("\n");
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	defaultExceptionHandler();

	// Cut slot1 power to save battery
	disableSlot1();
	consoleDemoInit();
	iprintf("Starting.\n");
	if (fatInitDefault()) {
		FILE* pathtxt = fopen("/_nds/CTR-NDSForwarder/path.txt", "r");
		char path[255];
		if(fgets(path, 255, pathtxt)==NULL) {
			consoleDemoInit();
			iprintf("Failed. Unknown\n");
		} else if (access(path, F_OK) != 0) {
			consoleDemoInit();
			iprintf("Not found:\n%s\n\n", argv[1]);
			iprintf("Please recreate the forwarder\n");
			iprintf("with the correct ROM path.\n");
		} else {
		nitroFSInit("/_nds/CTR-NDSForwarder/sdcard.nds");

		ndsPath = (std::string)path;
		iprintf("Working???\n");
		CIniFile ntrforwarderini( "sd:/_nds/ntr_forwarder.ini" );

		bootstrapFile = ntrforwarderini.GetInt("NTR-FORWARDER", "BOOTSTRAP_FILE", 0);

		printf(path);
		printf("\n");
		printf("Press START\n");
		while (1) {
			scanKeys();
			if (keysDown() & KEY_START) break;
			swiWaitForVBlank();
		}

		romfolder = ndsPath;
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		chdir(romfolder.c_str());

		filename = ndsPath;
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx)
		{
			filename.erase(0, last_slash_idx + 1);
		}

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		bool dsiBinariesFound = checkDsiBinaries(f_nds_file);
		int isHomebrew = checkIfHomebrew(f_nds_file);
		bool isDSiWare = false;

		if (isHomebrew == 0) {
			mkdir ("saves", 0777);
		}

		fclose(f_nds_file);

		extern sNDSHeaderExt ndsHeader;

		if ((ndsHeader.gameCode[0] == 0x48 && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x4B && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x5A && ndsHeader.makercode[0] != 0 && ndsHeader.makercode[1] != 0)
		 || (ndsHeader.gameCode[0] == 0x42 && ndsHeader.gameCode[1] == 0x38 && ndsHeader.gameCode[2] == 0x38))
		{ if (ndsHeader.unitCode != 0)
			isDSiWare = true; // Is a DSiWare game
		}

		argarray.push_back(strdup("NULL"));

		if (isHomebrew == 2) {
			argarray.at(0) = argv[1];
			int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
			if (!consoleInited) {
				consoleDemoInit();
				consoleInited = true;
			}
			iprintf("Start failed. Error %i\n", err);
			if (err == 1) iprintf ("ROM not found.\n");
		} else {
			mkdir ("sd:/_nds/nds-bootstrap", 0777);

			FILE *headerFile = fopen("sd:/_nds/ntr-forwarder/header.bin", "rb");
			FILE *srBackendFile = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
			fread(__DSiHeader, 1, 0x1000, headerFile);
			fwrite((char*)__DSiHeader+0x230, sizeof(u32), 2, srBackendFile);
			fclose(headerFile);
			fclose(srBackendFile);
			// Delete cheat data
			remove("sd:/_nds/nds-bootstrap/cheatData.bin");
			remove("sd:/_nds/nds-bootstrap/wideCheatData.bin");

			const char *typeToReplace = ".nds";
			if (extention(filename, ".dsi")) {
				typeToReplace = ".dsi";
			} else if (extention(filename, ".ids")) {
				typeToReplace = ".ids";
			} else if (extention(filename, ".srl")) {
				typeToReplace = ".srl";
			} else if (extention(filename, ".app")) {
				typeToReplace = ".app";
			}

			savename = ReplaceAll(filename, typeToReplace, ".sav");
			romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			savepath = romFolderNoSlash+"/saves/"+savename;

			std::string dsiWareSrlPath = ndsPath;
			std::string dsiWarePubPath = ReplaceAll(savepath, ".sav", ".pub");
			std::string dsiWarePrvPath = ReplaceAll(savepath, ".sav", ".prv");

			if (isDSiWare) {
				if ((getFileSize(dsiWarePubPath.c_str()) == 0) && (ndsHeader.pubSavSize > 0)) {
					consoleDemoInit();
					iprintf("Creating public save file...\n");
					iprintf ("\n");
					takeWhileMsg();

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr",
						 (unsigned int)ndsHeader.pubSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, ndsHeader.pubSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					iprintf("Public save file created!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
				}

				if ((getFileSize(dsiWarePrvPath.c_str()) == 0) && (ndsHeader.prvSavSize > 0)) {
					consoleDemoInit();
					iprintf("Creating private save file...\n");
					iprintf ("\n");
					takeWhileMsg();

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%x.savhdr",
						 (unsigned int)ndsHeader.prvSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, ndsHeader.prvSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					iprintf("Private save file created!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
				}
			} else if (isHomebrew == 0 && (strncmp(ndsHeader.gameCode, "NTR", 3) != 0)) {
				u32 orgsavesize = getFileSize(savepath.c_str());
				u32 savesize = 524288; // 512KB (default size for most games)

				u32 gameTidHex = 0;
				tonccpy(&gameTidHex, &ndsHeader.gameCode, 4);

				for (int i = 0; i < (int)sizeof(ROMList)/12; i++) {
					ROMListEntry* curentry = &ROMList[i];
					if (gameTidHex == curentry->GameCode) {
						if (curentry->SaveMemType != 0xFFFFFFFF) savesize = sramlen[curentry->SaveMemType];
						break;
					}
				}

				bool saveSizeFixNeeded = false;

				// TODO: If the list gets large enough, switch to bsearch().
				for (unsigned int i = 0; i < sizeof(saveSizeFixList) / sizeof(saveSizeFixList[0]); i++) {
					if (memcmp(ndsHeader.gameCode, saveSizeFixList[i], 3) == 0) {
						// Found a match.
						saveSizeFixNeeded = true;
						break;
					}
				}

				if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize && saveSizeFixNeeded)) {
					consoleDemoInit();
					iprintf ((orgsavesize == 0) ? "Creating save file...\n" : "Expanding save file...\n");
					iprintf ("\n");
					takeWhileMsg();

					if (orgsavesize > 0) {
						fsizeincrease(savepath.c_str(), "sd:/temp.sav", savesize);
					} else {
						FILE *pFile = fopen(savepath.c_str(), "wb");
						if (pFile) {
							fseek(pFile, savesize - 1, SEEK_SET);
							fputc('\0', pFile);
							fclose(pFile);
						}
					}
					iprintf ("Done!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
				}
			}

			if (isHomebrew == 0 && (ndsHeader.unitCode == 2 && !dsiBinariesFound)) {
				consoleDemoInit();
				iprintf ("The DSi binaries are missing.\n");
				iprintf ("Please obtain a clean ROM\n");
				iprintf ("to replace the current one.\n");
				iprintf ("\n");
				iprintf ("Press A to proceed to run in\n");
				iprintf ("DS mode.\n");
				while (1) {
					scanKeys();
					if (keysDown() & KEY_A) break;
					swiWaitForVBlank();
				}
			}

			int donorSdkVer = 0;
			bool dsModeForced = false;

			if (isHomebrew == 0) {
				if (ndsHeader.unitCode > 0 && ndsHeader.unitCode < 3) {
					scanKeys();
					dsModeForced = (keysHeld() & KEY_Y);
				}
				donorSdkVer = SetDonorSDK(ndsPath.c_str());
			}

			char sfnSrl[62];
			char sfnPub[62];
			char sfnPrv[62];
			if (isDSiWare) {
				fatGetAliasPath("sd:/", dsiWareSrlPath.c_str(), sfnSrl);
				fatGetAliasPath("sd:/", dsiWarePubPath.c_str(), sfnPub);
				fatGetAliasPath("sd:/", dsiWarePrvPath.c_str(), sfnPrv);
			}

			CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
			// Fix weird bug where some settings would be cleared
			boostCpu = bootstrapini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
			boostVram = bootstrapini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
			dsiMode = bootstrapini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", dsiMode);
			cacheFatTable = bootstrapini.GetInt("NDS-BOOTSTRAP", "CACHE_FAT_TABLE", cacheFatTable);
			language = bootstrapini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", language);
			region = bootstrapini.GetInt("NDS-BOOTSTRAP", "REGION", region);

			// Write
			bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ndsPath);
			if (isDSiWare) {
				bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
				bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
			} else {
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
			}
			if (isHomebrew == 0) {
				bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", isDSiWare ? "" : setApFix(filename.c_str()));
			}
			bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", "");
			bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", setClockSpeed(filename.c_str()));
			//bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", setCardReadDMA(filename.c_str()));
			bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", setAsyncCardRead(filename.c_str()));
			if (dsModeForced || ndsHeader.unitCode == 0 || (ndsHeader.unitCode > 0 && ndsHeader.unitCode < 3 && !dsiBinariesFound)) {
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
			} else {
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 1);
			}
			//bootstrapini.SetInt("NDS-BOOTSTRAP", "CACHE_FAT_TABLE", cacheFatTable);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", 0);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", 0);
			#ifdef DSI
			bootstrapini.SetInt("NDS-BOOTSTRAP", "CONSOLE_MODEL", 0);
			#else
			bootstrapini.SetInt("NDS-BOOTSTRAP", "CONSOLE_MODEL", 2);
			#endif
			bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", language);
			bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", region);
			bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );

			if (isHomebrew == 1) {
				argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds");
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
				if (!consoleInited) {
					consoleDemoInit();
					consoleInited = true;
				}
				iprintf("Start failed. Error %i\n", err);
				if (err == 1) iprintf ("nds-bootstrap (hb) not found.\n");
			} else {
				argarray.at(0) = (char*)(bootstrapFile ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds");
				int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
				if (!consoleInited) {
					consoleDemoInit();
					consoleInited = true;
				}
				iprintf("Start failed. Error %i\n", err);
				if (err == 1) iprintf ("nds-bootstrap not found.\n");
			}
		}
		}
	} else {
		// Subscreen as a console
		videoSetModeSub(MODE_0_2D);
		vramSetBankH(VRAM_H_SUB_BG);
		consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);	

		iprintf ("fatinitDefault failed!\n");
	}

	iprintf ("\n");		
	iprintf ("Press B to return to\n");
	#ifdef DSI
	iprintf ("DSi Menu.\n");
	#else
	iprintf ("HOME Menu.\n");
	#endif

	while (1) {
		scanKeys();
		if (keysDown() & KEY_B) fifoSendValue32(FIFO_USER_01, 1);	// Tell ARM7 to reboot into 3DS HOME Menu (power-off/sleep mode screen skipped)
		swiWaitForVBlank();
	}

	return 0;
}
