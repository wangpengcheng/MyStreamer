#ifndef FILE_REQUEST_HANDLER_H
#define FILE_REQUEST_HANDLER_H
#include "web_request_handler.h"

NAMESPACE_START

class FileRequestHandler//:public WebRequestHandlerInterface
{
public:
    explicit FileRequestHandler();
    FileRequestHandler(const std::string &new_path_);
    inline void setRootPath(const std::string & new_path){root_path_=new_path;} 
    void HandleHttpRequest( const WebRequest& request,WebResponse&  response );
private:
    std::string root_path_;
};

NAMESPACE_END

#endif