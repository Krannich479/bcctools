#ifndef BARCODE_COUNTER_H_
#define BARCODE_COUNTER_H_

#include <seqan/sequence.h>     // include for Dna5String
#include "infer_whitelist.h"    // include for kseq_read


template<class TContainer>
class BarcodeCounter
{
    BarcodeCounter(const int barCodeLength) : _bcLength(barCodeLength) {}
    
    TContainer _table;

    int _bcLength;
    
    
public:
    
    void resize_10X(){
        seqan::resize(_table, (uint64_t)1 << (2*_bcLength), 0);
    }
    
    
    void count_10X(kseq_t * seq1){
        
        while (kseq_read(seq1) >= 0){
        
            seqan::Dna5String rx = prefix(seq1->seq.s, _bcLength);
            typename seqan::Iterator<seqan::Dna5String, seqan::Rooted>::Type it = begin(rx);
            typename seqan::Iterator<seqan::Dna5String, seqan::Rooted>::Type itEnd = end(rx);
            bool hasN = false;
            for (; it != itEnd; ++it)
            {
                if (*it == 'N')
                {
                    hasN = true;
                    break;
                }
            }
            if (hasN == false)
            {
                seqan::DnaString rrx = rx;
                uint64_t h = hash(rrx);
                if (_table[h] != seqan::maxValue<uint16_t>())
                    ++_table[h];
            }
        }
    }
    
    const TContainer& get_counts() const { return _table; }
    
};







#endif // BARCODE_COUNTER_H_
