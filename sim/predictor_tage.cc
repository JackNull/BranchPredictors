#include "predictor_tage.h"
#include <cstdlib>
#include <time.h>
#include <bitset>
#include <math.h>

#define BIMODAL_CTR_MAX  3
#define BIMODAL_CTR_INIT 2
#define TAGPRED_CTR_MAX  7
#define TAGPRED_CTR_INIT 0
#define BIMODALLOG   14

#define NUMCOMP 7
#define TAGWIDTH 11

#define TAGPREDLOG 12
#define MAXHIST 131
#define MINHIST 5

#define DEBUG 1

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////
// TAGGED PREDICTOR SIZE:  2^12 (TAGPRELOG) * 2 * (3 (ctr) + 2 (u) + 9(tag))
//                        +2^12 (TAGPRELOG) * 2 * (3 (ctr) + 2 (u) + 8(tag))
// BIMODAL PREDICTOR SIZE: 2^14 (BIMODALLOG) * 2
// TOTAL SIZE: BIMODAL + TAGPRED + GHR
/////////////////////////////////////////////////////////////

PREDICTOR::PREDICTOR(void)
{
    
    // First initiating Bimodal Table
    // Its a simple 2 bit counter table 
    
    bimodalLog = BIMODALLOG;
    bimodalsize = (1<< bimodalLog);
    bimodal = new UINT64[bimodalsize];

    for(UINT64 ii=0; ii< bimodalsize; ii++)
    {
        bimodal[ii]=BIMODAL_CTR_INIT;
    }
    
    // Next to initiating the taggedPredictors
    tagPredLog = TAGPREDLOG;
    compsize = (1 << tagPredLog);
    //cout << " No of entries in tag predictors = " << compsize << endl;          
    
    for(UINT64 ii = 0; ii < NUMCOMP ; ii++)
    {
       tagPred[ii] = new TagEntry[compsize];
    }
    for(UINT64 ii = 0; ii < NUMCOMP; ii++)
    {
        for(UINT64 j =0; j < compsize; j++)
        {
            tagPred[ii][j].ctr = 0;
            tagPred[ii][j].tag = 0;
            tagPred[ii][j].u = 0;
        }
      
    }
    
    // STORAGESIZE = 0;

    // compute geometric series
    geometric[0] = MAXHIST - 1;
    geometric[NUMCOMP-1] = MINHIST;
    for (int i = 1; i < NUMCOMP - 1; i++)
    {
        geometric[NUMCOMP-i-1] =
            (int) ((MINHIST * pow ((MAXHIST - 1) / (double) MINHIST,
                                    i / (double) (NUMCOMP - 1))) + 0.5);
    }

    if (DEBUG)
    {
        fprintf(stderr, "Geometric Series:");
        for (int i = NUMCOMP - 1; i >= 0; i--)
        {
            fprintf(stderr, "%d ", geometric[i]);
            //if(i < NUMCOMP/2)
                //STORAGESIZE += compsize * (2 + 3 + TAGWIDTH);
            //else
                //STORAGESIZE += compsize * (2 + 3 + (TAGWIDTH - 1));
        }
        // bimodal storage
        //STORAGESIZE += bimodalsize * 2;
        //fprintf(stderr, "\n");
        //fprintf(stderr, "Storage Size = %ld bits\n", STORAGESIZE);
    }
    
    // Initializing Compressed Buffers.
    // first for index of the the tagged tables
    for(int i = 0; i < NUMCOMP; i++)
    {
        indexComp[i].compHist = 0;
        indexComp[i].geomLength = geometric[i];
        indexComp[i].targetLength = TAGPREDLOG;
    }
    
    // The tables have different tag width
    // for 5-component predictor, tag width = 9/8 for T0-T1/T2-T3
    // for 8-component predictor, tag width = 11/10 for T0-T2/T3-T6
    for(int j = 0; j < 2 ; j++)
    {
        for(int i=0; i < NUMCOMP; i++)
        {
            tagComp[j][i].compHist = 0;
            tagComp[j][i].geomLength = geometric[i];
            if(j == 0)
            {
                if(i < NUMCOMP/2)
                    tagComp[j][i].targetLength = TAGWIDTH;
                else
                    tagComp[j][i].targetLength = TAGWIDTH - 1;    
            }
            else
            {
                if(i < NUMCOMP/2)
                    tagComp[j][i].targetLength = TAGWIDTH;
                else
                    tagComp[j][i].targetLength = TAGWIDTH - 1;
            }
        }   
    }

    // Preditions banks and prediction values 
    primePred = -1;
    altPred = -1;
    T_i = NUMCOMP;
    T_a = NUMCOMP;

    for(int i=0; i < NUMCOMP; i++)
    {    
        GI[i] = 0;
    }
    for(int i=0; i < NUMCOMP; i++)
    {    
        tag[i] = 0;
    }
    clock = 0;
    clock_flip = 1;
    PHR = 0;
    GHR.reset();
    altBetterCount = 8;
}

