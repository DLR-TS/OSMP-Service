# OSMP Service

Service for OSMP-FMUs to communicate via gRPC with CoSiMa.

Server Address can be given as [::]:<port> via command line parameter.
If no parameter is given default is: 0.0.0.0:51425

Pass a "-d" as a parameter to enable debug prints.

For test run copy the FMU from test to your build directory and start the OSMP-Service.
In CoSiMa is a comment out test in test-OSMPInterface which uses the OSMP-Service.

# docker build
Create personal access token (PAT) in GitLab.
Create .TOKEN file in project root.
Paste PAT in file: <username>:<accesstoken>
```sh
 docker build -t setlevel:osmpservice .
```
# manual build
in root folder:
```sh
 mkdir build && cd build
 cmake .. -DBUILD_SHARED_LIBS=false -DCMAKE_BUILD_TYPE=Release
 cmake --build . --target OSMPService
```
