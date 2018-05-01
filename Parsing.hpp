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

#include "POU.hpp"
#include <vector>
#include <string>

using namespace std;

// Values describing the type of POU found
#define POU_NONE 0
#define POU_GLOBAL_VAR 1
#define POU_PROGRAM 2

// POU keywords
#define PROGRAM_IDENTIFIER "PROGRAM"
#define GLOBAL_VAR_IDENTIFIER "VAR_GLOBAL"

// Export tag
#define EXPORT_TAG "EXPORT"

// Comments
#define COMMENT_START "(*"
#define COMMENT_END "*)"

// Portable version of strcmpi
bool StrCmpI(const char* str1, const char* str2);

// Index all the exported variables of a project file
void IndexExportedVariables(vector<POU>& index, const char* buffer, const unsigned int projectsize);

// Skip a comment, starting from an offset inside the string
void SkipComment(const string& declaration, unsigned int& offset);

// Try to find a POU name
bool GetProgramName(const string& declaration, unsigned int& offset, string& name);
