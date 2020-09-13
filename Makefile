# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/cltx/projects/cpp/recognice

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/cltx/projects/cpp/recognice

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cltx/projects/cpp/recognice/CMakeFiles /home/cltx/projects/cpp/recognice/CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/cltx/projects/cpp/recognice/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named recognice

# Build rule for target.
recognice: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 recognice
.PHONY : recognice

# fast build rule for target.
recognice/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/build
.PHONY : recognice/fast

src/common/Camera.o: src/common/Camera.cpp.o

.PHONY : src/common/Camera.o

# target to build an object file
src/common/Camera.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/Camera.cpp.o
.PHONY : src/common/Camera.cpp.o

src/common/Camera.i: src/common/Camera.cpp.i

.PHONY : src/common/Camera.i

# target to preprocess a source file
src/common/Camera.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/Camera.cpp.i
.PHONY : src/common/Camera.cpp.i

src/common/Camera.s: src/common/Camera.cpp.s

.PHONY : src/common/Camera.s

# target to generate assembly for a file
src/common/Camera.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/Camera.cpp.s
.PHONY : src/common/Camera.cpp.s

src/common/ConfigurationFile.o: src/common/ConfigurationFile.cpp.o

.PHONY : src/common/ConfigurationFile.o

# target to build an object file
src/common/ConfigurationFile.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ConfigurationFile.cpp.o
.PHONY : src/common/ConfigurationFile.cpp.o

src/common/ConfigurationFile.i: src/common/ConfigurationFile.cpp.i

.PHONY : src/common/ConfigurationFile.i

# target to preprocess a source file
src/common/ConfigurationFile.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ConfigurationFile.cpp.i
.PHONY : src/common/ConfigurationFile.cpp.i

src/common/ConfigurationFile.s: src/common/ConfigurationFile.cpp.s

.PHONY : src/common/ConfigurationFile.s

# target to generate assembly for a file
src/common/ConfigurationFile.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ConfigurationFile.cpp.s
.PHONY : src/common/ConfigurationFile.cpp.s

src/common/ImageManipulation.o: src/common/ImageManipulation.cpp.o

.PHONY : src/common/ImageManipulation.o

# target to build an object file
src/common/ImageManipulation.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ImageManipulation.cpp.o
.PHONY : src/common/ImageManipulation.cpp.o

src/common/ImageManipulation.i: src/common/ImageManipulation.cpp.i

.PHONY : src/common/ImageManipulation.i

# target to preprocess a source file
src/common/ImageManipulation.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ImageManipulation.cpp.i
.PHONY : src/common/ImageManipulation.cpp.i

src/common/ImageManipulation.s: src/common/ImageManipulation.cpp.s

.PHONY : src/common/ImageManipulation.s

# target to generate assembly for a file
src/common/ImageManipulation.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/ImageManipulation.cpp.s
.PHONY : src/common/ImageManipulation.cpp.s

src/common/TelegramBot.o: src/common/TelegramBot.cpp.o

.PHONY : src/common/TelegramBot.o

# target to build an object file
src/common/TelegramBot.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/TelegramBot.cpp.o
.PHONY : src/common/TelegramBot.cpp.o

src/common/TelegramBot.i: src/common/TelegramBot.cpp.i

.PHONY : src/common/TelegramBot.i

# target to preprocess a source file
src/common/TelegramBot.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/TelegramBot.cpp.i
.PHONY : src/common/TelegramBot.cpp.i

src/common/TelegramBot.s: src/common/TelegramBot.cpp.s

.PHONY : src/common/TelegramBot.s

# target to generate assembly for a file
src/common/TelegramBot.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/TelegramBot.cpp.s
.PHONY : src/common/TelegramBot.cpp.s

src/common/main.o: src/common/main.cpp.o

.PHONY : src/common/main.o

# target to build an object file
src/common/main.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/main.cpp.o
.PHONY : src/common/main.cpp.o

src/common/main.i: src/common/main.cpp.i

.PHONY : src/common/main.i

# target to preprocess a source file
src/common/main.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/main.cpp.i
.PHONY : src/common/main.cpp.i

src/common/main.s: src/common/main.cpp.s

.PHONY : src/common/main.s

# target to generate assembly for a file
src/common/main.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/common/main.cpp.s
.PHONY : src/common/main.cpp.s

src/unix/NotificationIconUnix.o: src/unix/NotificationIconUnix.cpp.o

.PHONY : src/unix/NotificationIconUnix.o

# target to build an object file
src/unix/NotificationIconUnix.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/unix/NotificationIconUnix.cpp.o
.PHONY : src/unix/NotificationIconUnix.cpp.o

src/unix/NotificationIconUnix.i: src/unix/NotificationIconUnix.cpp.i

.PHONY : src/unix/NotificationIconUnix.i

# target to preprocess a source file
src/unix/NotificationIconUnix.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/unix/NotificationIconUnix.cpp.i
.PHONY : src/unix/NotificationIconUnix.cpp.i

src/unix/NotificationIconUnix.s: src/unix/NotificationIconUnix.cpp.s

.PHONY : src/unix/NotificationIconUnix.s

# target to generate assembly for a file
src/unix/NotificationIconUnix.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/recognice.dir/build.make CMakeFiles/recognice.dir/src/unix/NotificationIconUnix.cpp.s
.PHONY : src/unix/NotificationIconUnix.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... recognice"
	@echo "... src/common/Camera.o"
	@echo "... src/common/Camera.i"
	@echo "... src/common/Camera.s"
	@echo "... src/common/ConfigurationFile.o"
	@echo "... src/common/ConfigurationFile.i"
	@echo "... src/common/ConfigurationFile.s"
	@echo "... src/common/ImageManipulation.o"
	@echo "... src/common/ImageManipulation.i"
	@echo "... src/common/ImageManipulation.s"
	@echo "... src/common/TelegramBot.o"
	@echo "... src/common/TelegramBot.i"
	@echo "... src/common/TelegramBot.s"
	@echo "... src/common/main.o"
	@echo "... src/common/main.i"
	@echo "... src/common/main.s"
	@echo "... src/unix/NotificationIconUnix.o"
	@echo "... src/unix/NotificationIconUnix.i"
	@echo "... src/unix/NotificationIconUnix.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system
