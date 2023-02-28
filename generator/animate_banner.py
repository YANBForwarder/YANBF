import subprocess

ENDIANESS = "little"

def uint(bytes, endianess = "little"):
    return int.from_bytes(bytes, endianess, signed=False)

def printb(title, bytes):
    print(f"{title} = {bytes} = {uint(bytes)}")

def convert_png_to_etc1a4(png_path, out_dir = "data/"): #dir has to end with a separator
    print(f"Using arguments: 3dstex-win-x86.exe -r -o auto-etc1 {png_path} {out_dir}etc1.bin")
    subprocess.run(["3dstex-win-x86.exe", "-r", "-o", "auto-etc1", png_path, f"{out_dir}etc1.bin"])

def get_etc1a4_data_from_png(png_path, out_dir = "data/"): #dir has to end with a separator
    convert_png_to_etc1a4(png_path, out_dir)
    with open(f"{out_dir}etc1.bin", "r+b") as f:
        data = f.read()
    return data

def edit_bcmdl(new_etc1_data, bcmdl_path = "./data/banner/banner0.bcmdl"):
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