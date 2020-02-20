/*
    对象类的基本信息，用来模拟基本的类对象信息
*/
#ifndef BASE_OBJ_INFORMATION_H
#define BASE_OBJ_INFORMATION_H

#include <string>
#include <map>
#include "base_error.h"

NAMESPACE_START

typedef std::map<std::string, std::string> PropertyMap;


class BaseObjectInformation
{
public:
    virtual ~BaseObjectInformation( ) { }

    virtual Error GetProperty( const std::string& propertyName, std::string& value ) const = 0;

    virtual PropertyMap GetAllProperties( ) const = 0;
};

class XObjectInformationMap : public BaseObjectInformation
{
public:
    XObjectInformationMap( const PropertyMap& infoMap ) : InfoMap( infoMap ) { }

    virtual Error GetProperty( const std::string& propertyName, std::string& value ) const
    {
        Error ret = Error::UnknownProperty;

        PropertyMap::const_iterator itProperty = InfoMap.find( propertyName );

        if ( itProperty != InfoMap.end( ) )
        {
            value = itProperty->second;
            ret   = Error::Success;
        }

        return ret;
    }

    virtual PropertyMap GetAllProperties( ) const
    {
        return InfoMap;
    }

private:
    PropertyMap InfoMap;
};


NAMESPACE_END
#endif //