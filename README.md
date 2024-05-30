# OSMP Service

The OSMP Service can open a [OSMP](https://github.com/OpenSimulationInterface/osi-sensor-model-packaging)-FMU. The OSMP Service is addressed by the [CoSiMa](https://github.com/DLR-TS/CoSiMa).
A CoSiMa instance together with multiple OSMP Service instances creates a simulation.

The OSMP Service act as a server. The address can be given as [::]:\<port\> via command line parameter.
If no parameter is given default is: 0.0.0.0:51425

Pass -v as a parameter to enable verbose output.

# Installation Guide

## Docker build

```sh
 docker build -t ostar:osmpservice .
```

## Manual build

in root folder:
```sh
 mkdir build && cd build
 cmake .. -DBUILD_SHARED_LIBS=false -DCMAKE_BUILD_TYPE=Release
 cmake --build . --target OSMPService
```

## Windows with MSVC 2017
Open the folder in Visual Studio and use the cmake integration.

# Contacts

bjoern.bahn@dlr.de frank.baumgarten@dlr.de

This software was originally developed as part of [SetLevel](https://setlevel.de/).
