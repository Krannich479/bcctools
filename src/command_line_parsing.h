#ifndef COMMAND_LINE_PARSING_H_
#define COMMAND_LINE_PARSING_H_

#include <seqan/arg_parse.h>

enum Command
{
    BC_INFER_WHITELIST = 0,
    BC_INDEX = 1,
    BC_CORRECT = 2,
    BC_STATS = 3
};

enum Platform
{
    CUSTOM = 0,
    CHROMIUM = 1,
    TELLSEQ= 2,
    STLFR = 3
};

struct ReadStructure
{
    bool read;      // 0 = barcode in first read in pair, 1 = barcode in second read in pair

    int barcodeLength;
    seqan::String<seqan::Pair<unsigned> > barcodePositions;     // { [startpos, endpos[, ... }
    seqan::String<seqan::Pair<unsigned> > forwardSequencePositions; //{[seqStartPos, seqEndPos[, ... }
    seqan::String<seqan::Pair<unsigned> > reverseSequencePositions;

    typedef seqan::Iterator<const seqan::CharString, seqan::Rooted>::Type TIter;

private:
    // Parse and return a number in the structure string.
    inline unsigned parseNumber(std::string & buffer, TIter & it)
    {
        SEQAN_ASSERT(seqan::empty(buffer));
        while (std::isdigit(*it))
        {
            buffer += *it;
            ++it;
            if (seqan::atEnd(it))
            {
                std::stringstream what;
                what << "Structure-string ending with numeric value. Each position must be followed by an identifier"
                     << " (one of 'x', 'b', and 's'). See README for more details.";
                SEQAN_THROW(seqan::ParseError(what.str()));
            }
        }
        unsigned num = std::stoi(buffer);
        seqan::clear(buffer);
        return num;
    }

    // Return -1 if the string ended, 0 if we are still on the same read and 1 if the read changed.
    inline int parseIdentifier(std::string & buffer, TIter & it, const bool & isFirstRead)
    {
        SEQAN_ASSERT(seqan::empty(buffer));
        if (*it == 'b' || *it == 'x' || *it == 's')
            buffer += *it;
        else
        {
            std::stringstream what;
            what << "All positions in the structure-string must be followed by an an identifier "
                 << "(one of 'x', 'b', and 's'). See README for details.";
            SEQAN_THROW(seqan::ParseError(what.str()));
        }
        ++it;
        if (atEnd(it))
        {
            return -1;
        }
        else if (std::isdigit(*it))
        {
            return 0;
        }
        else if ((*it == 'r' && isFirstRead) || (*it == 'f' && !isFirstRead))
        {
            return 1;
        }
        else if ((*it == 'r' && isFirstRead) || (*it == 'f' && !isFirstRead))
        {
            std::stringstream what;
            what << "Structure-string contains forward read identifier \'" << *it << "\' two times, but must only contain each read identifier exactly once. See README for details.";
            SEQAN_THROW(seqan::ParseError(what.str()));
        }
        else
        {
            std::stringstream what;
            what << "Each position identifier in the structure-string must be followed by a numeric position, a read identifiert or the end of the string. See readme for details.";
            SEQAN_THROW(seqan::ParseError(what.str()));
        }
    }
    // Parse the part of the structure string that belong to a single read.
    // Return true at the end of the current read.
    // Return false at the end of the whole structure string.
    inline bool parseSingleRead(TIter & it, bool & noBarcodeYet, bool & currentlyOnSecondRead)
    {
        unsigned currentPos = 0;
        std::string buffer;
        seqan::reserve(buffer, 3);
        while (!seqan::atEnd(it))
        {
            unsigned start = currentPos;
            unsigned end = currentPos + parseNumber(buffer, it);
            int retCode = parseIdentifier(buffer, it, !read);
            if (buffer[0] == 'b')
            {
                if (noBarcodeYet)
                {
                    read = currentlyOnSecondRead;
                    noBarcodeYet = false;
                }

                seqan::appendValue(barcodePositions, seqan::Pair<unsigned>(start, end));
            }
            else if (buffer[0] == 's')
            {
                if (currentlyOnSecondRead)
                    seqan::appendValue(reverseSequencePositions, seqan::Pair<unsigned>(start, end));
                else
                    seqan::appendValue(forwardSequencePositions, seqan::Pair<unsigned>(start, end));
            }
            if (retCode == -1)
                return false;

            else if (retCode == 0)
            {
                seqan::clear(buffer);
                currentPos = end;
                continue; // continue on the same read
            }
            else
            {
                ++it;
                currentlyOnSecondRead = !currentlyOnSecondRead;
                return true;   //continue on the other read.
            }
        }
        return false;
    }
    // Check if the positions in the ReadStructure are not empty and throw an error otherwise.
    inline void checkAndThrow()
    {
        std::stringstream what;
        bool e = false;
        if (empty(barcodePositions))
        {
            what << "No barcode positions found in structure string! ";
            e = true;
        }
        if (empty(forwardSequencePositions) && empty(reverseSequencePositions))
        {
            what << "No sequence positions found in structure string!";
            e = true;
        }
        if (e)
            SEQAN_THROW(seqan::ParseError(what.str()));
    }
    // Calculate and return the total length of the barcode sequence.
    inline unsigned getBarcodeLenght()
    {
        barcodeLength = 0;
        for (unsigned i = 0; i < length(barcodePositions); ++i)
        {
            barcodeLength += barcodePositions[i].i2 - barcodePositions[i].i1;
        }
    }
    // Print the content of the ReadStructure to the standard output.
    inline void print() const
    {
        std::cout << "Read structure initialized with barcode  of length " << barcodeLength << " at: "
                  << barcodePositions << "in read " << read + 1 << std::endl;
        std::cout << "Forward sequence at: " << forwardSequencePositions << std::endl;
        std::cout << "Reverse sequence at " << reverseSequencePositions <<std::endl;
    }

public:
    ReadStructure()
    {
        read = 0;
        barcodeLength = 0;
    }

