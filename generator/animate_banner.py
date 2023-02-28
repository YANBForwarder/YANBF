import os
import shutil
import subprocess

ENDIANESS = "little"

def uint(bytes, endianess = "little"):
    return int.from_bytes(bytes, endianess, signed=False)

def printb(title, bytes):
    print(f"{title} = {bytes} = {uint(bytes)}")

def convert_png_to_etc1a4(png_path, out_dir = ""): #dir has to end with a separator
    subprocess.run(["tools/3dstex-win-x86.exe", "-r", "-o", "auto-etc1", png_path, f"{out_dir}etc1.bin"])

def get_etc1a4_data_from_png(png_path, out_dir = ""): #dir has to end with a separator
    convert_png_to_etc1a4(png_path, out_dir)
    with open(f"{out_dir}etc1.bin", "r+b") as f:
        data = f.read()
    return data

def edit_bcmdl(new_etc1_data, bcmdl_path = "./data/banner/banner0.bcmdl"):
    #os.remove("./template_copy.bcmdl")
    #shutil.copy2("./banner0.bcmdl", "./template_copy.bcmdl")
    with open(bcmdl_path, "rb+") as f:
        f.seek(4 + 2 + 2 + 4, 0)
        pFileSizeBytes = f.tell()
        uiFileSizeBytes = uint(f.read(4))
        f.seek(4 + 4 + 4, 1)
        #DATA
        dictList = []
        for i in range(0, 15):
            dictList.append((f.read(4), f.tell() + uint(f.read(4))))
        f.seek(dictList[1][1], 0)
        #DICT1
        f.seek(4 + 4 + 4 + 4 + 2 + 0xA + 4 + 2 + 2 + 4, 1)
        uiObjectOffsetDict1 = f.tell() + uint(f.read(4))
        #TXOB
        f.seek(uiObjectOffsetDict1 + 0x44, 0)
        pTxobTextureDataSize = f.tell()
        uiTxobTextureDataSize = uint(f.read(4))
        f.seek(uiObjectOffsetDict1 + 0x48, 0)
        pTxobTextureDataOffset = f.tell() + uint(f.read(4))
        #TXOB DATA
        f.seek(pTxobTextureDataOffset, 0)
        f.write(new_etc1_data)
        #update sizes
        uiNewTxobSize = len(new_etc1_data)
        bNewTxobSize = uiNewTxobSize.to_bytes(4, ENDIANESS)
        uiNewTotalSize = uiFileSizeBytes - uiTxobTextureDataSize + uiNewTxobSize
        bNewTotalSize = uiNewTotalSize.to_bytes(4, ENDIANESS)
        #write new texture size to txob offset
        f.seek(pTxobTextureDataSize, 0)
        f.write(bNewTxobSize)
        #write new file size in header
        f.seek(pFileSizeBytes, 0)
        f.write(bNewTotalSize)

    #os.rename("./template_copy.bcmdl", bcmdl_path)

if __name__ == "__main__":
    etc1_data = get_etc1a4_data_from_png("./Unbenannt-2.png")
    edit_bcmdl(etc1_data)


