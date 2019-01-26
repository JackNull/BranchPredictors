#include "../sim/predictor_2level.h"

#include "catch.hpp"

TEST_CASE("BhrTableHash") {
    BhrTableHash tb;

    vector<UINT64> PCs = {0, 4, 8, 12, 16, 20};

    SECTION("access") {
        for(auto pc: PCs) {
            REQUIRE(tb.contains(pc) == false);
            tb.update(pc, true);
            REQUIRE(tb.contains(pc) == true);
        }
    }
}
