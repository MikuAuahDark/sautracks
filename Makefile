# Makefile

all: sautracks

sautracks:
	-mkdir bin\\mingw
	windres -O coff sautracks.rc sautracks.res
	g++ --std=c++11 -static-libgcc -static-libstdc++ -o bin/mingw/sautracks src/Main.cc src/UserTracks.cc src/WinMainWrapper.cc

vscmd:
	-mkdir bin\\vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c src\\Main.cc src\\UserTracks.cc src\\WinMainWrapper.cc
	rc -v -l 0 sautracks.rc
	link -OUT:"bin\\vscmd\\sautracks.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\sautracks.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE Main.obj UserTracks.obj WinMainWrapper.obj sautracks.res ComCtl32.lib shell32.lib user32.lib
	rm *.obj sautracks.res