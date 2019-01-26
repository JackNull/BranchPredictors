#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>
// #include <inttypes.h>
// #include <math.h>

#include "utils.h"
#include <vector>
#include <list>
#include <unordered_map>
#include <cmath>

using namespace std;

typedef UINT32 uint;
typedef UINT32 bhr_content_t;

#define BHR_CONTENT_SIZE 12
#define AUTOMATON_STATE_SIZE 2

#define AHRT_SIZE 512
#define AHRT_ASSOC 4
#define AHRT_SET_NUM (AHRT_SIZE / AHRT_ASSOC)
#define AHRT_INDEX_OFFSET ((UINT64)log2(AHRT_SET_NUM))
#define AHRT_ENTRY_INDEX(x) ((x) & (AHRT_SET_NUM - 1))
#define AHRT_ENTRY_TAG(x) ((x) >> AHRT_INDEX_OFFSET)

class Automaton {
protected:
    uint size;
public:
    Automaton(uint size);
    virtual uint nextState(uint state, bool dir) = 0;
    virtual bool predict(uint state) = 0;
    virtual ~Automaton();
};

class AutomatonSature: public Automaton {
public:
    AutomatonSature(uint size);
    uint nextState(uint state, bool dir);
    bool predict(uint state);
};

typedef AutomatonSature Automaton_t;

class BranchHistoryRegistor {
protected:
    uint size;
    bhr_content_t content;
public:
    BranchHistoryRegistor();
    BranchHistoryRegistor(bhr_content_t size);
    void update(bool dir);
    bhr_content_t getIndexOfPatternTable();
};

class BhrTable {
public:
    virtual bool contains(UINT64 PC) = 0;
    virtual BranchHistoryRegistor& get(UINT64 PC) = 0;
    virtual void update(UINT64 PC, bool dir) = 0;
    virtual ~BhrTable();
};

class BhrTableHash: public BhrTable {
private:
    unordered_map<UINT64, BranchHistoryRegistor> bhrTable;
public:
    bool contains(UINT64 PC);
    BranchHistoryRegistor& get(UINT64 PC);
    void update(UINT64 PC, bool dir);
};

// BhrTableAssoc begin
struct BhrTableAssocEntry {
    UINT64 tag;
    BranchHistoryRegistor bhr;
    BhrTableAssocEntry();
    BhrTableAssocEntry(UINT64 tag);
};

/*
  LRU structure to store BhrTable entries
*/
class BhrTableAssocSet {
private:
    list<BhrTableAssocEntry> entries;
    void touch(decltype(entries)::iterator);
public:
    BhrTableAssocSet();
    bool contains(UINT64 tag);
    BhrTableAssocEntry &get(UINT64 tag);
    void add(UINT64 tag);
};

class BhrTableAssoc: public BhrTable {
private:
    vector<BhrTableAssocSet> bhrTableSets;
public:
    BhrTableAssoc();
    bool contains(UINT64 PC);
    BranchHistoryRegistor& get(UINT64 PC);
    void update(UINT64 PC, bool dir);
};
// BhrTableAssoc end

class BranchPatternTable {
private:
    Automaton *automaton;
    vector<uint> bpTable;

public:
    BranchPatternTable(Automaton *automaton);
    bool predict(bhr_content_t index);
    void update(bhr_content_t index, bool dir);
};

class PREDICTOR{
private:
    BhrTable *bhrTable;
    Automaton *automaton;
    BranchPatternTable *bpTable;
public:

    PREDICTOR(void);

    // The interface to the functions below CAN NOT be changed
    bool    GetPrediction(UINT64 PC);  
    void    UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
    void    TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

    // Contestants can define their own functions below
    ~PREDICTOR();
};

#endif

