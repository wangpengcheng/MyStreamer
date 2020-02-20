

#include "base_obj_configuration_serializer.h"

#ifdef WIN32
    #include <windows.h>
#endif

NAMESPACE_START

ObjectConfigurationSerializer::ObjectConfigurationSerializer( ) :
    FileName( ),
    ObjectToConfigure( )
{

}

ObjectConfigurationSerializer::ObjectConfigurationSerializer( const std::string& fileName,
                                                                const std::shared_ptr<BaseObjectConfigurator>& objectToConfigure ) :
    FileName( fileName ),
    ObjectToConfigure( objectToConfigure )
{
}

// 将属性存储到文件中
Error ObjectConfigurationSerializer::SaveConfiguration( ) const
{
    Error ret  = Error::Success;

    if ( ( FileName.empty( ) ) || ( !ObjectToConfigure ) )
    {
        ret = Error::Failed;
    }
    else
    {
        FILE* file = nullptr;

#ifdef WIN32
        {
            int charsRequired = MultiByteToWideChar( CP_UTF8, 0, FileName.c_str( ), -1, NULL, 0 );

            if ( charsRequired > 0 )
            {
                WCHAR* filenameUtf16 = (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired );

                if ( MultiByteToWideChar( CP_UTF8, 0, FileName.c_str( ), -1, filenameUtf16, charsRequired ) > 0 )
                {
                    file = _wfopen( filenameUtf16, L"w" );
                }

                free( filenameUtf16 );
            }
        }
#else
        file = fopen( FileName.c_str( ), "w" );
#endif

        if ( file == nullptr )
        {
            ret = Error::IOError;
        }
        else
        {
            std::map<std::string, std::string> properties = ObjectToConfigure->GetAllProperties( );
            bool                first = true;

            // write a simple file, where property name and value go separate lines
            for ( auto property : properties )
            {
                if ( !first )
                {
                    fprintf( file, "\n" );
                }

                fprintf( file, "%s\n%s\n", property.first.c_str( ), property.second.c_str( ) );

                first = false;
            }

            fclose( file );
        }
    }

    return ret;
}

// 从配置文件中加载属性
Error ObjectConfigurationSerializer::LoadConfiguration( ) const
{
    Error ret  = Error::Success;

    if ( ( FileName.empty( ) ) || ( !ObjectToConfigure ) )
    {
        ret = Error::Failed;
    }
    else
    {
        FILE* file = nullptr;
        
#ifdef WIN32
        {
            int charsRequired = MultiByteToWideChar( CP_UTF8, 0, FileName.c_str( ), -1, NULL, 0 );

            if ( charsRequired > 0 )
            {
                WCHAR* filenameUtf16 = (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired );

                if ( MultiByteToWideChar( CP_UTF8, 0, FileName.c_str( ), -1, filenameUtf16, charsRequired ) > 0 )
                {
                    file = _wfopen( filenameUtf16, L"r" );
                }

                free( filenameUtf16 );
            }
        }
#else        
        file = fopen( FileName.c_str( ), "r" );
#endif

        if ( file == nullptr )
        {
            ret = Error::IOError;
        }
        else
        {
            char   buffer[256];
            std::string name;
            std::string line;
            bool   gotName = false;

            while ( fgets( buffer, sizeof( buffer ) - 1, file ) )
            {
                line = std::string( buffer );
                //获取
                while ( ( !line.empty( ) ) &&
                        ( ( line.back( ) == ' ' )  || ( line.back( ) == '\t' ) ||
                          ( line.back( ) == '\n' ) || ( line.back( ) == '\r' ) ) )
                {
                    line.pop_back( );
                }

                // allow blank lines between configuration options, but not between option and its value
                if ( ( !line.empty( ) ) || ( gotName ) )
                {
                    if ( !gotName )
                    {
                        name    = line;
                        gotName = true;
                    }
                    else
                    {
                        ObjectToConfigure->SetProperty( name, line );
                        gotName = false;
                    }
                }
            }
        }
    }

    return ret;
}

NAMESPACE_END