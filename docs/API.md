# API 使用说明
## 1.jpeg图片循环请求

### 1.1 主要请求数据
图像请求主要协议是HTTP协议
请求path:`/camera/jpeg`
请求数据HTTP头部示例:

```json
GET /camera/jpeg?t=1603202154390 HTTP/1.1
Host: 192.168.58.143:8000
Connection: keep-alive
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Accept: image/avif,image/webp,image/apng,image/*,*/*;q=0.8
Referer: http://192.168.58.143:8000/index.html
Accept-Encoding: gzip, deflate
Accept-Language: en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7
```
响应图片原始数据:

```json
HTTP/1.1 200 OK
Content-Length: 0
Connection: Keep-Alive
Cache-Control: no-store, must-revalidate
Content-Type: image/png
Timesmap: 1603207683013 /* 注意这里的自定时间戳 */
NodeName: car1 /* 自定义节点名称 */
Expires: 0
Pragma: no-cache
body:
....../* 返回jpeg压缩图像 */
```


### 1.2 注意事项

图像传输请求是请求单张图片，因此需要客户端自定义刷新频率和方法。可以通过自定义刷新频率的方式来进行视频显示的更新

## 2 mjpeg流支持
_参考链接:_
- [MJPEG百度百科](https://baike.baidu.com/item/MJPEG/8966488?fr=aladdin)
- [C#开源实现MJPEG流传输](https://www.cnblogs.com/gaochundong/p/csharp_mjpeg_streaming.html#what_is_mjpeg)
- [MJPEG协议入门介绍](http://blog.chinaunix.net/uid-22670933-id-1771591.html)
- [MJPEG原理](https://blog.csdn.net/u011426247/article/details/80547683)

### 2.1 请求和返回数据

```json
GET /camera/mjpeg HTTP/1.1
Host: localhost:8000
Connection: keep-alive
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/83.0.4103.61 Chrome/83.0.4103.61 Safari/537.36
Accept: image/webp,image/apng,image/*,*/*;q=0.8
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: no-cors
Sec-Fetch-Dest: image
Referer: http://localhost:8000/index.html
Accept-Encoding: gzip, deflate, br
Accept-Language: en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7
```
响应mjpeg数据:

```json
HTTP/1.1 200 OK
Cache-Control: no-store, must-revalidate
Connection: Keep-Alive
Content-Type: multipart/x-mixed-replace; boundary=--myboundary
Expires: 0
Pragma: no-cache

--myboundary
Content-Type: image/jpeg
Content-Length: 23545

/* ... jpeg图像数据 */

--myboundary
Content-Type: image/jpeg
Content-Length: 23545

/* ... jpeg图像数据 */

........
```
### 2.2 注意事项

- 响应mjpeg流是一个不断写入的流式文件，除非主动关闭，否则会一直接收流文件，注意浏览器缓存。
- 视频刷新频率主要由视频输入端决定，不能客户端控制。
- `boundary=--myboundary`中定义边界，由浏览器进行数据分割，但是注意每个自定义头部必须含有数据类型和长度大小，并以自定义分割符`boundary=--myboundary`为开头。自定义头部和数据体之间必须空行；例如:`\r\n--myboundary\r\nContent-Type: image/jpeg\r\nContent-Length: xxxx \r\n\r\n`
- 默认模式为;`/camera/jpeg`请求。

## 3 示例代码

JS端的示例代码如下:
```javascript
// 定义闭包内
var Camera = (function ()
{
    // 设置请求路径
    var jpegUrl       = '/camera/jpeg';
    var mjpegUrl      = '/camera/mjpeg';
    var mjpegMode;
    var frameInterval;
    var imageElement;
    var timeStart;
    // 刷新显示画布元素
    function refreshImage( )
    {
        timeStart = new Date( ).getTime( );
        // 更新图片对应请求链接
        imageElement.src = jpegUrl + '?t=' + timeStart;
    }
    
    function onImageError( )
    {
        // also give it a small pause on error
        setTimeout( refreshImage, 1000 );
    }

    function onImageLoaded( )
    {
        var timeTaken = new Date( ).getTime( ) - timeStart;
        setTimeout( refreshImage, ( timeTaken > frameInterval ) ? 0 : frameInterval - timeTaken );
    }
    
    var start = function( fps )
    {
        imageElement = document.getElementById( 'camera' );
        // 设置加载和回调函数
        imageElement.onload  = onImageLoaded;
        imageElement.onerror = onImageError;
        
        if ( ( typeof fps == 'number' ) &&  ( fps != 0 ) )
        {
            frameInterval = 1000 / fps;
        }
        else
        {
            frameInterval = 100;
        }
        
        // mjpeg模式优先
        mjpegMode = true;
        
        refreshImage( );
    };

    return {
        Start: start
    }
} )( );

// 使用
camera.start(20);

```

## 使用示例代码
[camera.js](https://github.com/wangpengcheng/MyStreamer/blob/master/web/camera.js)
