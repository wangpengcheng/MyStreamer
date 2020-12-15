# MyStreamer

基于NVIDIA TX2的低延时网络视频流传输系统；实现V4L2标准摄像头的图像采集处理和网络mjpeg发送。实现了简单的http服务器；实现了无锁环形缓冲区，作为图像处理缓冲区。短小精悍。目录结构如下：

- `base`:基础的处理函数集合，包括字符串处理，文件处理等
- `net`:网络服务器，实现了TCP和http服务，暂时只支持参数查询。
- `imgproc`:图像处理模块
- `camera`:摄像头驱动接口相关集合。
- `web`:httpserver中的网页页面和相关资源
- `webcamera`:网络摄像头模块，实现浏览器实时视频流



## TODO

- [ ] 摄像头属性网络查询和修改接口
- [ ] UDP server和组播,支持RTSP
- [ ] gstreamer摄像头驱动支持
- [ ] imgproc图像处理模块完善
- [ ] 异构设备GPU，FPGA加速支持
- [ ] 深度学习目标检测模型加速


## Install And Test
```shell
# git clone 
git clone https://github.com/wangpengcheng/MyStreamer.git
cd MyStreamer

# build 
mkdir build
cd build && cmake .. -DCMAKE_BUILD_TYPE=Release


# run and test

#test Mystreamer 
./install/bin/Release/MyStreamer

# test_http_sever
./install/bin/Release/http_server_test

# make docs
./config/build_doc.sh

``` 
