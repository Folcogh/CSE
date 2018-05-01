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

#pragma once

// Extension of a CoDeSys project file
#define PRO_FILE_EXT ".pro"

// Extension of a symbol file created when compiling a project file
#define SYM_FILE_EXT ".SYM_XML"

// Filename addition to modified symbol files
#define CSE_FILENAME_ADDITION " - EXPORT"

// First bytes of a .pro file
#define CODESYS_PROJECT_SIGNATURE "CoDeSys"

// Start/End of the symbol list in the symbol files
#define SYM_BLOCK_START "<SymbolVarList>"
#define SYM_BLOCK_END "</SymbolVarList>"