#with open("./file.bin", "r+b") as img:
#    new_data = img.read()
#
#with open("./banner0.bcmdl", "r+b") as f:
#    print("opened")
#    #read_string(f, 0)
#    magic = f.read(4)
#    print(f"magic = {magic}")
#    #f.seek(4, 0)
#    endianess = f.read(2)
#    print(endianess)
#    if endianess == b"\xff\xfe":
#        ENDIANESS = "little"
#    else:
#        ENDIANESS = "big"
#    cfgwHeaderSize = f.read(2)
#    revision = f.read(4)
#    fileSizeBytes = f.read(4)
#    printb("fileSizeBytes", fileSizeBytes)
#    numberEntries = f.read(4)
#
#    headerDataOffset = f.tell()
#    magicData = f.read(4)
#    sizeDataBytes = f.read(4)
#    printb("sizeDataBytes", sizeDataBytes)
#    dictList = []
#    for i in range(0, 15):
#        dictList.append((f.read(4), f.tell() + uint(f.read(4))))
#    print(dictList)
#    print(dictList[1][1])
#
#    #textures dict
#    f.seek(dictList[1][1], 0)
#    magicDict1 = f.read(4)
#    #print(magicDict1)
#    sizeDict1Bytes = f.read(4)
#    printb("sizeDict1Bytes", sizeDict1Bytes)
#    numberEntriesDict1 = f.read(4)
#    #printb("numberEntriesDict1", numberEntriesDict1)
#    f.seek(0x4 + 0x2 + 0xA, 1)
#    f.seek(0x4 + 0x2 + 0x2, 1)
#    symbolOffsetDict1 = f.tell() + uint(f.read(4))
#    objectOffsetDict1 = f.tell() + uint(f.read(4))
#    #print(symbolOffsetDict1)
#    #print(objectOffsetDict1)
#
#    #txob from dict
#    f.seek(objectOffsetDict1, 0)
#    flags = f.read(4)
#    magicTxob = f.read(4)
#    #printb("magicTxob", magicTxob)
#    f.seek(8, 1)
#    symbolOffsetTxob = f.tell() + uint(f.read(4))
#    f.seek(4, 1)
#    txobTextureHeight = f.read(4)
#    #printb("txobTextureHeight", txobTextureHeight)
#    txobTextureWidth = f.read(4)
#    #printb("txobTextureWidth", txobTextureWidth)
#    f.seek(objectOffsetDict1 + 0x28, 0)
#    txobMipmapLevels = f.read(4)
#    #printb("txobMipmapLevels", txobMipmapLevels)
#    f.seek(objectOffsetDict1 + 0x34, 0)
#    txobTextureFormat = f.read(4)
#    #printb("txobTextureFormat", txobTextureFormat)
#    f.seek(objectOffsetDict1 + 0x3c, 0)
#    txobTextureHeight2 = f.read(4)
#    #printb("txobTextureHeight2", txobTextureHeight2)
#    f.seek(objectOffsetDict1 + 0x40, 0)
#    txobTextureWidth2 = f.read(4)
#    #printb("txobTextureWidth2", txobTextureWidth2)
#    f.seek(objectOffsetDict1 + 0x44, 0)
#    txobTextureDataSize = f.read(4)
#    printb("txobTextureDataSize", txobTextureDataSize)
#    f.seek(objectOffsetDict1 + 0x48, 0)
#    txobTextureDataOffset = f.tell() + uint(f.read(4))
#    print(f"txobTextureDataOffset = {txobTextureDataOffset}")
#
#
#    f.seek(txobTextureDataOffset, 0)
#    
#    #write new_data
#    f.write(new_data)
#    #get new_data size
#    new_txob_size = new_data.__sizeof__()
#    old_total_size = uint(fileSizeBytes)
#    old_txob_size = uint(txobTextureDataSize)
#    new_total_size = old_total_size - old_txob_size + new_txob_size
#    new_total_size_bytes = new_total_size.to_bytes(4, ENDIANESS)
#    #write new data to txob offset
#    f.seek(objectOffsetDict1 + 0x44, 0)
#    f.write(new_txob_size.to_bytes(4, ENDIANESS))
#    #change file size in header
#    f.seek(4 + 2 + 2 + 4, 0)
#    f.write(new_total_size_bytes)
#
#
#
#    #forwardCompatibility = f.read(1)
#    #u16version = int.from_bytes(f.read(2), ENDIANESS, signed=False)
#    #print(u16version)
#    #u32mainHeaderOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #u32stringTableOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #u32gpuCommandsOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #u32dataOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #if backwardCompatibility > b"\x20":
#    #    u32dataExtendedOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #else:
#    #    u32dataExtendedOffset = 0
#    #u32relocationTableOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)
##
##
##
#    #f.seek(u32mainHeaderOffset + 0x24, 0)
#    #u32texturesPointerTableOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False) + u32mainHeaderOffset
#    #u32texturesPointerTableEntries = int.from_bytes(f.read(4), ENDIANESS, signed=False)
#    #print(u32mainHeaderOffset)
#    #print(u32texturesPointerTableEntries)
#
#
#    #u32dataExtendedOffset = int.from_bytes(f.read(4), ENDIANESS, signed=False)

