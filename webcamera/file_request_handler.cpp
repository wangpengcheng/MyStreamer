#include "file_request_handler.h"

using namespace MY_NAME_SPACE;

FileRequestHandler::FileRequestHandler()
{

}
FileRequestHandler::FileRequestHandler(const std::string& new_path_):
                    //WebRequestHandlerInterface(" ",false),
                    root_path_(new_path_)
{

};
void FileRequestHandler::HandleHttpRequest( const WebRequest& request,WebResponse&  response  )
{
    string req_path=request.path();
    std::string full_name=root_path_+req_path;
    if(FileExiting(full_name)){
        std::string type=GetFileType(req_path);
        auto search=WebResponse::file_type.find(type);
        if(search!=WebResponse::file_type.end()){
            type=search->second;
        }else{
            type="";
        }
        if(!type.empty())
        {
            std::string file_string=ReadFile(full_name);
            response.setStatusCode(WebResponse::k200Ok);
            response.setStatusMessage("OK");
            response.setContentType(type);
            response.setBody(file_string);
            return ;
        }
    }
    std::cout<<"====================="<<std::endl;
    std::cout<<"file:"<<full_name<<"error"<<std::endl;
    response.SendFast(WebResponse::k404NotFound,"Not Found this file");
}