// all defines pulled from TWiLight Menu++
// https://github.com/DS-Homebrew/TWiLightMenu/blob/587a3ab0bd74d5f74f6eb1dd34ad9673b18c4606/romsel_dsimenutheme/arm9/source/ndsheaderbanner.h

/*---------------------------------------------------------------------------------

    This file is a part of libnds

	Copyright (C) 2005 Michael Noland (joat) and Jason Rogers (dovoto)

	SPDX-License-Identifier: Zlib

---------------------------------------------------------------------------------*/

#![allow(non_snake_case)]
#![allow(non_camel_case_types)]

struct sNDSHeaderExt {
	gameTitle: [u8; 12],		// 12 characters for the game title.
	gameCode: [u8; 4],			// 4 characters for the game code.
	makercode: [u8; 2],			// identifies the (commercial) developer.
	unitCode: u8,				// identifies the required hardware.
	deviceType: u8,				// type of device in the game card
	deviceSize: u8,				// capacity of the device (1 << n Mbit)
	reserved1: [u8; 9],
	romversion: u8,				// version of the ROM.
	flags: u8,					// bit 2: auto-boot flag.

	arm9romOffset: u32,			// offset of the arm9 binary in the nds file.
	arm9executeAddress: u32,	// adress that should be executed after the binary has been copied.
	arm9destination: u32,		// destination address to where the arm9 binary should be copied.
	arm9binarySize: u32,		// size of the arm9 binary.

	arm7romOffset: u32,			// offset of the arm7 binary in the nds file.
	arm7executeAddress: u32,	// adress that should be executed after the binary has been copied.
	arm7destination: u32,		// destination address to where the arm7 binary should be copied.
	arm7binarySize: u32,		// size of the arm7 binary.

	filenameOffset: u32,		// File Name Table (FNT) offset.
	filenameSize: u32,			// File Name Table (FNT) size.
	fatOffset: u32,				// File Allocation Table (FAT) offset.
	fatSize: u32,				// File Allocation Table (FAT) size.

	arm9overlaySource: u32,		// File arm9 overlay offset.
	arm9overlaySize: u32,		// File arm9 overlay size.
	arm7overlaySource: u32,		// File arm7 overlay offset.
	arm7overlaySize: u32,		// File arm7 overlay size.

	cardControl13: u32,			// Port 40001A4h setting for normal commands (used in modes 1 and 3)
	cardControlBF: u32,			// Port 40001A4h setting for KEY1 commands (used in mode 2)
	bannerOffset: u32,			// offset to the banner with icon and titles etc.

	secureCRC16: u16,			// Secure Area Checksum, CRC-16.

	readTimeout: u16,			// Secure Area Loading Timeout.

	unknownRAM1: u32,			// ARM9 Auto Load List RAM Address (?)
	unknownRAM2: u32,			// ARM7 Auto Load List RAM Address (?)

	bfPrime1: u32,				// Secure Area Disable part 1.
	bfPrime2: u32,				// Secure Area Disable part 2.
	romSize: u32,				// total size of the ROM.

	headerSize: u32,			// ROM header size.
	zeros88: [u32; 14],
	gbaLogo: [u8; 156],			// Nintendo logo needed for booting the game.
	logoCRC16: u16,				// Nintendo Logo Checksum, CRC-16.
	headerCRC16: u16,			// header checksum, CRC-16.

	debugRomSource: u32,		// debug ROM offset.
	debugRomSize: u32,			// debug size.
	debugRomDestination: u32,	// debug RAM destination.
	offset_0x16C: u32,			//reserved?

	zero: [u8; 0x10],
	mbk1: u32,
	mbk2: u32,
	mbk3: u32,
	mbk4: u32,
	mbk5: u32,
	a9mbk6: u32,
	a9mbk7: u32,
	a9mbk8: u32,
	a7mbk6: u32,
	a7mbk7: u32,
	a7mbk8: u32,
	mbk9: u32,
	region: u32,
	accessControl: u32,
	arm7SCFGSettings: u32,
	dsi_unk1: u16,
	dsi_unk2: u8,
	dsi_flags: u8,

	arm9iromOffset: u32,		// offset of the arm9 binary in the nds file.
	arm9iexecuteAddress: u32,
	arm9idestination: u32,		// destination address to where the arm9 binary should be copied.
	arm9ibinarySize: u32,		// size of the arm9 binary.

	arm7iromOffset: u32,		// offset of the arm7 binary in the nds file.
	deviceListDestination: u32,
	arm7idestination: u32,		// destination address to where the arm7 binary should be copied.
	arm7ibinarySize: u32,		// size of the arm7 binary.

	zero2: [u8; 0x20],

	// 0x200
	// TODO: More DSi-specific fields.
	dsi1: [u32; 0x10/4],
	twlRomSize: u32,
	dsi_unk3: u32,
	dsi_unk4: u32,
	dsi_unk5: u32,
	dsi2: [u8; 0x10],
	dsi_tid: u32,
	dsi_tid2: u32,
	pubSavSize: u32,
	prvSavSize: u32,
	dsi3: [u8; 0x174]
}

struct sNDSBannerExt {
	version: u16,		        // version of the banner.
	crc: [u16; 4],		        // CRC-16s of the banner.
	reserved: [u8; 22],
	icon: [u8; 512],		        // 32*32 icon of the game with 4 bit per pixel.
	palette: [u16; 16],	        // the palette of the icon.
	titles: [[u16;128];8],	    // title of the game in 8 different languages.

	// [0xA40] Reserved space, possibly for other titles.
	reserved2: [u8;0x800],

	// DSi-specific.
	dsi_icon: [[u8;512];512],	    // DSi animated icon frame data.
	dsi_palette: [[u16;16];8],	// Palette for each DSi icon frame.
	dsi_seq: [u16;64],	        // DSi animated icon sequence.
	reserved3: [u8;64]
}

// sNDSBanner version.
enum sNDSBannerVersion {
	NDS_BANNER_VER_ORIGINAL	= 0x0001,
	NDS_BANNER_VER_ZH		= 0x0002,
	NDS_BANNER_VER_ZH_KO	= 0x0003,
	NDS_BANNER_VER_DSi		= 0x0103,
}

// sNDSBanner sizes.
enum sNDSBannerSize {
	NDS_BANNER_SIZE_ORIGINAL	= 0x0840,
	NDS_BANNER_SIZE_ZH			= 0x0940,
	NDS_BANNER_SIZE_ZH_KO		= 0x0A40,
	NDS_BANNER_SIZE_DSi			= 0x23C0,
}
