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
include CMakeFiles/database.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/database.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/database.dir/flags.make

CMakeFiles/database.dir/Relation.cpp.o: CMakeFiles/database.dir/flags.make
CMakeFiles/database.dir/Relation.cpp.o: ../../Relation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/database.dir/Relation.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/database.dir/Relation.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Relation.cpp

CMakeFiles/database.dir/Relation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/database.dir/Relation.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Relation.cpp > CMakeFiles/database.dir/Relation.cpp.i

CMakeFiles/database.dir/Relation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/database.dir/Relation.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Relation.cpp -o CMakeFiles/database.dir/Relation.cpp.s

CMakeFiles/database.dir/Operators.cpp.o: CMakeFiles/database.dir/flags.make
CMakeFiles/database.dir/Operators.cpp.o: ../../Operators.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/database.dir/Operators.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/database.dir/Operators.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Operators.cpp

CMakeFiles/database.dir/Operators.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/database.dir/Operators.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Operators.cpp > CMakeFiles/database.dir/Operators.cpp.i

CMakeFiles/database.dir/Operators.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/database.dir/Operators.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Operators.cpp -o CMakeFiles/database.dir/Operators.cpp.s

CMakeFiles/database.dir/Parser.cpp.o: CMakeFiles/database.dir/flags.make
CMakeFiles/database.dir/Parser.cpp.o: ../../Parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/database.dir/Parser.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/database.dir/Parser.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Parser.cpp

CMakeFiles/database.dir/Parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/database.dir/Parser.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Parser.cpp > CMakeFiles/database.dir/Parser.cpp.i

CMakeFiles/database.dir/Parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/database.dir/Parser.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Parser.cpp -o CMakeFiles/database.dir/Parser.cpp.s

CMakeFiles/database.dir/Utils.cpp.o: CMakeFiles/database.dir/flags.make
CMakeFiles/database.dir/Utils.cpp.o: ../../Utils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/database.dir/Utils.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/database.dir/Utils.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Utils.cpp

CMakeFiles/database.dir/Utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/database.dir/Utils.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Utils.cpp > CMakeFiles/database.dir/Utils.cpp.i

CMakeFiles/database.dir/Utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/database.dir/Utils.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Utils.cpp -o CMakeFiles/database.dir/Utils.cpp.s

CMakeFiles/database.dir/Joiner.cpp.o: CMakeFiles/database.dir/flags.make
CMakeFiles/database.dir/Joiner.cpp.o: ../../Joiner.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/database.dir/Joiner.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/database.dir/Joiner.cpp.o -c /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Joiner.cpp

CMakeFiles/database.dir/Joiner.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/database.dir/Joiner.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Joiner.cpp > CMakeFiles/database.dir/Joiner.cpp.i

CMakeFiles/database.dir/Joiner.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/database.dir/Joiner.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/cp_project1/student08395/2021_ite4065_2018008395/project1/Joiner.cpp -o CMakeFiles/database.dir/Joiner.cpp.s

# Object files for target database
database_OBJECTS = \
"CMakeFiles/database.dir/Relation.cpp.o" \
"CMakeFiles/database.dir/Operators.cpp.o" \
"CMakeFiles/database.dir/Parser.cpp.o" \
"CMakeFiles/database.dir/Utils.cpp.o" \
"CMakeFiles/database.dir/Joiner.cpp.o"

# External object files for target database
database_EXTERNAL_OBJECTS =

libdatabase.a: CMakeFiles/database.dir/Relation.cpp.o
libdatabase.a: CMakeFiles/database.dir/Operators.cpp.o
libdatabase.a: CMakeFiles/database.dir/Parser.cpp.o
libdatabase.a: CMakeFiles/database.dir/Utils.cpp.o
libdatabase.a: CMakeFiles/database.dir/Joiner.cpp.o
libdatabase.a: CMakeFiles/database.dir/build.make
libdatabase.a: CMakeFiles/database.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX static library libdatabase.a"
	$(CMAKE_COMMAND) -P CMakeFiles/database.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/database.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/database.dir/build: libdatabase.a

.PHONY : CMakeFiles/database.dir/build

CMakeFiles/database.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/database.dir/cmake_clean.cmake
.PHONY : CMakeFiles/database.dir/clean

CMakeFiles/database.dir/depend:
	cd /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/cp_project1/student08395/2021_ite4065_2018008395/project1 /home/cp_project1/student08395/2021_ite4065_2018008395/project1 /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release /home/cp_project1/student08395/2021_ite4065_2018008395/project1/build/release/CMakeFiles/database.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/database.dir/depend

