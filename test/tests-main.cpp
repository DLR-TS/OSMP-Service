/**
@authors German Aerospace Center: Nils Wendorff, Björn Bahn, Danny Behnecke
*/

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "Utils.h"

TEST_CASE("compare strings") {
	std::string a = "OSMPSensorDATA[2]";
	std::string b = "OSMPSensorDATA[1]";

	bool c = matchingNames(a, b);
	CHECK(!c);
}
