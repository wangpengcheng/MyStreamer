#ifndef BASE_STR_TOOLS_H
#define BASE_STR_TOOLS_H

#include <string>
#include "base_define.h"


// Trim spaces from the start of a string
std::string& StringLTrimg( std::string& s );
// Trim spaces from the end of a string
std::string& StringRTrim( std::string& s );
// Trim spaces from both ends of a string
std::string& StringTrim( std::string& s );

// Replace sub-string within a string
std::string& StringReplace( std::string& s, const std::string& lookFor, const std::string& replaceWith );

#endif //BASE_STR_TOOLS_H