    ReadStructure(seqan::CharString s)
    {
        //F10b6x10b6x10b100sR...
        SEQAN_ASSERT(!empty(s));
        toLower(s);
        std::stringstream what;
        TIter it = seqan::begin(s);

        // See if we currently are in the forward or reverse read.
        bool noBarcodeYet = true;
        bool currentlyOnSecondRead;
        if (*it == 'f')
            currentlyOnSecondRead = 0;
        else if (*it == 'r')
            currentlyOnSecondRead = 1;
        else
        {
            what << "Invalid read specifier " << *it << " at start of structure-string. Must be one of 'F', 'R'. "
                 << "See README for more details.";
            SEQAN_THROW(seqan::ParseError(what.str()));
        }
        ++it;
        // Now parse the rest of the structure String.
        if (parseSingleRead(it, noBarcodeYet, currentlyOnSecondRead))
            parseSingleRead(it, noBarcodeYet, currentlyOnSecondRead);

        checkAndThrow();
        getBarcodeLenght();
        print();                // This line is totally optional and can be removed if you don't want the info.
     }
};

struct Options
{
    Command cmd;

    Platform          platform;
    seqan::CharString structure;
    seqan::CharString whitelistFile;
    seqan::CharString fastqFile1;
    seqan::CharString fastqFile2;
    seqan::CharString inputFile;
    seqan::CharString outFile;
    ReadStructure readStructure;

    int bcLength;
    int spacerLength;
    unsigned whitelistCutoff;
    double minEntropy;
    unsigned numAlts;

    Options() :
        bcLength(16), spacerLength(7), whitelistCutoff(0), minEntropy(0.5), numAlts(16)
    {}
};

bool fileExists(seqan::CharString const & filename);
seqan::ArgumentParser::ParseResult parseCommandLine(Options & options, int argc, char const ** argv);

#endif  // COMMAND_LINE_PARSING_H_
