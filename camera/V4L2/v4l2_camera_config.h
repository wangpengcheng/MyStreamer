#ifndef V4L2_CAMERA_CONFIG_H
#define V4L2_CAMERA_CONFIG_H
#include "base_obj_configurator.h"
#include "v4l2_camera.h"

NAMESPACE_START

// 获取设置基本属性
class V4L2CameraConfig : public BaseObjectConfigurator
{
public:
    V4L2CameraConfig( const std::shared_ptr<V4L2Camera>& camera );

    Error SetProperty( const std::string& propertyName, const std::string& value );
    Error GetProperty( const std::string& propertyName, std::string& value ) const;

    std::map<std::string, std::string> GetAllProperties( ) const;

private:
    std::shared_ptr<V4L2Camera> mCamera;
};

// 设置默认信息
class V4L2CameraPropsInfo : public BaseObjectInformation
{
public:
    V4L2CameraPropsInfo( const std::shared_ptr<V4L2Camera>& camera );

    Error GetProperty( const std::string& propertyName, std::string& value ) const;

    std::map<std::string, std::string> GetAllProperties( ) const;

private:
    std::shared_ptr<V4L2Camera> mCamera;
};

NAMESPACE_END

#endif