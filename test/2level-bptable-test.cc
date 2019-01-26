#include "../sim/predictor_2level.h"

#include "catch.hpp"

TEST_CASE("BpTable") {
    Automaton *automaton = new AutomatonSature(2);
    BranchPatternTable tb(automaton);

    SECTION("update/predict should match Automaton") {
        bhr_content_t index = 0xff;
        tb.update(index, false);
        REQUIRE(tb.predict(index) == false);
        tb.update(index, true);
        REQUIRE(tb.predict(index) == false);
        tb.update(index, true);
        REQUIRE(tb.predict(index) == true);
        tb.update(index, true);
        REQUIRE(tb.predict(index) == true);
        tb.update(index, true);
        tb.update(index, false);
        tb.update(index, false);
        REQUIRE(tb.predict(index) == false);
    }
}
