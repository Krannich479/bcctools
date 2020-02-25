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
    
    void resize(){
        seqan::resize(_table, (uint64_t)1 << (2*_readStructure.barcodeLength), 0);
    }
    
    
    void count(kseq_t * seq1){
        
        while (kseq_read(seq1) >= 0){
            
            seqan::Dna5String barcode;

            for (unsigned i = 0; i < seqan::length(_readStructure.barcodePositions); ++i)
            {
                seqan::Dna5String sub_barcode = seqan::infix(seq1->seq.s,
                                                             _readStructure.barcodePositions[i].i1,
                                                             _readStructure.barcodePositions[i].i2);
                
                appendValue(barcode, sub_barcode);
            }
        
            // check for Ns in barcode
            typename seqan::Iterator<seqan::Dna5String, seqan::Rooted>::Type it = begin(barcode);
            typename seqan::Iterator<seqan::Dna5String, seqan::Rooted>::Type itEnd = end(barcode);
            bool hasN = false;
            for (; it != itEnd; ++it)
            {
                if (*it == 'N')
                {
                    hasN = true;
                    break;
                }
            }
            
            // count barcode
            if (hasN == false)
            {
                seqan::DnaString barcode_copy = barcode;
                uint64_t h = hash(barcode_copy);
                if (_table[h] != seqan::maxValue<uint16_t>())       // avoid counting overflow
                    ++_table[h];
            }
        }
    }
    
    const TContainer& get_counts() const { return _table; }
    
};







#endif // BARCODE_COUNTER_H_
