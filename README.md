# HAIRCUT
![build badge](https://github.com/buster-jangle/PROGENY/actions/workflows/cmake_build_ubuntu-latest.yml/badge.svg?branch=master)
![build badge](https://github.com/buster-jangle/PROGENY/actions/workflows/cmake_tests_ubuntu-latest.yml/badge.svg?branch=master)
![build badge](https://github.com/buster-jangle/PROGENY/actions/workflows/doxygen-gh-pages.yml/badge.svg?branch=master)
  
PROGENY is a template for C++ projects, plus a script to automatically generate new projects with a given name and set up a GitHub repo. PROGENY is intended to demonstrate & encourage modern C++ development practices using CMake, Git, GitHub, and Doxygen. PROGENY is also a fully functional C++ project on its own (Primary executables just return "Hello world", test executables always pass, and the template library has one class with example functions to demonstrate Doxygen usage.)

Template includes the following:
* Default library
* Default executable
* Default test
* Default .gitignore tailored for C++ development in CLion
* Default git submodules (PLOG & CLI11) to promote standardized logging and CLI interface
* Default install locations defined in top-level CMakeLists.txt
* Documentation generation via doxygen
* CMake script automatically pulls git submodules, and when compiled as release, builds Doxygen documentation.


## Required packages

[PLOG](https://github.com/SergiusTheBest/plog) and [CLI11](https://github.com/CLIUtils/CLI11) are required, but will be cloned automatically as git submodules when project is built. [Doxygen](https://github.com/doxygen/doxygen) is recommended, but not required.


## Using PROGENY

* Click "Use this template" to create your new repo, and give it a name. 
* Clone the repo to a local directory
* Run `progeny.sh` to initialize the new project. This will automatically perform the following:
  * Replace all occurences of "HAIRCUT" with your new project name.
  * Add a creation date to template files with the current date
  * Reset the git repo
  * delete `progeny.sh`
* Your project your now be able tobuild and run tests.
  
## Building PROGENY

Building PROGENY also builds documentation with Doxygen (if it is installed). You can run `compile.sh` to automatically build the project, or manually build with the following commands:
```
mkdir build
cd build
cmake ..
make -j24
make test #Optional run unit tests defined in CMake
```

## Install

By default, the PROGENY templat installs primary executables and libraries in /usr/local/(lib/include). Install destinations are defined in a variable in the top level CMakeLists.txt so it can be easily changed.


## Maintenance

Maintainers: Sean O'Callaghan

Tickets: Can be opened in Github Issues.

Also heres how to put a GIF in a README.md, in case you wanted to know ;)
![Alt Text](https://upload.wikimedia.org/wikipedia/commons/7/79/Perfect-loop-cube.gif)
