/**
 * @file base_error.h
 * @brief 定义基础错误类信息
 * @details 声明简单的基础错误类信息支持错误类的定义和扩展
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 12:30:38
 * @copyright Copyright (c) 2020  IRLSCU
 * @todo 添加错误码支持和错误信息支持
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Commit date</th>
 *    <th>Version</th> 
 *    <th>Author</th>  
 *    <th>Description</th>
 * <tr>
 *    <td>2020-12-15 12:30:38</td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>基础错误定义头文件</td>
 * </tr>
 * </table>
 * 
 */
#ifndef BASE_ERROR_H
#define BASE_ERROR_H

#include <string>
#include "base_tool.h"

NAMESPACE_START
/**
 * @brief 基础错误定义类,支持各种错误信息输出
 */
class BaseError
{
public:
    enum BaseErrorCode
    {
        Success = 0,               ///< Generic Success
        Failed,                    ///< Generic failure
        NullPointer,               ///< Input parameter is a null pointer
        OutOfMemory,               ///< Out of memory
        IOError,                   ///< I/O error
        DeivceNotReady,            ///< Device (whatever it might be) is not ready for the requested action
        ConfigurationNotSupported, ///< Configuration is not supported by device/object/whoever
        UnknownProperty,           ///< Specified property is not known
        UnsupportedProperty,       ///< Specified property is not supported by device/object/whoever
        InvalidPropertyValue,      ///< Specified property value is not valid
        ReadOnlyProperty,          ///< Specified property is read only
        UnsupportedPixelFormat,    ///< Pixel format (of an image) is not supported
        ImageParametersMismatch,   ///< Parameters of images (width/height/format) don't match
        FailedImageEncoding        ///< Failed image encoding
    };

public:
    /**
     * @brief Construct a new Base Error object
     * @param  code             错误码编号
     */
    BaseError(BaseErrorCode code = Success);
    ~BaseError();
    /**
     * @brief Get the error code
     */
    operator BaseErrorCode() const { return mCode; }
    /**
     * @brief Check if error code is Success
     */
    operator bool() const { return (mCode == Success); }
    /**
     * @brief Get error code as integer
     * @return int error code
     */
    int Code() const { return static_cast<int>(mCode); }
    /**
     * @brief  输出错误对应string
     * @return std::string 
     */
    std::string ToString() const;

private:
    BaseErrorCode mCode;
};
typedef BaseError Error;

NAMESPACE_END //namespace

#endif // BASE_ERROR_H