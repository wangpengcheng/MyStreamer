/**
 * @file base_obj_information.h
 * @brief 基础信息解析相关类
 * @details 包含BaseObjectInformation对应的接口以及简单实现类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 17:32:28
 * @copyright Copyright (c) 2020  IRLSCU
 *
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Commit date</th>
 *    <th>Version</th>
 *    <th>Author</th>
 *    <th>Description</th>
 * <tr>
 *    <td>2020-12-15 17:32:28 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>添加注释信息</td>
 * </tr>
 * </table>
 */
#ifndef BASE_OBJ_INFORMATION_H
#define BASE_OBJ_INFORMATION_H

#include <string>
#include <map>
#include "base_error.h"

NAMESPACE_START
/**
 * @brief 属性map别名,用来定义属性名称和值的变量集合
 */
typedef std::map<std::string, std::string> PropertyMap;

/**
 * @brief 信息解析对象相关接口类
 */
class BaseObjectInformation
{
public:
    /**
     * @brief Destroy the Base Object Information object
     * @attention 接口虚继承，需要将析构函数定义为虚函数，防止向下转换时的内存泄露
     */
    virtual ~BaseObjectInformation() {}
    /**
     * @brief Get the Property object 获取属性接口名称
     * @param  propertyName     属性的名称
     * @param  value            返回的值
     * @return Error            错误信息 @see ::BaseError
     */
    virtual Error GetProperty(const std::string &propertyName, std::string &value) const = 0;
    /**
     * @brief Get the All Properties object
     * @return PropertyMap 属性图 @see ::PropertyMap
     */
    virtual PropertyMap GetAllProperties() const = 0;
};
/**
 * @brief ObjectInformationMap 信息对象实现类，只要是对BaseObjectInformation的接口进行实现
 */
class ObjectInformationMap : public BaseObjectInformation
{
public:
    /**
     * @brief Construct a new Object Information Map object
     * @param  infoMap          属性Map元素
     */
    ObjectInformationMap(const PropertyMap &infoMap) : InfoMap(infoMap) {}
    /**
     * @brief Get the Property object
     * @param  propertyName     属性名称
     * @param  value            对应的值
     * @return Error            错误信息 @see ::BaseError
     */
    virtual Error GetProperty(const std::string &propertyName, std::string &value) const
    {
        Error ret = Error::UnknownProperty;

        PropertyMap::const_iterator itProperty = InfoMap.find(propertyName);

        if (itProperty != InfoMap.end())
        {
            value = itProperty->second;
            ret = Error::Success;
        }

        return ret;
    }
    /**
     * @brief Get the All Properties object
     * @return PropertyMap
     */
    virtual PropertyMap GetAllProperties() const
    {
        return InfoMap;
    }

private:
    PropertyMap InfoMap; ///< 属性数据表
};

NAMESPACE_END
#endif //