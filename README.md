# OSMP Client

Client for OSMP-FMUs to communicate via gRPC with CoSiMa.

Server Address can be given as [::]:<port> via command line parameter.
If no parameter is given default is: 0.0.0.0:51425

For test run copy the FMU from test to your build directory and start the OSMP-Client.
In CoSiMa is a comment out test in test-OSMPInterface which uses the OSMP-Client.