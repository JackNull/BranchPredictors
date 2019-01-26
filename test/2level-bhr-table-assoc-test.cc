#include "../sim/predictor_2level.h"

#include "catch.hpp"

TEST_CASE("BhrTableAssoc") {

    SECTION("LRU") {
        BhrTableAssoc tb;

        vector<UINT64> PCs = {0};
        for(int i = 1; i <= AHRT_ASSOC + 1; i++) {
            PCs.push_back(PCs[i - 1] + AHRT_SET_NUM);
        }

        REQUIRE(tb.contains(PCs[0]) == false);
        tb.update(PCs[0], true);
        REQUIRE(tb.contains(PCs[0]) == true);
        for(int i = 1; i <= AHRT_ASSOC; i++) {
            tb.update(PCs[i], true);
        }
        REQUIRE(tb.contains(PCs[0]) == false);
        REQUIRE(tb.contains(PCs[1]) == true);
        tb.get(PCs[1]);
        tb.update(PCs.back(), true);
        REQUIRE(tb.contains(PCs[1]) == true);
        REQUIRE(tb.contains(PCs[2]) == false);
    }
}
