#include <algorithm>
#include <functional>
#include "base_str_tools.h"


NAMESPACE_START
// 去除开头的空字符
std::string& StringLTrimg( std::string& s )
{
    s.erase( s.begin( ), std::find_if( s.begin( ), s.end( ),
        std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
    return s;
}
//去除尾部空字符
std::string& StringRTrim( std::string& s )
{
    s.erase( std::find_if( s.rbegin( ), s.rend( ),
        std::not1( std::ptr_fun<int, int>( std::isspace ) ) ).base( ), s.end( ) );
    return s;
}
//去除首位空字符
std::string& StringTrim( std::string& s )
{
    return StringLTrimg( StringRTrim( s ) );
}
//空字符串的代替
// Replace sub-string within a string
std::string& StringReplace(std::string& s, const std::string& lookFor, const std::string& replaceWith )
{
    if ( !lookFor.empty( ) )
    {
        size_t index          = 0;
        size_t lookForLen     = lookFor.length( );
        size_t replaceWithLen = replaceWith.length( );
        
        while ( index != std::string::npos )
        {
            index = s.find( lookFor, index );

            if ( index != std::string::npos )
            {
                s.replace( index, lookForLen, replaceWith );

                index += replaceWithLen;
            }
        }
    }

    return s;
}

NAMESPACE_END