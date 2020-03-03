#include "web_camera_control_handler.h"
#include "logging.h"
#include "base_str_tools.h"
#include "base_json_parser.h"

using namespace MY_NAME_SPACE;


const static char* StatusOK                   = "OK";
const static char* StatusInvalidJson          = "Invalid JSON object";
const static char* StatusUnknownProperty      = "Unknown property";
const static char* StatusInvalidPropertyValue = "Invalid property value";
const static char* StatusPropertyFailed       = "Failed setting property";
/* 内部函数 */
// Get all or the list of specified variables

static void HandleGetRequest( const std::shared_ptr<V4L2CameraConfig>& infoObject, const std::string& varsToGet, WebResponse& response )
{
    std::map<string, string> values;
    string              reply = "{\"status\":\"OK\",\"config\":{";
    bool                first = true;
    /* 获取全部属性 */
    if ( varsToGet.empty( ) )
    {
        // get all properties of the object
        values = infoObject->GetAllProperties( );
    }
    else
    {
        // get only specified properties (comma separated list)
        int start = 0;

        while ( varsToGet[start] != '\0' )
        {
            int end = start;

            while ( ( varsToGet[end] != '\0' ) && ( varsToGet[end] != ',' ) )
            {
                end++;
            }

            int count = end - start;

            if ( count != 0 )
            {
                string varName = varsToGet.substr( start, count );
                string varValue;

                if ( infoObject->GetProperty( varName, varValue ) == Error::Success )
                {
                    values.insert( std::pair<std::string, std::string>( varName, varValue ) );
                }
            }

            start = end;
            if ( varsToGet[start] == ',' )
            {
                start++;
            }
        }
    }

    // form a JSON response with values of the properties
    for ( auto kvp : values )
    {
        std::map<std::string, std::string> innerValue;

        if ( !first )
        {
            reply += ",";
        }

        reply += "\"";
        reply += kvp.first;
        reply += "\":";
        
        // a dirty hack for providing already serialized JSON
        if ( ( kvp.second.length( ) >= 2 ) && ( kvp.second.front( ) == '{' ) && ( kvp.second.back( ) == '}' ) &&
             ( SimpleJsonParser( kvp.second, innerValue ) ) )
        {
            reply += kvp.second;
        }
        else
        {
            reply += "\"";
            reply += StringReplace( kvp.second, "\"", "\\\"" );
            reply += "\"";
        }

        first = false;
    }

    reply += "}}";
    response.setStatusCode(WebResponse::k200Ok);
    response.setStatusMessage("OK");
    response.setContentType("application/json");
    response.addHeader("Cache-Control","no-store, must-revalidate");
    response.addHeader("Pragma","no-cache");
    response.addHeader("Expires","0");
    response.setBody(reply);

}

// Set all variables specified in the posted JSON
void HandlePostRequest( const std::shared_ptr<V4L2CameraConfig>& objectToConfig, const std::string& body, WebResponse& response )
{
    std::string status="OK";
    std::map<std::string, std::string> values;
    string              reply = "{\"status\":\"";
    string              failedProperty;

    if ( !SimpleJsonParser( body, values ) )
    {
        status = std::string(StatusInvalidJson);
    }
    else
    {
        for ( auto kvp : values )
        {
            Error ecode = objectToConfig->SetProperty( kvp.first, kvp.second );

            if ( ecode != Error::Success )
            {
                failedProperty = kvp.first;
                switch ( ecode.Code( ) )
                {
                case Error::UnknownProperty:
                    status = StatusUnknownProperty;
                    break;
                case Error::InvalidPropertyValue:
                    status = StatusInvalidPropertyValue;
                    break;
                default:
                    status = StatusPropertyFailed;
                    break;
                }
            }
        }
    }

    reply += status;
    if ( !failedProperty.empty( ) )
    {
        reply += "\",\"property\":\"";
        reply += failedProperty;
    }
    reply += "\"}";
    response.setStatusCode(WebResponse::k200Ok);
    response.setStatusMessage("OK");
    response.setContentType("application/json");
    response.addHeader("Cache-Control","no-store, must-revalidate");
    response.addHeader("Pragma","no-cache");
    response.addHeader("Expires","0");
    response.setBody(reply);
}


CameraInfoHandler::CameraInfoHandler(
        const std::shared_ptr<V4L2Camera>& camera,
        const std::string& url):
        WebRequestHandlerInterface( url, false )
{
    camera_data_=std::make_shared<V4L2CameraConfig>(camera);
}

void CameraInfoHandler::HandleHttpRequest(const WebRequest& request,WebResponse&  response )
{
    if(request.method()==WebRequest::kGet){
        /* 检查数据指针是否正常 */
        if(camera_data_)
        {
           HandleGetRequest( this->camera_data_, ""/* request.GetVariable( "vars" ) */, response );
            
        }else{
            LOG_WARN<<"Camrea not exit Please Check agin";
            response.SendFast(WebResponse::k404NotFound,"Don't have this Camera info ");
            response.setCloseConnection(true);
        }
    }else{
        response.SendFast(WebResponse::k405MethodNotAllowed,"Method Not Allowed Please Using Get ");
        response.setCloseConnection(true);
    }
    
};


/*camers control handler */
CameraControlHandler::CameraControlHandler(
        const std::shared_ptr<V4L2Camera>& camera,
        const std::string& url):
        WebRequestHandlerInterface( url, false )
{
    /* 初始化只能指针 */
    camera_data_=std::make_shared<V4L2CameraConfig>(camera);
}

void CameraControlHandler::HandleHttpRequest(const WebRequest& request,WebResponse&  response )
{
    
}