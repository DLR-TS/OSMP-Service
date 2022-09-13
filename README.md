# OSMP Service

The OSMP Service can open a OSMP-FMU (https://github.com/OpenSimulationInterface/osi-sensor-model-packaging). The OSMP Service is addressed by the CoSiMa.
A CoSiMa instance together with multiple OSMP Service instances creates a simulation.

The OSMP Service act as a server. The address can be given as [::]:\<port\> via command line parameter.
If no parameter is given default is: 0.0.0.0:51425

Pass -v as a parameter to enable verbose output.

# Installation Guide

## Docker build
Create personal access token (PAT) for gitlab.setlevel.de.
Create .TOKEN file in project root.

Paste PAT in file: \<username\>:\<accesstoken\>
```sh
 docker build -t setlevel:osmpservice .
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
