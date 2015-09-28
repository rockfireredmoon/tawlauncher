bin_PROGRAMS=TAWLauncher
TAWLauncher_SOURCES=TAWLauncher.cpp

if MINGW
TAWLauncher_CPPFLAGS = -DWINDOWS_PLATFORM=1
TAWLauncher_LDFLAGS = -mwindows -static-libgcc -static-libstdc++
endif
