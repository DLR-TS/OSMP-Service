# OSTAR OSMP-Service

OSMP-Service is a part of [OSTAR](https://github.com/DLR-TS/OSTAR-Quickstart).
It connects [FMI](https://fmi-standard.org/) models with [OSI](https://www.asam.net/standards/detail/osi/) messages to OSTAR.
It also can replay trajectories to OSI messages and save OSI trace files.

# Installation Guide

## Linux

in root folder:
```sh
 # Preparation
 pip install conan==1.59.0
 # Checkout
 git clone https://github.com/DLR-TS/OSMP-Service.git
 cd OSMP-Service
 # Build
 mkdir build && cd build
 cmake .. -DBUILD_SHARED_LIBS=false -DCMAKE_BUILD_TYPE=Release
 cmake --build . --target OSMPService
 # Run
 cd bin
 ./OSMPService
```

## Docker

```sh
 git clone https://github.com/DLR-TS/OSMP-Service.git
 cd OSMP-Service
 docker build -t ostar:osmpservice .
```

# Windows

Install [conan 1.x](https://conan.io/) \
Add conan.exe to PATH environment variable \
Open the directory in Visual Studio and use the cmake integration.

# Run

| Important runtime parameter | Description |
| ------ | ------ |
| \<port\> | open port |
| \<ip\>:\<port\> | open port and allow only connections from ip |
| -v | verbose prints |

Many configurations are available through CoSiMa Configuration.
The complete list can be found [here](https://github.com/DLR-TS/Carla-OSI-Service/blob/master/Configuration.md).

# Further information

Check out the OSTAR documentation at [OSTAR Quickstart](https://github.com/DLR-TS/OSTAR-Quickstart/tree/main/docu).

# Contacts

bjoern.bahn@dlr.de frank.baumgarten@dlr.de opensource-ts@dlr.de

This software was originally developed as part of [SetLevel](https://setlevel.de/).
