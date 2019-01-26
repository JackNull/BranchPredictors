#include "../sim/predictor_2level.h"

#include "catch.hpp"

TEST_CASE("Brach History Register") {
    uint sz = 16;
    BranchHistoryRegistor bhr(sz);
    bhr_content_t j = 0;

    for(int i = 0; i < sz; i++, j = (j << 1) + 1) {
        REQUIRE(bhr.getIndexOfPatternTable() == j);
        bhr.update(true);
    }
    REQUIRE(bhr.getIndexOfPatternTable() == j);
    for(int i = 0; i < sz; j -= (1 << i), i++) {
        REQUIRE(bhr.getIndexOfPatternTable() == j);
        bhr.update(false);
    }
    REQUIRE(bhr.getIndexOfPatternTable() == j);
}
