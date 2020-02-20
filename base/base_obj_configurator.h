#ifndef IOBJECT_CONFIGURATOR_H
#define IOBJECT_CONFIGURATOR_H

#include <string>
#include <map>
#include "base_obj_information.h"
NAMESPACE_START

class BaseObjectConfigurator : public BaseObjectInformation
{
public:
    virtual Error SetProperty( const std::string& propertyName, const std::string& value ) = 0;
};

typedef BaseObjectConfigurator  ObjectConfigurator;

NAMESPACE_END

#endif // IOBJECT_CONFIGURATOR_HPP