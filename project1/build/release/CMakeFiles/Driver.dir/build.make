# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cp_project1/student08395/2021_ite4065_2018008395/project1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release

# Include any dependencies generated for this target.
include CMakeFiles/Driver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Driver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Driver.dir/flags.make

CMakeFiles/Driver.dir/main.cpp.o: CMakeFiles/Driver.dir/flags.make
CMakeFiles/Driver.dir/main.cpp.o: ../../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Driver.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Driver.dir/main.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/main.cpp

CMakeFiles/Driver.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Driver.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/main.cpp > CMakeFiles/Driver.dir/main.cpp.i

CMakeFiles/Driver.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Driver.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/main.cpp -o CMakeFiles/Driver.dir/main.cpp.s

# Object files for target Driver
Driver_OBJECTS = \
"CMakeFiles/Driver.dir/main.cpp.o"

# External object files for target Driver
Driver_EXTERNAL_OBJECTS =

Driver: CMakeFiles/Driver.dir/main.cpp.o
Driver: CMakeFiles/Driver.dir/build.make
Driver: libdatabase.a
Driver: CMakeFiles/Driver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Driver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Driver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Driver.dir/build: Driver

.PHONY : CMakeFiles/Driver.dir/build

CMakeFiles/Driver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Driver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Driver.dir/clean

CMakeFiles/Driver.dir/depend:
	cd /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cp_project1/student08395/2021_ite4065_2018008395/project1 /home/cp_project1/student08395/2021_ite4065_2018008395/project1 /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles/Driver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Driver.dir/depend

