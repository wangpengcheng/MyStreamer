
#include "base_error.h"

NAMESPACE_START 

/* 预定于错误提示信息 */
static const char* ErrorMessages[] =
{
    "Success",

    "Generic failure",
    "Input parameter is a null pointer",
    "Out of memory",
    "I/O error",
    "Device is not ready",
    "Configuration is not supported",
    "Property is not known",
    "Property is not supported",
    "Property value is not valid",
    "Property is read only",
    "Pixel format is not supported",
    "Parameters of images don't match",
    "Failed image encoding"
};


BaseError::BaseError(BaseErrorCode code) : mCode(code)
{
    
}

BaseError::~ BaseError()
{
}

std::string BaseError::ToString( ) const
{
        std::string ret;

    if ( ( mCode >= 0 ) && ( mCode < sizeof( ErrorMessages ) / sizeof( ErrorMessages[0] ) ) )
    {
        ret = ErrorMessages[mCode];
    }
    else
    {
        char buffer[64];

        sprintf( buffer, "Unknown error code: %d", mCode );
        ret = buffer;
    }

    return ret;
}

NAMESPACE_END
