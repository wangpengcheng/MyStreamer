# API 使用说明

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
```

## 注意事项

图像传输请求是请求单张图片，因此需要客户端自定义刷新频率和方法。可以通过自定义刷新频率的方式来进行视频显示的更新

## 示例代码

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
        
        // always try capturing in MJPEG mode
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