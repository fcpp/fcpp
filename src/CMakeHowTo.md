# FCPP - CMake build guide

## WINDOWS (MinGW-w64)

To build FCPP, you'll need:

- MinGW-w64 8.1.0 (posix threads)
- CMake 3.9 (or higher)

After setting the respective environmental variables, go into the ./src directory through the console.

If you'd like to build FCPP with the OpenGL features, type:
```
cmake -S ./ -B ./bin -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DFCPP_BUILD_GL=ON -Wno-dev
```
Otherwise:
```
cmake -S ./ -B ./bin -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -Wno-dev
```

Type the following command in order to build the library.
```
cmake --build ./bin/
```

Wait for the library to be built on your Windows system.

## LINUX

To build FCPP, you'll need:

- xorg-dev package (X11)
- CMake 3.9 (or higher)

To install the X11 package, type the following command:
```
sudo apt-get  install xorg-dev
```

After installing X11, go into the ./src directory through the console.

If you'd like to build FCPP with the OpenGL features, type:
```
cmake -S ./ -B ./bin -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DFCPP_BUILD_GL=ON -Wno-dev
```
Otherwise:
```
cmake -S ./ -B ./bin -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -Wno-dev
```

Type the following command in order to build the library.
```
cmake --build ./bin/
```

Wait for the library to be built on your Linux system.
