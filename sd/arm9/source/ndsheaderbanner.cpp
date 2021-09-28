#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

#include "ndsheaderbanner.h"

static u32 arm9Sig[3][4];
sNDSHeaderExt ndsHeader;

// Subroutine function signatures arm9
u32 moduleParamsSignature[2]   = {0xDEC00621, 0x2106C0DE};

//
// Look in @data for @find and return the position of it.
//
/*u32 getOffset(u32* addr, size_t size, u32* find, size_t sizeofFind, int direction)
{
	u32* end = addr + size/sizeof(u32);

    //u32 result = 0;
	bool found = false;

	do {
		for(int i=0;i<(int)sizeofFind;i++) {
			if (addr[i] != find[i]) 
			{
				break;
			} else if(i==(int)sizeofFind-1) {
				found = true;
			}
		}
		if(!found) addr+=direction;
	} while (addr != end && !found);

	if (addr == end) {
		return NULL;
	}

	return (u32)addr;
}*/

//char arm9binary[0x20000];

bool checkDsiBinaries(FILE* ndsFile) {
	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);

	if (ndsHeader.unitCode == 0) {
		return true;
	}

	fseek(ndsFile, 0x8000, SEEK_SET);
	fread(arm9Sig[0], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm9iromOffset, SEEK_SET);
	fread(arm9Sig[1], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm7iromOffset, SEEK_SET);
	fread(arm9Sig[2], sizeof(u32), 4, ndsFile);
	for (int i = 1; i < 3; i++) {
		if (arm9Sig[i][0] == arm9Sig[0][0]
		 && arm9Sig[i][1] == arm9Sig[0][1]
		 && arm9Sig[i][2] == arm9Sig[0][2]
		 && arm9Sig[i][3] == arm9Sig[0][3]) {
			return false;
		}
		if (arm9Sig[i][0] == 0
		 && arm9Sig[i][1] == 0
		 && arm9Sig[i][2] == 0
		 && arm9Sig[i][3] == 0) {
			return false;
		}
		if (arm9Sig[i][0] == 0xFFFFFFFF
		 && arm9Sig[i][1] == 0xFFFFFFFF
		 && arm9Sig[i][2] == 0xFFFFFFFF
		 && arm9Sig[i][3] == 0xFFFFFFFF) {
			return false;
		}
	}

	return true;
}

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
/*u32 getSDKVersion(FILE* ndsFile) {
	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);
	
	fseek(ndsFile, ndsHeader.arm9romOffset, SEEK_SET);
	if(ndsHeader.arm9binarySize > 0x20000) ndsHeader.arm9binarySize = 0x20000;
	fread(&arm9binary, 1, ndsHeader.arm9binarySize, ndsFile);

	// Looking for moduleparams
	uint32_t moduleparams = getOffset((u32*)arm9binary, ndsHeader.arm9binarySize, (u32*)moduleParamsSignature, 2, 1);
	if(!moduleparams) {
		return 0;
	}

	return ((module_params_t*)(moduleparams - 0x1C))->sdk_version;
}*/

int checkIfHomebrew(FILE* ndsFile) {
	int res = 0;

	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);
	
	fseek(ndsFile, (ndsHeader.arm9romOffset <= 0x200 ? ndsHeader.arm9romOffset : ndsHeader.arm9romOffset+0x800), SEEK_SET);
	fread(arm9Sig[0], sizeof(u32), 4, ndsFile);
	if (arm9Sig[0][0] == 0xE3A00301
	 && arm9Sig[0][1] == 0xE5800208
	 && arm9Sig[0][2] == 0xE3A00013
	 && arm9Sig[0][3] == 0xE129F000) {
		res = 2; // Homebrew is recent (supports reading from SD without a DLDI driver)
		if (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000) {
			if ((ndsHeader.arm9binarySize == 0xC9F68 && ndsHeader.arm7binarySize == 0x12814)	// Colors! v1.1
			|| (ndsHeader.arm9binarySize == 0x1B0864 && ndsHeader.arm7binarySize == 0xDB50)	// Mario Paint Composer DS v2 (Bullet Bill)
			|| (ndsHeader.arm9binarySize == 0xD45C0 && ndsHeader.arm7binarySize == 0x2B7C)		// ikuReader v0.058
			|| (ndsHeader.arm9binarySize == 0x54620 && ndsHeader.arm7binarySize == 0x1538)		// XRoar 0.24fp3
			|| (ndsHeader.arm9binarySize == 0x2C9A8 && ndsHeader.arm7binarySize == 0xFB98)		// NitroGrafx v0.7
			|| (ndsHeader.arm9binarySize == 0x22AE4 && ndsHeader.arm7binarySize == 0xA764)) {	// It's 1975 and this man is about to show you the future
				res = 1; // Have nds-bootstrap load it (in case if it doesn't)
			}
		}
	} else if ((memcmp(ndsHeader.gameTitle, "NDS.TinyFB", 10) == 0)
			 || (memcmp(ndsHeader.gameTitle, "UNLAUNCH.DSI", 12) == 0)) {
		res = 2; // No need to use nds-bootstrap
	} else if ((memcmp(ndsHeader.gameTitle, "NMP4BOOT", 8) == 0)
	 || (ndsHeader.arm7executeAddress >= 0x037F0000 && ndsHeader.arm7destination >= 0x037F0000)) {
		res = 1; // Homebrew is old (requires a DLDI driver to read from SD)
	}

	return res;
}
