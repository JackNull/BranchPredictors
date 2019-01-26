#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include "utils.h"
// #include "tracer.h"
#include <bitset>

#define NUMCOMP 7
#define MAXHIST 131
#define MINHIST 5

// define global history CSR type
typedef bitset<MAXHIST> hist_t;

// Each entry in the tag Pred Table
struct TagEntry
{
    INT32 ctr;
    UINT64 tag;
    INT32 u;
};

// Folded History implementation ... from ghr (geometric length) -> compressed(target)
struct CompressedHist
{
    // Objective is to convert geomLength of GHR into tagPredLog which is of length equal to the index of the corresponding bank
    // It can be also used for tag when the final length would be the Tag
    UINT64 geomLength;
    UINT64 targetLength;
    UINT64 compHist;
      
    void updateCompHist(hist_t ghr)
    {
        // creating important masks
        int mask = (1 << targetLength) - 1;
        int mask1 = ghr[geomLength] << (geomLength % targetLength);
        int mask2 = (1 << targetLength);
        compHist  = (compHist << 1) + ghr[0];
        compHist ^= ((compHist & mask2) >> targetLength);
        compHist ^= mask1;
        compHist &= mask;
    }    
};

class PREDICTOR {
private:
    // global history register
    hist_t GHR;
    // 16 bit path history
    int PHR;
    // Bimodal
    UINT64  *bimodal;          // pattern history table
    UINT64  historyLength;     // history length
    UINT64  bimodalsize; // entries in pht 
    UINT64  bimodalLog;
    // Tagged Predictors
    TagEntry *tagPred[NUMCOMP];
    UINT64 compsize;
    UINT64 tagPredLog;
    int geometric[NUMCOMP];
    // Compressed Buffers
    CompressedHist indexComp[NUMCOMP];
    CompressedHist tagComp[2][NUMCOMP]; 
 
    // Predictions
    bool primePred;
    bool altPred;
    int T_i;
    int T_a;
    // Global Index compated only once
    UINT64 GI[NUMCOMP];
    UINT64 tag[NUMCOMP];
    UINT64 clock;
    int clock_flip;
    INT32 altBetterCount;
    long STORAGESIZE;
public:

    PREDICTOR(void);

    // The interface to the functions below CAN NOT be changed
    bool GetPrediction(UINT64 PC);  
    void UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
    void TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);
  
    // Contestants can define their own functions below

};


  

/***********************************************************/
#endif

