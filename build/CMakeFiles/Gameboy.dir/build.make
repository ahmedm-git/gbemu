# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ahmed/repos/gbemu

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ahmed/repos/gbemu/build

# Include any dependencies generated for this target.
include CMakeFiles/Gameboy.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/Gameboy.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/Gameboy.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Gameboy.dir/flags.make

CMakeFiles/Gameboy.dir/codegen:
.PHONY : CMakeFiles/Gameboy.dir/codegen

CMakeFiles/Gameboy.dir/src/main.cpp.o: CMakeFiles/Gameboy.dir/flags.make
CMakeFiles/Gameboy.dir/src/main.cpp.o: /home/ahmed/repos/gbemu/src/main.cpp
CMakeFiles/Gameboy.dir/src/main.cpp.o: CMakeFiles/Gameboy.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ahmed/repos/gbemu/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Gameboy.dir/src/main.cpp.o"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Gameboy.dir/src/main.cpp.o -MF CMakeFiles/Gameboy.dir/src/main.cpp.o.d -o CMakeFiles/Gameboy.dir/src/main.cpp.o -c /home/ahmed/repos/gbemu/src/main.cpp

CMakeFiles/Gameboy.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Gameboy.dir/src/main.cpp.i"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ahmed/repos/gbemu/src/main.cpp > CMakeFiles/Gameboy.dir/src/main.cpp.i

CMakeFiles/Gameboy.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Gameboy.dir/src/main.cpp.s"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ahmed/repos/gbemu/src/main.cpp -o CMakeFiles/Gameboy.dir/src/main.cpp.s

CMakeFiles/Gameboy.dir/src/bus.cpp.o: CMakeFiles/Gameboy.dir/flags.make
CMakeFiles/Gameboy.dir/src/bus.cpp.o: /home/ahmed/repos/gbemu/src/bus.cpp
CMakeFiles/Gameboy.dir/src/bus.cpp.o: CMakeFiles/Gameboy.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ahmed/repos/gbemu/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Gameboy.dir/src/bus.cpp.o"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Gameboy.dir/src/bus.cpp.o -MF CMakeFiles/Gameboy.dir/src/bus.cpp.o.d -o CMakeFiles/Gameboy.dir/src/bus.cpp.o -c /home/ahmed/repos/gbemu/src/bus.cpp

CMakeFiles/Gameboy.dir/src/bus.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Gameboy.dir/src/bus.cpp.i"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ahmed/repos/gbemu/src/bus.cpp > CMakeFiles/Gameboy.dir/src/bus.cpp.i

CMakeFiles/Gameboy.dir/src/bus.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Gameboy.dir/src/bus.cpp.s"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ahmed/repos/gbemu/src/bus.cpp -o CMakeFiles/Gameboy.dir/src/bus.cpp.s

CMakeFiles/Gameboy.dir/src/cpu.cpp.o: CMakeFiles/Gameboy.dir/flags.make
CMakeFiles/Gameboy.dir/src/cpu.cpp.o: /home/ahmed/repos/gbemu/src/cpu.cpp
CMakeFiles/Gameboy.dir/src/cpu.cpp.o: CMakeFiles/Gameboy.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/ahmed/repos/gbemu/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Gameboy.dir/src/cpu.cpp.o"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/Gameboy.dir/src/cpu.cpp.o -MF CMakeFiles/Gameboy.dir/src/cpu.cpp.o.d -o CMakeFiles/Gameboy.dir/src/cpu.cpp.o -c /home/ahmed/repos/gbemu/src/cpu.cpp

CMakeFiles/Gameboy.dir/src/cpu.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Gameboy.dir/src/cpu.cpp.i"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ahmed/repos/gbemu/src/cpu.cpp > CMakeFiles/Gameboy.dir/src/cpu.cpp.i

CMakeFiles/Gameboy.dir/src/cpu.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Gameboy.dir/src/cpu.cpp.s"
	/usr/sbin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ahmed/repos/gbemu/src/cpu.cpp -o CMakeFiles/Gameboy.dir/src/cpu.cpp.s

# Object files for target Gameboy
Gameboy_OBJECTS = \
"CMakeFiles/Gameboy.dir/src/main.cpp.o" \
"CMakeFiles/Gameboy.dir/src/bus.cpp.o" \
"CMakeFiles/Gameboy.dir/src/cpu.cpp.o"

# External object files for target Gameboy
Gameboy_EXTERNAL_OBJECTS =

bin/Gameboy: CMakeFiles/Gameboy.dir/src/main.cpp.o
bin/Gameboy: CMakeFiles/Gameboy.dir/src/bus.cpp.o
bin/Gameboy: CMakeFiles/Gameboy.dir/src/cpu.cpp.o
bin/Gameboy: CMakeFiles/Gameboy.dir/build.make
bin/Gameboy: CMakeFiles/Gameboy.dir/compiler_depend.ts
bin/Gameboy: CMakeFiles/Gameboy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/ahmed/repos/gbemu/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable bin/Gameboy"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Gameboy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Gameboy.dir/build: bin/Gameboy
.PHONY : CMakeFiles/Gameboy.dir/build

CMakeFiles/Gameboy.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Gameboy.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Gameboy.dir/clean

CMakeFiles/Gameboy.dir/depend:
	cd /home/ahmed/repos/gbemu/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ahmed/repos/gbemu /home/ahmed/repos/gbemu /home/ahmed/repos/gbemu/build /home/ahmed/repos/gbemu/build /home/ahmed/repos/gbemu/build/CMakeFiles/Gameboy.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/Gameboy.dir/depend

