#!/usr/bin/env python3

from cx_Freeze import setup, Executable

# Dependencies are automatically detected, but it might need fine tuning.
# "packages": ["os"] is used as example only
build_exe_options = {"packages": ["PIL", "requests"], "excludes": ["tkinter, PyQt5"], 'build_exe': "dist"}

# base="Win32GUI" should be used only for Windows GUI app
base = None

setup(
    name="generator",
    version="1.5.0",
    description="YANBF Generator",
    options={"build_exe": build_exe_options},
    executables=[Executable("generator.py", base=base)],
)
