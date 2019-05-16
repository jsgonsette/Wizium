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
  
### Compile with CMake on Linux

Assuming CMake in installed on your machine,

* Step in ```cmake``` directory
* Run the command ```cmake ..\Sources\```
* Run the command ```cmake --build .```


## Wrappers

### Python

[libWizium.py](./Wrappers/Python/libWizium.py) is a Python wrapper giving full access to the engine capabilities. A Wizium module instance can be created through the *Wizium* class, provided the path to the *libWizium* library file.

This class provides the following API:

* **dic_clear:** Flush the dictionary content;
* **dic_add_entries:** Add entries to the dictionary;
* **dic_find_random_entry:** Find a random entry in the dictionary, matching a mask;
* **dic_find_entry:** Find a random entry in the dictionary, matching a mask;
* **dic_gen_num_words:** Return the number of words in the dictionary;
* **grid_erase:** Erase the grid content;
* **grid_set_size:** Set the grid size. Content can be lost when shrinking;
* **grid_set_box:** Set the type of box at a given grid coordinate;
* **grid_write:** Write a word on the grid;
* **grid_read:** Read the whole content of the grid;
* **solver_start:** Start the grid generation process;
* **solver_step:** Move a few steps in the grid generation process;
* **solver_stop:** Stop the grid generation process;

