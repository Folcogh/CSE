// CSE (CoDeSys Symbol Exporter)
// Copyright(C) 2018 Martial Demolins AKA Folco
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "Parsing.hpp"
#include <string>
#include <locale>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>

// POU header:
// - 8 bytes 0xCD
// - 8 words 0x0200
static const char POUheader[] = {
    (char)0xCD, (char)0xCD, (char)0xCD, (char)0xCD, (char)0xCD, (char)0xCD, (char)0xCD, (char)0xCD,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00
};

// Ignored Keywords which can be found in a POU
static const vector<string> IgnoredKeywords = {
    "CONSTANT",
    "END_VAR",
    "FUNCTION",
    "FUNCTIONBLOCK",
    "FUNCTION_BLOCK",
    "PERSISTENT",
    "RETAIN",
    "TYPE",
    "VAR",
    "VAR_GLOBAL"
};

// Case insensitive string comparison
bool StrCmpI(const char* str1, const char* str2)
{
    while (*str1 && *str2) {
        if ((isalpha(*str1) && ((*str1 | 0x20) == (*str2 | 0x20)))
            || (*str1 == *str2)) {
            str1++, str2++;
            continue;
        }
        return false;
    }
    return *str1 == *str2;
}


// Parse the project file and index all exported variables
//
// Declaration rules in a CoDeSys file:
// - the PROGRAM/FUNCTION/FUNCTION_BLOCK keywords may be separated from the POU name with comments or EOL
// - a declaration may look like:
//      - symbol identifier
//      - 'AT' + '%' + 2 letters + <int>.<int> OR '*' (optional)
//      - ':'
//      - a type
//      - ':=' + <constant> (optional)
//      - ';'
//
// Comments and EOL may be inserted anywhere between the fields (but inside the address)
//
// Nevertheless, the syntax is not checked. It's possible to fool the parser with an invalid POU, but we don't care.
// It's the programmer responsibility to check that its program compiles fine.
// We just read a symbol name, and find ';' to know where the variable declaration terminates.
// We look into the next comment, possibly nested, to know if the variable is exported thanks to the export tag
void IndexExportedVariables(vector<POU>& index, const char* buffer, const unsigned int projectsize)
{
    //
    // Project parsing
    //
    for (unsigned int POUoffset = 0; POUoffset < projectsize; POUoffset ++) {
        // Search a POU header
        if (memcmp(&(buffer[POUoffset]), POUheader, sizeof(POUheader))) {
            continue;
        }
        POUoffset += sizeof(POUheader);

        // POU found, read its size (4 bytes, low endian)
        unsigned int Length = (unsigned char)buffer[POUoffset] +
            ((unsigned char)buffer[POUoffset + 1] << 8) +
            ((unsigned char)buffer[POUoffset + 2] << 16) +
            ((unsigned char)buffer[POUoffset + 3] << 24);
        POUoffset += 4;

        // Don't parse the POU if it's size is null or rogue
        // This prevents to parse out of the buffer
        if ((Length == 0) || (Length > projectsize - POUoffset)) {
            continue;
        }

        // Initialize the POU object with a default name
        POU pou;
        pou.POUname = string("");

        // Write the declaration field in a string, to be able to use the string methods
        string Declaration("");
        for (unsigned int i = 0; i < Length; i++, POUoffset++) {
            Declaration.push_back(buffer[POUoffset]);
        }

        //
        // POU identifier parsing
        //

        int POUtype = POU_NONE;  // Become true when an identifier is found in the declaration
        unsigned int Offset = 0; // Offset of the parser in the declaration

        // Look for an identifier. Exported variables can be found in programs and global variable lists.
        // So we look for the string "PROGRAM" or "GLOBAL_VAR"
        while (Offset < Declaration.size()) {
            // Program ?
            if (Declaration.compare(Offset, sizeof(PROGRAM_IDENTIFIER) - 1, PROGRAM_IDENTIFIER) == 0) {
                Offset += sizeof(PROGRAM_IDENTIFIER) - 1;
                if (GetProgramName(Declaration, Offset, pou.POUname)) {
                    POUtype = POU_PROGRAM;
                }
                break;
            }
    
            // Global var ?
            else if (Declaration.compare(Offset, sizeof(GLOBAL_VAR_IDENTIFIER) - 1, GLOBAL_VAR_IDENTIFIER) == 0) {
                POUtype = POU_GLOBAL_VAR;
                break;
            }

            // Comment ?
            else if (Declaration.compare(Offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
                SkipComment(Declaration, Offset);
            }

            // Else skip current char
            else {
                Offset ++;
            }
        }

        // If no identifier is found, or the end of the POU is reached, don't index the POU
        if ((POUtype == POU_NONE) || (Offset > + Declaration.size())) {
            continue;
        }

        // We need an identifier between a program and its name, else we skip this POU
        if (POUtype == POU_PROGRAM) {
            unsigned char Chr = Declaration[Offset];
            unsigned char Chr2 = 0;
            if (Declaration.size() > Offset + 2) {
                Chr2 = Declaration[Offset + 1];
            }

            if (!((Chr == ' ') || (Chr == '\t') || (Chr == 0x0D) || ((Chr == '(') && (Chr2 == '*')))) {
                continue;
            }
        }

        // The POU seems valid, we can index it
        index.push_back(pou);

        //
        // Exported variables parsing
        //

        while (Offset < Declaration.size()) {
            string SymbolName("");
            // Symbol ?
            if (isalpha(Declaration[Offset])) {
                // Read the symbol
                while (isalnum(Declaration[Offset]) || (Declaration[Offset] == '_')) {
                    SymbolName.push_back(Declaration[Offset++]);
                }

                // If this is a keyword, skip it
                bool IgnoreKeyword = false;
                for (unsigned int i = 0; i < IgnoredKeywords.size(); i++) {
                    if (IgnoredKeywords[i] == SymbolName) {
                        IgnoreKeyword = true;
                    }
                }
                if (IgnoreKeyword) {
                    continue;
                }

                // Look for a semicolon
                bool SemicolonFound = false;
                while (Offset < Declaration.size()) {
                    // Semicolon ?
                    if (Declaration[Offset] == ';') {
                        SemicolonFound = true;
                        break;
                    }

                    // Comment ?
                    else if (Declaration.compare(Offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
                        SkipComment(Declaration, Offset);
                    }

                    // Skip any other char
                    else {
                        Offset ++;
                    }
                }

                // Check that a semicolon were found, else continue with the next POU
                if (!SemicolonFound) {
                    continue;
                }

                // Check for the export tag in the next comment
                while (Offset < Declaration.size()) {
                    // Comment ?
                    if (Declaration.compare(Offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
                        unsigned int InitOffset = Offset;
                        SkipComment(Declaration, Offset);
                        // Check if a export tag is inside the comment
                        auto TagOffset = Declaration.find(EXPORT_TAG, InitOffset);
                        if ((TagOffset != string::npos) && (TagOffset < Offset)) {
                            VARIABLE variable;
                            variable.SymbolName = SymbolName;
                            index.back().Variables.push_back(variable);
                        }

                        // Continue with the next symbol
                        break;
                    }

                    // Symbol ?
                    else if (isalpha(Declaration[Offset])) {
                        break;
                    }

                    // Skip any other char
                    else {
                        Offset ++;
                    }
                }
            }

            // Comment ?
            else if (Declaration.compare(Offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
                SkipComment(Declaration, Offset);
            }

            // Skip any other char
            else {
                Offset ++;
            }
        }
    }
}


// Find a program name, return true if one is found   
bool GetProgramName(const string& declaration, unsigned int& offset, string& name)
{
    while (offset < declaration.size()) {
        // Symbol ?
        if (isalpha(declaration[offset])) {
            while (isalnum(declaration[offset]) || (declaration[offset] == '_')) {
                name.push_back(declaration[offset++]);
            }
            return true;
        }

        // Comment ?
        else if (declaration.compare(offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
            SkipComment(declaration, offset);
            offset --; // The loop will skip the last comment char
         }

         // Skip any other char
        else {
            offset ++;
        }
    }

    // No name found
    return false;
}


// Skip a comment recursively, for nested comments
void SkipComment(const string& declaration, unsigned int& offset)
{
    // Skip comment begin marker
    offset += 2;

    // Parse the declaration
    for (; offset < declaration.size(); offset ++) {
        // End of comment ?
        if (declaration.compare(offset, sizeof(COMMENT_END) - 1, COMMENT_END) == 0) {
            offset += 2;
            return;
        }

        // Nested comment ?
        if (declaration.compare(offset, sizeof(COMMENT_START) - 1, COMMENT_START) == 0) {
            SkipComment(declaration, offset);
        }
    }
}
