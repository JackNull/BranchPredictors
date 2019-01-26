#include "../sim/predictor_2level.h"

#include "catch.hpp"

#include <numeric>

TEST_CASE("Simple cases", "[predictor]") {
    PREDICTOR p;

    vector<UINT64> pcs((1 << AUTOMATON_STATE_SIZE) + 1);
    iota(pcs.begin(), pcs.end(), 0);

    SECTION("pattern b00 - b11") {
        for(int i = 0; i < 3; i++) {
            for(int j = 1; j < (1 << AUTOMATON_STATE_SIZE); j++) {
                p.UpdatePredictor(pcs[j], OPTYPE_JMP_DIRECT_COND, true, false, 0);
                if(j & (1 << (AUTOMATON_STATE_SIZE - 1))) {
                    REQUIRE(p.GetPrediction(pcs[0]) == true);
                } else {
                    REQUIRE(p.GetPrediction(pcs[0]) == false);
                }
            }
            p.UpdatePredictor(pcs[0], OPTYPE_JMP_DIRECT_COND, true, false, 0);
        }
    }
}
