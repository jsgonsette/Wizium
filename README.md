# Wizium

This project is a crosswords generator engine able to generate grids like this from dictonaries:

![Crosswords](./Pictures/Crosswords.png)

The engine comes as a library written in C++. A python wrapper is available right now, other languages may be easily added.


## How to compile the library

If you are on Windows and do not want to compile the library, you can simply use the pre-generated one available [here](./Binaries/Windows/libWizium_x64.dll) and skip this step. Otherwise, any C++ compiler should fit your needs. Currently it has been tested with Visual Studio on Windows and GCC on Linux.

* The [CMakelists.txt](./Sources/CMakeLists.txt) file in the source directory let you use [CMake](https://cmake.org/) for the generation on any platform.
* On Windows, you can find Visual Studio projects in the [Projects](./Projects/) directory.

### Compile on Windows with Visual Studio

Assuming Visual Studio is installed on your machine,

* Launch the "Developer Command Prompt" from the start menu;
* Run the command: ```msbuild ./Projects/VS2017/libWizium.vcxproj -p:Configuration=Release -p:Platform=x64```;
* The DLL should be created in the ```./Binaries/Windows``` directory.

### Compile with CMake on Windows

Assuming CMake and Visual Studio are installed on your machine,

* Step in ```cmake``` directory
* Run the command ```cmake -DCMAKE_GENERATOR_PLATFORM=x64 ..\Sources\```
* Run the command ```cmake --build . --config Release --target install```
* The DLL should be created in the ```./Binaries/Windows``` directory.
  

## Wrappers

### Python

[libWizium.py](./Wrappers/Python/libWizium.py) is a Python wrapper giving full access to the engine capabilities. A Wizium module instance can be created through the *Wizium* class, provided the path to the *libWizium* library file.

This class provides the following API:

* **DIC_Clear:** Flush the dictionary content;