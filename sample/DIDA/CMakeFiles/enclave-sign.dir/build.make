# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/dajiejie/dida-sgx-sealing

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dajiejie/dida-sgx-sealing

# Utility rule file for enclave-sign.

# Include the progress variables for this target.
include sample/DIDA/CMakeFiles/enclave-sign.dir/progress.make

sample/DIDA/CMakeFiles/enclave-sign:
	cd /home/dajiejie/dida-sgx-sealing/sample/DIDA && /opt/intel/sgxsdk/bin/x64/sgx_sign sign -key /home/dajiejie/dida-sgx-sealing/sample/DIDA/Enclave/Enclave_private.pem -config /home/dajiejie/dida-sgx-sealing/sample/DIDA/Enclave/Enclave.config.xml -enclave /home/dajiejie/dida-sgx-sealing/sample/DIDA/libenclave.so -out /home/dajiejie/dida-sgx-sealing/sample/DIDA/enclave.signed.so

enclave-sign: sample/DIDA/CMakeFiles/enclave-sign
enclave-sign: sample/DIDA/CMakeFiles/enclave-sign.dir/build.make

.PHONY : enclave-sign

# Rule to build all files generated by this target.
sample/DIDA/CMakeFiles/enclave-sign.dir/build: enclave-sign

.PHONY : sample/DIDA/CMakeFiles/enclave-sign.dir/build

sample/DIDA/CMakeFiles/enclave-sign.dir/clean:
	cd /home/dajiejie/dida-sgx-sealing/sample/DIDA && $(CMAKE_COMMAND) -P CMakeFiles/enclave-sign.dir/cmake_clean.cmake
.PHONY : sample/DIDA/CMakeFiles/enclave-sign.dir/clean

sample/DIDA/CMakeFiles/enclave-sign.dir/depend:
	cd /home/dajiejie/dida-sgx-sealing && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dajiejie/dida-sgx-sealing /home/dajiejie/dida-sgx-sealing/sample/DIDA /home/dajiejie/dida-sgx-sealing /home/dajiejie/dida-sgx-sealing/sample/DIDA /home/dajiejie/dida-sgx-sealing/sample/DIDA/CMakeFiles/enclave-sign.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : sample/DIDA/CMakeFiles/enclave-sign.dir/depend

