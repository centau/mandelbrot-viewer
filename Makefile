all: build_release

build_run_release: build_release run_release 
build_run_debug: build_debug run_debug

build_release: compile link_release clean
build_debug: compile link_debug clean

compile:
	g++ -I src/include -c src/Main.cpp 

link_release:
	g++ Main.o -o bin/Release/Main.exe -L src/lib/Release -l sfml-graphics -l sfml-window -l sfml-system -mwindows
link_debug:
	g++ Main.o -o bin/Debug/Main.exe -L src/lib/Debug -l sfml-graphics-d -l sfml-window-d -l sfml-system-d

run_release:
	bin/Release/Main.exe
run_debug:
	bin/Debug/Main.exe

clean:
	del *.o