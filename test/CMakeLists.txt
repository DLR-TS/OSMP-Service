set(TEST_RESOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/resources)

configure_file(${CMAKE_CURRENT_LIST_DIR}/TestResourceDirectory.h.in ${CMAKE_CURRENT_BINARY_DIR}/../include/TestResourceDirectory.h)

set (TESTFILES 
${CMAKE_CURRENT_LIST_DIR}/tests-main.cpp
${CMAKE_CURRENT_LIST_DIR}/tests-OSMPInterface.cpp
PARENT_SCOPE)
