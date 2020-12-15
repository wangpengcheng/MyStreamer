/**
 * @file base_obj_configurator.h
 * @brief 基础配置对象类
 * @details 基础配置对象类，主要是实现BaseObjectInformation \n
 * 接口的属性设置功能 @see ::BaseObjectInformation
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 19:10:43
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
 *    <td>2020-12-15 19:10:43 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng </td>
 *    <td>内容 </td>
 * </tr>
 * </table>
 */
#ifndef IOBJECT_CONFIGURATOR_H
#define IOBJECT_CONFIGURATOR_H

#include <string>
#include <map>
#include "base_obj_information.h"
NAMESPACE_START
/**
 * @brief 对象构造类，主要是实现BaseObjectInformation \n
 * 接口的属性设置功能 @see ::BaseObjectInformation
 */
class BaseObjectConfigurator : public BaseObjectInformation
{
public:
    /**
     * @brief  设置属性设置方法
     * @param  propertyName     属性名称
     * @param  value            属性值
     * @return Error            方法执行是否正确 @link ::BaseError 
     */
    virtual Error SetProperty( const std::string& propertyName, const std::string& value ) = 0;
};
/**
 * @brief 基础配置类的别名
 */
typedef BaseObjectConfigurator  ObjectConfigurator;

NAMESPACE_END

#endif // IOBJECT_CONFIGURATOR_HPP