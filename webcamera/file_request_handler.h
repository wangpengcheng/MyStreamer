#ifndef FILE_REQUEST_HANDLER_H
#define FILE_REQUEST_HANDLER_H
#include "web_request_handler.h"

NAMESPACE_START

class FileRequestHandler:public WebRequestHandlerInterface
{
public:
    FileRequestHandler(const std::string &new_path_);

    void HandleHttpRequest( const WebRequest& request,WebResponse&  response );
private:
    std::string root_path_;
};

NAMESPACE_END

#endif