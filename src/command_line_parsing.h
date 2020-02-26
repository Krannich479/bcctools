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

inline unsigned parseNumber(string & buffer, seqan::Iterator<const seqan::CharString, seqan::Rooted()>::Type & it)
{
    SEQAN_ASSERT(empty(buffer));
    while (isNumeric(*it))
    {
        buffer += *it;
        ++it;
        if (!seqan::atEnd(it))
        {
            std::stringstream what;
            what << "Structure-string ending with numeric value. Each position must be followed by an identifier (one of 'x', 'b', and 's'). See README for more details.";
            SEQAN_THROW(ParseError(what.str()));
        }
    }
    unsigned num = std::stoi(buffer);
    seqan::clear(buffer);
    return num;
}

// return -1 if the string ended, 0 if we are still on the same read and 1 if the read changed.
inline int parseIdentifier(string & buffer,
                           seqan::Iterator<const seqan::CharString, seqan::Rooted()>::Type & it,
                           const bool & isFirstRead)
{
    SEQAN_ASSERT(empty(buffer));
    if (*it == 'b' || *it == 'x' || *it == 's')
        buffer += *it;
    else
    {
        std::stringstream what;
        what << "All positions in the structure-string must be followed by an an identifier (one of 'x', 'b', and 's'). See README for details.";
        SEQAN_THROW(ParseError(what.str()));
    }
    ++it;
    if (atEnd(it))
        return -1;
    else if (isNumeric(*it))
    {
        return 0;
    }
    else if ((*it == 'R' && isFirstRead) || (*it == 'F' && !isFirstRead))
    {
        return 1;
    }
    else if ((*it == 'F' && isFirstRead) || (*it == 'R' && !isFirstRead))
    {
        std::stringstream what;
        what << "Structure-string contains forward read identifier \'" << *it << "\' two times, but must only contain each read identifier exactly once. See README for details.";
        SEQAN_THROW(ParseError(what.str()));
    }
    else
    {
        std::stringstream what;
        what << "Each position identifier in the structure-string must be followed by a numeric position, a read identifiert or the end of the string. See readme for details.";
        SEQAN_THROW(ParseError(what.str()));
    }
}

struct ReadStructure
{
    bool read;      // 0 = barcode in first read in pair, 1 = barcode in second read in pair

    int barcodeLength;
    seqan::String<seqan::Pair<unsigned> > barcodePositions;     // ( [startpos,endpos[, ...)
    seqan::Pair<seqan::Pair<unsigned> > forwardSequencePositions; //<[seqStartPosR1, seqEndPosR2[, [seqStartPosR2, seqEndPosR2[>
    seqan::Pair<seqan::Pair<unsigned> > reverseSequencePositions;

    void ReadStructure(seqan::CharString s)
    {
        //F10b6x10b6x10b100sR...
        SEQAN_ASSERT(!empty(s));
        toLower(s);
        std::stringstream what;
        seqan::Iterator<const seqan::CharString, seqan::Rooted()>::Type it = seqan::begin(s);

        // See if we currently are in the forward or reverse read.
        bool noBarcodeYet = true;
        bool currentlyOnSecondRead;
        if (*it == 'f')
            currentlyOnSecondRead = 0;
        else if (*it == 'r')
            currentlyOnSecondRead = 1;
        else
        {
            what << "Invalid read specifier " << *it << " at start of structure-string. Must be one of 'F', 'R'. See README for more details.";
            SEQAN_THROW(ParseError(what.str()));
        }
        // Now parse the rest of the structure String.
        unsigned currentPos = 0;
        string buffer;
        reserve(buffer, 3);
        while (!seqan::atEnd(it))
        {
            unsigned start = currentPos;
            unsigned end = currentPos + parseNum(it);
            int retCode = parseIdentifier(buffer, it, read);
            if (buffer[0] == 'b')
            {
                if (noBarcodeYet)
                    read = currentlyOnSecondRead;

                seqan::appendValue(barcodePositions, seqan::Pair<unsigned>(start, end));
            }
            else if (buffer[0] == 's')
                if (currentlyOnSecondRead)
                    seqan::appendValue(reverseSequencePositions, seqan::Pair<unsigned>(start, end));
                else
                    seqan::appendValue(forwardSequencePositions, seqan::Pair<unsigned>(start, end));

            if (retCode == -1)
                ;// done

            else if (retCode == 0)
                clear(buffer);
                continue; // continue on the same read
            else
            {
                currentlyOnSecondRead = !currentlyOnSecondRead; //continue on the other read.
                // Probably continue coding here
            }
        }
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
