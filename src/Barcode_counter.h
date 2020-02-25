#ifndef BARCODE_COUNTER_H_
#define BARCODE_COUNTER_H_

#include <seqan/sequence.h>         // include for Dna5String
#include "infer_whitelist.h"        // include for kseq_read
#include "command_line_parsing.h"   // include for ReadStructure struct


template<class TContainer>
class BarcodeCounter
{
    BarcodeCounter(const ReadStructure &rs) : _readStructure(rs) {}
    
    TContainer _table;

    ReadStructure _readStructure;
    
    
public:
    
    void resize(){ seqan::resize(_table, (uint64_t)1 << (2*_readStructure.barcodeLength), 0); }

    /**
     * @brief   This function counts the occurrences of each barcode
     * @param   seq1 are the reads from a FASTQ file
     */
    void count(kseq_t * seq1);
    
    const TContainer& get_counts() const { return _table; }
    
};







#endif // BARCODE_COUNTER_H_
