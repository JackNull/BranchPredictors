#include <algorithm>
#include "predictor_2level.h"
#include <limits>

using namespace std;

#define MAX_UINT_OF_SIZE(x) ((uint)((1 << ((uint)(x))) - 1))
#define BITS_OF_TYPE(x) ((sizeof(x)) * 8)

Automaton::Automaton(uint size): size(size) {}
Automaton::~Automaton() {}

AutomatonSature::AutomatonSature(uint size): Automaton(size) {}
uint AutomatonSature::nextState(uint state, bool dir) {
    return dir ? (state == MAX_UINT_OF_SIZE(size) ? state : state + 1) :
        (state == 0 ? 0 : state - 1);
}
bool AutomatonSature::predict(uint state) {
    return state & (uint)(1 << (size - 1));
}

BranchHistoryRegistor::BranchHistoryRegistor() {
    size = BHR_CONTENT_SIZE;
    content = 0;
}
BranchHistoryRegistor::BranchHistoryRegistor(bhr_content_t size): size(size) {
    content = 0;
}
void BranchHistoryRegistor::update(bool dir) {
    content = (content << 1) | dir;
}
bhr_content_t BranchHistoryRegistor::getIndexOfPatternTable() {
    return size >= BITS_OF_TYPE(bhr_content_t) ? content : content & MAX_UINT_OF_SIZE(size);
}

BhrTable::~BhrTable() {}

bool BhrTableHash::contains(UINT64 PC) {
    return bhrTable.count(PC);
}
BranchHistoryRegistor& BhrTableHash::get(UINT64 PC) {
    return bhrTable[PC];
}
void BhrTableHash::update(UINT64 PC, bool dir) {
    bhrTable[PC].update(dir);
}

// BhrTableAssoc begin

BhrTableAssocEntry::BhrTableAssocEntry() {
    tag = numeric_limits<UINT64>::max();
}

BhrTableAssocEntry::BhrTableAssocEntry(UINT64 tag): tag(tag) {}

BhrTableAssocSet::BhrTableAssocSet() {}

void BhrTableAssocSet::touch(list<BhrTableAssocEntry>::iterator it) {
    entries.splice(entries.end(), entries, it);
}

bool BhrTableAssocSet::contains(UINT64 tag) {
    return find_if(entries.begin(), entries.end(), [tag](const auto &e) { return e.tag == tag; }) != entries.end();
}

BhrTableAssocEntry &BhrTableAssocSet::get(UINT64 tag) {
    auto it = find_if(entries.begin(), entries.end(), [tag](const auto &e) { return e.tag == tag; });
    touch(it);
    return *it;
}

void BhrTableAssocSet::add(UINT64 tag) {
    if(entries.size() >= AHRT_ASSOC) {
        entries.erase(entries.begin());
    }
    entries.emplace_back(tag);
}


BhrTableAssoc::BhrTableAssoc() {
    bhrTableSets = decltype(bhrTableSets)(AHRT_SET_NUM);
}
bool BhrTableAssoc::contains(UINT64 PC) {
    return bhrTableSets[AHRT_ENTRY_INDEX(PC)].contains(AHRT_ENTRY_TAG(PC));
}
BranchHistoryRegistor &BhrTableAssoc::get(UINT64 PC) {
    auto index = AHRT_ENTRY_INDEX(PC);
    auto tag = AHRT_ENTRY_TAG(PC);
    auto &bhrSet = bhrTableSets[index];
    if(!bhrSet.contains(tag)) {
        bhrSet.add(tag);
    }
    return bhrSet.get(tag).bhr;
}
void BhrTableAssoc::update(UINT64 PC, bool dir) {
    auto &bhr = this->get(PC);
    bhr.update(dir);
}
// BhrTableAssoc end

BranchPatternTable::BranchPatternTable(Automaton *automaton): automaton(automaton) {
    bpTable = decltype(bpTable)(1 << BHR_CONTENT_SIZE);
}
bool BranchPatternTable::predict(bhr_content_t index) {
    return automaton->predict(bpTable[index]);
}
void BranchPatternTable::update(bhr_content_t index, bool dir) {
    bpTable[index] = automaton->nextState(bpTable[index], dir);
}

PREDICTOR::PREDICTOR() {
    #ifdef BHR_TABLE_ASSOC
    bhrTable = new BhrTableAssoc();
    #else
    bhrTable = new BhrTableHash();
    #endif // BHR_TABLE_ASSOC

    automaton = new AutomatonSature(AUTOMATON_STATE_SIZE);
    bpTable = new BranchPatternTable(automaton);
}

PREDICTOR::~PREDICTOR() {
    delete bhrTable;
    delete automaton;
    delete bpTable;
}

bool PREDICTOR::GetPrediction(UINT64 PC) {
    auto &bhr = bhrTable->get(PC);
    return bpTable->predict(bhr.getIndexOfPatternTable());
}

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 brachTarget) {
    auto old_index = bhrTable->get(PC).getIndexOfPatternTable();
    bhrTable->update(PC, resolveDir);
    bpTable->update(old_index, resolveDir);
}

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 brachTarget) {
    
}
