#include "net_buffer.h"
#include "net_http_context.h"

using namespace MY_NAME_SPACE;
using namespace MY_NAME_SPACE::net;
/* 按照行来对参数进行读取 主要进行参数解析*/
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space))
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      /* 查找参数 */
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request_.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

/* 检查请求，结构体是否发生错误 */
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{

	//	std::string buffer(buf->peek(),buf->readableBytes());
	//std::cout<<"=======buffer:"<<buffer<<std::endl;	
	bool ok = true;
	bool hasMore = true;
	while (hasMore)
	{
		if (state_ == kExpectRequestLine)
		{
			const char* crlf = buf->findCRLF();
			if (crlf)
			{
				ok = processRequestLine(buf->peek(), crlf);
				if (ok)
				{
					request_.setReceiveTime(receiveTime);
					buf->retrieveUntil(crlf + 2);
					state_ = kExpectHeaders;
				}else{
					hasMore = false;
				}
			}else{
				hasMore = false;
			}
		}else if (state_ == kExpectHeaders) {
			const char* crlf = buf->findCRLF();
			if (crlf)
			{
				const char* colon = std::find(buf->peek(), crlf, ':');
				if (colon != crlf)
				{
					request_.addHeader(buf->peek(), colon, crlf);
				}else{
					// empty line, end of header
					// FIXME:
					state_ = kGotAll;
					hasMore = false;
				}
				buf->retrieveUntil(crlf + 2);
			}else{
				hasMore = false;
			}
		}else if (state_ == kExpectBody){
			// FIXME:
			const char* end_body=buf->findCRLF();
			std::string body(buf->peek(),end_body);
			std::cout<<"======="<<body<<std::endl;
		}
	}
	       std::string buffer(buf->peek(),buf->readableBytes());
        std::cout<<"=======buffer:"<<buffer<<std::endl;

	return ok;
}
