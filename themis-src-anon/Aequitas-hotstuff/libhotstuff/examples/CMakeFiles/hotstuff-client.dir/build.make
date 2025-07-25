# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_SOURCE_DIR = /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff

# Include any dependencies generated for this target.
include examples/CMakeFiles/hotstuff-client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/CMakeFiles/hotstuff-client.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/hotstuff-client.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/hotstuff-client.dir/flags.make

examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o: examples/CMakeFiles/hotstuff-client.dir/flags.make
examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o: examples/hotstuff_client.cpp
examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o: examples/CMakeFiles/hotstuff-client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o"
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o -MF CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o.d -o CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o -c /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples/hotstuff_client.cpp

examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.i"
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples/hotstuff_client.cpp > CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.i

examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.s"
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples/hotstuff_client.cpp -o CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.s

# Object files for target hotstuff-client
hotstuff__client_OBJECTS = \
"CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o"

# External object files for target hotstuff-client
hotstuff__client_EXTERNAL_OBJECTS =

examples/hotstuff-client: examples/CMakeFiles/hotstuff-client.dir/hotstuff_client.cpp.o
examples/hotstuff-client: examples/CMakeFiles/hotstuff-client.dir/build.make
examples/hotstuff-client: libhotstuff.a
examples/hotstuff-client: salticidae/libsalticidae.a
examples/hotstuff-client: secp256k1/.libs/libsecp256k1.a
examples/hotstuff-client: examples/CMakeFiles/hotstuff-client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable hotstuff-client"
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hotstuff-client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/hotstuff-client.dir/build: examples/hotstuff-client
.PHONY : examples/CMakeFiles/hotstuff-client.dir/build

examples/CMakeFiles/hotstuff-client.dir/clean:
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples && $(CMAKE_COMMAND) -P CMakeFiles/hotstuff-client.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/hotstuff-client.dir/clean

examples/CMakeFiles/hotstuff-client.dir/depend:
	cd /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples /home/taeung/themis-src-anon/Aequitas-hotstuff/libhotstuff/examples/CMakeFiles/hotstuff-client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : examples/CMakeFiles/hotstuff-client.dir/depend

