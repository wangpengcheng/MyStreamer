/**
 * @file base_obj_configuration_serializer.h
 * @brief 配置序列化文件类，用来解析配置文件
 * @details 配置文件序列化类，主要用来定义配置文件的序列化接口和解析相关属性类 \n
 * 但是因为时间原因，没有对主要的服务器参数进行配置和修改
 * @todo 将 main.cpp 中的相关文件设置为对应的构造函数类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 16:23:14
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
 *    <td>2020-12-15 16:23:14 </td>
 *    <td>1.0 </td>
 *    <td>wangpengcheng    </td>
 *    <td>添加注释文档</td>
 * </tr>
 * </table>
 */
/**
 *
 */
#ifndef BASE_OBJ_CONFIGURATION_SERIALIZER_H
#define BASE_OBJ_CONFIGURATION_SERIALIZER_H

#include <memory>
#include "base_obj_configurator.h"

NAMESPACE_START
/**
 * @brief 配置文件读取抽象类
 */
class ObjectConfigurationSerializer
{
public:
    /**
     * @brief Construct a new Object Configuration Serializer object
     */
    ObjectConfigurationSerializer();
    /**
     * @brief Construct a new Object Configuration Serializer object
     * @param  fileName         配置文件名称
     * @param  objectToConfigureMy 需要映射的配置文件数据指针 @see ::BaseObjectConfigurator
     */
    ObjectConfigurationSerializer(const std::string &fileName,
                                  const std::shared_ptr<BaseObjectConfigurator> &objectToConfigure);
    /**
     * @brief  配置文件数据存储错误
     * @return Error 错误类型; @see ::BaseError
     */
    Error SaveConfiguration() const;
    /**
     * @brief 配置数据加载错误
     * @return Error 错误类型； @see ::BaseError
     */
    Error LoadConfiguration() const;

private:
    std::string FileName;                                      ///< 文件名称,建议使用相对文件路径+名称
    std::shared_ptr<BaseObjectConfigurator> ObjectToConfigure; ///< 映射的配置文件数据指针，使用智能指针管理内存
};

NAMESPACE_END
#endif // BASE_OBJ_CONFIGURATION_SERIALIZER_H