bool PREDICTOR::GetPrediction(UINT64 PC){
  
    // Base Prediction   
    bool basePrediction;
    UINT64 bimodalIndex   = (PC) % (bimodalsize);
    UINT64 bimodalCounter = bimodal[bimodalIndex];
  
    if(bimodalCounter > BIMODAL_CTR_MAX / 2)
    {
        basePrediction = 1; 
    }
    else
    {
        basePrediction = 0; 
    }
  
    // Hash to get tag includes info about bank, pc and global history compressed
    // formula given in PPM paper 
    // pc[9:0] xor CSR1 xor (CSR2 << 1)
    for(int i = 0; i < NUMCOMP; i++)
    { 
        tag[i] = PC ^ tagComp[0][i].compHist ^ (tagComp[1][i].compHist << 1);
        // These need to be masked
        // 9 bit tags for T0 and T1, 8 bit tags for T2 and T3
        if (i < NUMCOMP/2)
            tag[i] &= ((1 << TAGWIDTH) - 1);
        else
            tag[i] &= ((1 << (TAGWIDTH - 1)) - 1);
    }

    // compute tagged components index
    for (int i = 0; i < NUMCOMP; i++)
    {
        GI[i] = PC ^ (PC >> (TAGPREDLOG - i)) ^ indexComp[i].compHist ^ PHR ^ (PHR >> TAGPREDLOG);
    }
    
    UINT64 index_mask = ((1<<TAGPREDLOG) - 1);
    for(int i = 0; i < NUMCOMP; i++)
    {
        GI[i] = GI[i] & index_mask;
    }
       
    // get two predictions prime and alt (alternate)
    primePred = -1;
    altPred = -1;
    T_i = NUMCOMP;
    T_a = NUMCOMP;
       
    // See if any tag matches
    // provider component
    for(int i = 0; i < NUMCOMP; i++)
    {
        if(tagPred[i][GI[i]].tag == tag[i])
        {
            T_i = i;
            break;
        }
    }
    // alternate component
    for(int i = T_i + 1; i < NUMCOMP; i++)
    {
        if(tagPred[i][GI[i]].tag == tag[i])
        {
            T_a = i;
            break;
        }
    }
    // tag match hit
    if(T_i < NUMCOMP)
    {        
        if(T_a == NUMCOMP)
        {
            altPred = basePrediction;
        }
        else
        {
            if(tagPred[T_a][GI[T_a]].ctr >= TAGPRED_CTR_MAX/2)
                altPred = TAKEN;
            else 
                altPred = NOT_TAKEN;
        }
        
        if((tagPred[T_i][GI[T_i]].ctr != 3) ||(tagPred[T_i][GI[T_i]].ctr != 4 ) || (tagPred[T_i][GI[T_i]].u != 0) || (altBetterCount < 8))
        {
            if(tagPred[T_i][GI[T_i]].ctr >= TAGPRED_CTR_MAX/2)
                primePred = TAKEN;
            else 
                primePred = NOT_TAKEN;
            return primePred;
        }
        else
        {
            return altPred;
        }
    }
    // miss - return base prediction
    else
    {
        altPred = basePrediction;
        return altPred;
    }
}
 
