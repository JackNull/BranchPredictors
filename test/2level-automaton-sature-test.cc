#include "../sim/predictor_2level.h"

#include "catch.hpp"

TEST_CASE("AutomatonSature next state") {
    AutomatonSature automaton(2);

    REQUIRE(automaton.nextState(0, true) == 1);
    REQUIRE(automaton.nextState(1, true) == 2);
    REQUIRE(automaton.nextState(2, true) == 3);
    REQUIRE(automaton.nextState(3, true) == 3);
    REQUIRE(automaton.nextState(0, false) == 0);
    REQUIRE(automaton.nextState(1, false) == 0);
    REQUIRE(automaton.nextState(2, false) == 1);
    REQUIRE(automaton.nextState(3, false) == 2);
}

TEST_CASE("AutomatonSature predict") {
    AutomatonSature automaton(2);

    REQUIRE(automaton.predict(0) == false);
    REQUIRE(automaton.predict(1) == false);
    REQUIRE(automaton.predict(2) == true);
    REQUIRE(automaton.predict(3) == true);
}
