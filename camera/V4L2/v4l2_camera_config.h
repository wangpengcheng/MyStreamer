/**
 * @file v4l2_camera_config.h
 * @brief 相机参数设置相关配置类
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-17 22:16:34
 * @copyright Copyright (c) 2020  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th> Commit date</th>
 *    <th> Version </th> 
 *    <th> Author </th>  
 *    <th> Description </th>
 * <tr>
 *    <td> 2020-12-17 22:16:34 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td>内容</td>
 * </tr>
 * </table>
 */
#ifndef V4L2_CAMERA_CONFIG_H
#define V4L2_CAMERA_CONFIG_H
#include "base_obj_configurator.h"
#include "v4l2_camera.h"

NAMESPACE_START

/**
 * @brief 相机基本配置属性类
 */
class V4L2CameraConfig : public BaseObjectConfigurator
{
public:
    /**
     * @brief Construct a new V4L2CameraConfig object
     * @param  camera           创建的相机对象
     */
    V4L2CameraConfig( const std::shared_ptr<V4L2Camera>& camera );
    /**
     * @brief Set the Property object
     * @param  propertyName     属性名称
     * @param  value            属性的值
     * @return Error            错误信息
     */
    Error SetProperty( const std::string& propertyName, const std::string& value );
    /**
     * @brief Get the Property object
     * @param  propertyName     属性名称
     * @param  value            属性的值
     * @return Error            错误信息
     */
    Error GetProperty( const std::string& propertyName, std::string& value ) const;
    /**
     * @brief 获取所有的属性值
     * @return std::map<std::string, std::string> 属性值
     */
    std::map<std::string, std::string> GetAllProperties( ) const;

private:
    std::shared_ptr<V4L2Camera> mCamera;        ///<  相机基本元素数据
};

/**
 * @brief 相机信息查询对象
 */
class V4L2CameraPropsInfo : public BaseObjectInformation
{
public:
    /**
     * @brief Construct a new V4L2CameraPropsInfo object
     * @param  camera           相机对象指针
     */
    V4L2CameraPropsInfo( const std::shared_ptr<V4L2Camera>& camera );
    /**
     * @brief Get the Property object
     * @param  propertyName     属性名称
     * @param  value            属性值
     * @return Error            错误信息
     */
    Error GetProperty( const std::string& propertyName, std::string& value ) const;
    /**
     * @brief Get the All Properties object
     * @return std::map<std::string, std::string> 所有属性结果 
     */
    std::map<std::string, std::string> GetAllProperties( ) const;

private:
    std::shared_ptr<V4L2Camera> mCamera;        ///< 相机数据指针
};

NAMESPACE_END

#endif