void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget){
 
    bool strong_old_present = false;
    bool new_entry = 0;
    // tag match hit
    if (T_i < NUMCOMP)
    {
        // 1st update the useful counter if altpred != pred
        if (predDir != altPred)
        {
            // increment u if pref is correct
            if (predDir == resolveDir)
            {
                tagPred[T_i][GI[T_i]].u = SatIncrement(tagPred[T_i][GI[T_i]].u, BIMODAL_CTR_MAX);
            }
            else
            {
                tagPred[T_i][GI[T_i]].u = SatDecrement(tagPred[T_i][GI[T_i]].u);
            }
        }
        // 2nd update the prediction counter of the provider component 
        if(resolveDir)
        {
            tagPred[T_i][GI[T_i]].ctr = SatIncrement(tagPred[T_i][GI[T_i]].ctr, TAGPRED_CTR_MAX);
        }
        else
        {
            tagPred[T_i][GI[T_i]].ctr = SatDecrement(tagPred[T_i][GI[T_i]].ctr);
        }
    }
    // tag match miss
    else
    {
        UINT64 bimodalIndex = (PC) % (bimodalsize);
        if(resolveDir)
        {
            bimodal[bimodalIndex] = SatIncrement(bimodal[bimodalIndex], BIMODAL_CTR_MAX);
        }
        else
        {
            bimodal[bimodalIndex] = SatDecrement(bimodal[bimodalIndex]);
        }
    }
    // Check if the current Entry which gave the prediction is a newly allocated entry or not.
	if (T_i < NUMCOMP)
	{
	    if((tagPred[T_i][GI[T_i]].u == 0) &&((tagPred[T_i][GI[T_i]].ctr  == 3) || (tagPred[T_i][GI[T_i]].ctr  == 4)))
        {
            new_entry = true;
            if (primePred != altPred)
            {
                if (altPred == resolveDir)
                {
                // Alternate prediction more useful is a counter to be of 4 bits
                    if (altBetterCount < 15)
                    {  
                        altBetterCount++;
                    }    
                }
                else if (altBetterCount > 0)
                {
                    altBetterCount--;
                }
            }
	    }
	}

    // Proceeding to allocation of the entry.
    if((!new_entry) || (new_entry && (primePred != resolveDir)))
    {    
        if (((predDir != resolveDir) & (T_i > 0)))
        {           
            for (int i = 0; i < T_i; i++)
            {
                if (tagPred[i][GI[i]].u == 0)
                    strong_old_present = true;
            }
            // If no entry useful than decrease useful bits of all entries but do not allocate
            if (strong_old_present == false)
            {
                for (int i = T_i - 1; i >= 0; i--)
                {
                    tagPred[i][GI[i]].u--;
                }
            }
            // otherwise allocate entry with 2P(Tj) = P(Tk), j < k
            else
            {
                srand(time(NULL));
                int randNo = rand() % 100;
                int count = 0;
                int bank_store[NUMCOMP - 1] = {-1, -1, -1};
                int matchBank = 0;
                for (int i = 0; i < T_i; i++)
                {
                    if (tagPred[i][GI[i]].u == 0)
                    {
                        count++;
                        bank_store[i] = i;
                    }
                }  
                if(count == 1)
                {
                    matchBank = bank_store[0];
                }
                else if(count > 1)
                {
                    if(randNo > 33 && randNo <= 99)
                    {
                        matchBank = bank_store[(count-1)];
                    }
                    else
                    {
                        matchBank = bank_store[(count-2)];
                    }   
                }
        		for (int i = matchBank; i > -1; i--)
        		{
                    if ((tagPred[i][GI[i]].u == 0))
                    {
                        if(resolveDir)
                        {    
                            tagPred[i][GI[i]].ctr = 4;
                        }
                        else
                        {
                            tagPred[i][GI[i]].ctr = 3;
                        }    
                        tagPred[i][GI[i]].tag = tag[i];
                        tagPred[i][GI[i]].u = 0;
                        break;
                    }
                }
            }
        }
    }    


    // Periodically reset whole column of useful counter 
	clock++;
    // for every 256K instructions 1st MSB then LSB
	if(clock == (256*1024))
    {
        // reset clock
        clock = 0;
	    if(clock_flip == 1) // MSB
        {
            for (int jj = 0; jj < NUMCOMP; jj++)
            {    
                for (UINT64 ii = 0; ii < compsize; ii++)
                {
                    tagPred[jj][ii].u = tagPred[jj][ii].u & 1;
                }
            }
        }
        else // LSB
        {
            for (int jj = 0; jj < NUMCOMP; jj++)
            {    
                for (UINT64 ii = 0; ii < compsize; ii++)
                {
                    tagPred[jj][ii].u = tagPred[jj][ii].u & 2;
                }
            }
	    }
        clock_flip = 1 - clock_flip;
	}
    // update the GHR
    GHR = (GHR << 1);

    if(resolveDir == TAKEN)
    {
        GHR.set(0,1); 
    }

    for (int i = 0; i < NUMCOMP; i++)
    {
        indexComp[i].updateCompHist(GHR);
        tagComp[0][i].updateCompHist(GHR);
        tagComp[1][i].updateCompHist(GHR);
    }

    // PHR update is simple, jus take the last bit
    // Always Limited to 16 bits as per paper.
    PHR = (PHR << 1); 
    if(PC & 1)
    {
        PHR = PHR + 1;
    }
    PHR = (PHR & ((1 << 16) - 1));
}

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget){

    // This function is called for instructions which are not
    // conditional branches, just in case someone decides to design
    // a predictor that uses information from such instructions.
    // We expect most contestants to leave this function untouched.

    return;
}
