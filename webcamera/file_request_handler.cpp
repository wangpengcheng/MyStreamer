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
void FileRequestHandler::HandleHttpRequest(const TcpConnectionPtr &conn, const WebRequest& request,WebResponse&  response  )
{
    string req_path=request.path();
    std::string full_name=root_path_+req_path;
    std::string type="";
    if(FileExiting(full_name)){
        type=GetFileType(req_path);
        auto search=WebResponse::file_type.find(type);
        if(search!=WebResponse::file_type.end()){
            type=search->second;
        }else{
            type="";
        }
        if(!type.empty())
        {
            std::string file_string=ReadFile(full_name);
            if(!file_string.empty())
            {
                response.setStatusCode(WebResponse::k200Ok);
                response.setStatusMessage("OK");
                response.setContentType(type);
                response.setBody(file_string);
                response.addHeader("Content-Length",std::to_string(file_string.size()));
                return ;
            }else{
                std::cout<<"read file:"<<full_name<<"error"<<std::endl;
            }

        }
    }
    response.SendFast(WebResponse::k404NotFound," :(  Not Found File:"+req_path+" .");
}
