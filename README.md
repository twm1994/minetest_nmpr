# Minetest NMPR

This is an updated version of the [original repository](https://github.com/celeron55/minetest_nmpr) that works on Visual Studio 2017 Community version.

## Dependency

 - [JThread 1.3.3](http://research.edm.uhasselt.be/jori/page/CS/Jthread.html)
 - [Irrlicht-1.8.4](http://irrlicht.sourceforge.net/)
 - [CMake](https://cmake.org/download/)

## Modification on original repository

 - Update JThread and Irrlicht versions
 - Rebuild .lib and .h files in JThread
 - Update **Additional Include Directories** and **Additional Library Directories** in Visual Studio project Properties, and corresponding `include` statements in source code
 - Change line `#pragma comment(lib, "WSock32.Lib")` to `#pragma comment(lib, "Ws2_32.Lib")` in **socket.h**
 - Copy **Irrlicht.dll** from **\bin\Win32-VisualStudio** in **Irrlicht folder** to the output folder that contains **minetest.exe**

## License

### To be updated
