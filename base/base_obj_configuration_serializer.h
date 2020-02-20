/**
 * 配置序列化文件类，用来解析配置文件
*/
#ifndef BASE_OBJ_CONFIGURATION_SERIALIZER_H
#define BASE_OBJ_CONFIGURATION_SERIALIZER_H

#include <memory>
#include "base_obj_configurator.h"

NAMESPACE_START 

class ObjectConfigurationSerializer
{
public:
    ObjectConfigurationSerializer( );
    ObjectConfigurationSerializer( const std::string& fileName,
                                    const std::shared_ptr<BaseObjectConfigurator>& objectToConfigure );

    Error SaveConfiguration( ) const;
    Error LoadConfiguration( ) const;

private:
    std::string FileName;
    std::shared_ptr<BaseObjectConfigurator> ObjectToConfigure;
};

NAMESPACE_END
#endif //BASE_OBJ_CONFIGURATION_SERIALIZER_H