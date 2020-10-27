#version
cmake_minimum_required (VERSION 3.8)

#source
set(COSIMA_SOURCE ${COSIMA_SOURCE}
${CMAKE_CURRENT_LIST_DIR}/OSMPInterface.cpp
${CMAKE_CURRENT_LIST_DIR}/GRPCInterface.cpp
PARENT_SCOPE
)

