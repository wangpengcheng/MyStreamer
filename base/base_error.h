/**
 * @file base_error.h
 * @brief 
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 00:44:38
 * @copyright Copyright (c) 2020  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Date</th>
 *    <th>Version</th> 
 *    <th>Author</th>  
 *    <th>Description</th>
 * <tr>
 *    <td>2020-12-15 00:44:38 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>定义基础的错误类和相关字段</td>
 * </tr>
 * </table>
 */
#ifndef BASE_ERROR_H
#define BASE_ERROR_H

#include <string>
#include "base_tool.h"

NAMESPACE_START

class  BaseError
{
public:
    enum BaseErrorCode
    {
        Success = 0,

        Failed,                     // Generic failure
        NullPointer,                // Input parameter is a null pointer
        OutOfMemory,                // Out of memory
        IOError,                    // I/O error
        DeivceNotReady,             // Device (whatever it might be) is not ready for the requested action
        ConfigurationNotSupported,  // Configuration is not supported by device/object/whoever
        UnknownProperty,            // Specified property is not known
        UnsupportedProperty,        // Specified property is not supported by device/object/whoever
        InvalidPropertyValue,       // Specified property value is not valid
        ReadOnlyProperty,           // Specified property is read only
        UnsupportedPixelFormat,     // Pixel format (of an image) is not supported
        ImageParametersMismatch,    // Parameters of images (width/height/format) don't match
        FailedImageEncoding         // Failed image encoding
    };
public:
    BaseError(BaseErrorCode code = Success);
    ~ BaseError();

    // Get the error code
    operator BaseErrorCode( ) const { return mCode; }
    // Check if error code is Success
    operator bool( ) const { return ( mCode == Success ); }
    // Get error code as integer
    int Code( ) const { return static_cast<int>( mCode ); }

    std::string ToString( ) const;

private:
    BaseErrorCode mCode;
};
typedef BaseError Error;

NAMESPACE_END //namespace 

#endif // BASE_ERROR_H