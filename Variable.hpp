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

#include <string>

using namespace std;

// This structure describes an exported variable
typedef struct
{
    // Name of the variable
    string SymbolName;

    // True if the variable is found in the .SYM_XML file. Used to ensure that the symbol and the project files are consistent
    bool Used = false;
} VARIABLE;
