# Makefile

all: sautracks

sautracks:
	-mkdir -p bin/mingw
	windres -O coff sautracks.rc sautracks.res
	g++ --std=gnu++0x -static-libgcc -static-libstdc++ -o bin/mingw/sautracks -Dstrcmpi=_stricmp -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501  src/Main.cc src/UserTracks.cc src/WinMainWrapper.cc sautracks.res -lcomctl32 -lshell32 -lgdi32 -lcomdlg32
	rm sautracks.res

noconsole:
	-mkdir -p bin/mingw
	windres -O coff sautracks.rc sautracks.res
	g++ --std=gnu++0x -static-libgcc -static-libstdc++ -mwindows -o bin/mingw/sautracks -Dstrcmpi=_stricmp -D_WIN32_IE=0x0500 -D_WIN32_WINNT=0x0501 -DSAUTRACKS_NOLOGS src/Main.cc src/UserTracks.cc src/WinMainWrapper.cc sautracks.res -lcomctl32 -lshell32 -lgdi32 -lcomdlg32
	rm sautracks.res

vscmd:
	-mkdir -p bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c src\\Main.cc src\\UserTracks.cc src\\WinMainWrapper.cc
	rc -v -l 0 sautracks.rc
	link -OUT:"bin\\vscmd\\sautracks.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\sautracks.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE Main.obj UserTracks.obj WinMainWrapper.obj sautracks.res ComCtl32.lib shell32.lib user32.lib comdlg32.lib Gdi32.lib
	rm *.obj sautracks.res

vscmd_noconsole:
	-mkdir -p bin/vscmd
	cl -W3 -Zc:wchar_t -Ox -D"_CRT_SECURE_NO_WARNINGS" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c src\\Main.cc src\\UserTracks.cc src\\WinMainWrapper.cc
	rc -v -l 0 sautracks.rc
	link -OUT:"bin\\vscmd\\sautracks.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\sautracks.pdb" -DEBUG -RELEASE -SUBSYSTEM:WINDOWS -ENTRY:mainCRTStartup Main.obj UserTracks.obj WinMainWrapper.obj sautracks.res ComCtl32.lib shell32.lib user32.lib comdlg32.lib Gdi32.lib
	rm *.obj sautracks.res