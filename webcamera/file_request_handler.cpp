#include "file_request_handler.h"
#include "base_tool.h"

using namespace MY_NAME_SPACE;

FileRequestHandler::FileRequestHandler(const std::string& new_path_):
                    root_path_(new_path_)
{

};
void FileRequestHandler::HandleHttpRequest( const WebRequest& request,WebResponse&  response  )
{
    string req_path=request.path();
    std::string full_name=root_path_+req_path;
    if(FileExiting(full_name)){

    }else{
        response.SendFast(WebResponse::k404NotFound,"Not Found this file");
    }
}