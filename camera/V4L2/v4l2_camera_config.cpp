
#include <map>
#include <list>
#include <algorithm>

#include "v4l2_camera_config.h"

NAMESPACE_START


using namespace std;

#define TYPE_INT  (0)
#define TYPE_BOOL (1)

typedef struct
{
    VideoProperty  mVideoProperty;
    uint16_t        Type;
    uint16_t        Order;
    const char*     Name;
}
PropertyInformation;
/* 相关属性参数 */
const static map<string, PropertyInformation> SupportedProperties =
{
    { "brightness",  { VideoProperty::Brightness,            TYPE_INT,   0, "Brightness"              } },
    { "contrast",    { VideoProperty::Contrast,              TYPE_INT,   1, "Contrast"                } },
    { "saturation",  { VideoProperty::Saturation,            TYPE_INT,   2, "Saturation"              } },
    { "hue",         { VideoProperty::Hue,                   TYPE_INT,   3, "Hue"                     } },
    { "sharpness",   { VideoProperty::Sharpness,             TYPE_INT,   4, "Sharpness"               } },
    { "gain",        { VideoProperty::Gain,                  TYPE_INT,   5, "Gain"                    } },
    { "blc",         { VideoProperty::BacklightCompensation, TYPE_INT,   6, "Back Light Compensation" } },
    { "redBalance",  { VideoProperty::RedBalance,            TYPE_INT,   7, "Red Balance"             } },
    { "blueBalance", { VideoProperty::BlueBalance,           TYPE_INT,   8, "Blue Balance"            } },
    { "awb",         { VideoProperty::AutoWhiteBalance,      TYPE_BOOL,  9, "Automatic White Balance" } },
    { "hflip",       { VideoProperty::HorizontalFlip,        TYPE_BOOL, 10, "Horizontal Flip"         } },
    { "vflip",       { VideoProperty::VerticalFlip,          TYPE_BOOL, 11, "Vertical Flip"           } }
};

// ------------------------------------------------------------------------------------------

V4L2CameraConfig::V4L2CameraConfig( const shared_ptr<V4L2Camera>& camera ) :
    mCamera( camera )
{

}

// Set the specified property of a DirectShow video device
Error V4L2CameraConfig::SetProperty( const string& propertyName, const string& value )
{
    Error  ret       = Error::Success;
    int32_t propValue = 0;

    // assume all configuration values are numeric
    int scannedCount = sscanf( value.c_str( ), "%d", &propValue );

    if ( scannedCount != 1 )
    {
        ret = Error::InvalidPropertyValue;
    }
    else
    {
        map<string, PropertyInformation>::const_iterator itSupportedProperty = SupportedProperties.find( propertyName );

        if ( itSupportedProperty == SupportedProperties.end( ) )
        {
            ret = Error::UnknownProperty;
        }
        else
        {
            ret = mCamera->SetVideoProperty( itSupportedProperty->second.mVideoProperty, propValue );
        }
    }

    return ret;
}

// Get the specified property of a DirectShow video device
Error V4L2CameraConfig::GetProperty( const string& propertyName, string& value ) const
{
    Error  ret       = Error::Success;
    int32_t propValue = 0;
    char    buffer[32];

    // find the property in the list of supported
    map<string, PropertyInformation>::const_iterator itSupportedProperty = SupportedProperties.find( propertyName );

    if ( itSupportedProperty == SupportedProperties.end( ) )
    {
        ret = Error::UnknownProperty;
    }
    else
    {
        // get the property value itself
        ret = mCamera->GetVideoProperty( itSupportedProperty->second.mVideoProperty, &propValue );
    }

    if ( ret )
    {
        sprintf( buffer, "%d", propValue );
        value = buffer;
    }

    return ret;
}

// Get all supported properties of a DirectShow video device
map<string, string> V4L2CameraConfig::GetAllProperties( ) const
{
    map<string, string> properties;
    string              value;

    for ( auto property : SupportedProperties )
    {
        if ( GetProperty( property.first, value ) )
        {
            properties.insert( pair<string, string>( property.first, value ) );
        }
    }

    return properties;
}

// ------------------------------------------------------------------------------------------

V4L2CameraPropsInfo::V4L2CameraPropsInfo( const shared_ptr<V4L2Camera>& camera ) :
    mCamera( camera )
{
}

Error V4L2CameraPropsInfo::GetProperty( const std::string& propertyName, std::string& value ) const
{
    Error  ret = Error::Success;
    char    buffer[128];

    // find the property in the list of supported
    map<string, PropertyInformation>::const_iterator itSupportedProperty = SupportedProperties.find( propertyName );

    if ( itSupportedProperty == SupportedProperties.end( ) )
    {
        ret = Error::UnknownProperty;
    }
    else
    {
        int32_t min, max, step, def;

        // get property features - min/max/default/etc
        ret = mCamera->GetVideoPropertyRange( itSupportedProperty->second.mVideoProperty, &min, &max, &step, &def );

        if ( ret )
        {
            if ( itSupportedProperty->second.Type == TYPE_INT )
            {
                sprintf( buffer, "{\"min\":%d,\"max\":%d,\"def\":%d,\"type\":\"int\",\"order\":%d,\"name\":\"%s\"}",
                         min, max, def, itSupportedProperty->second.Order, itSupportedProperty->second.Name );
            }
            else
            {
                sprintf( buffer, "{\"def\":%d,\"type\":\"bool\",\"order\":%d,\"name\":\"%s\"}",
                         def, itSupportedProperty->second.Order, itSupportedProperty->second.Name );
            }

            value = buffer;
        }
    }

    return ret;
}

// Get information for all supported properties of a DirectShow video device
map<string, string> V4L2CameraPropsInfo::GetAllProperties( ) const
{
    map<string, string> properties;
    string              value;

    for ( auto property : SupportedProperties )
    {
        if ( GetProperty( property.first, value ) )
        {
            properties.insert( pair<string, string>( property.first, value ) );
        }
    }

    return properties;
}

NAMESPACE_END