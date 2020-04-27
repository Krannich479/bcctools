#include "Barcode_counter.h"
#include "utils.h"


template <class TContainer>
void BarcodeCounter<TContainer>::count(kseq_t * seq1)
{
    while (kseq_read(seq1) >= 0){
        
        seqan::Dna5String barcode;

        // this section is necessary because some technologies might use fractioned barcodes
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
