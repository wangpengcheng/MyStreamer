# 相关知识点笔记

## 1. ring buffer
_参考连接：_

- [Linux kfifo 源码分析](https://blog.csdn.net/Bruno_Mars/article/details/100061793#_kfifo__418)
- [巧夺天工的kfifo(修订版）](https://blog.csdn.net/linyt/article/details/53355355)
- [有没有多读多写的无锁队列](https://bbs.csdn.net/topics/370178192?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)
- [单生产者/单消费者 的 FIFO 无锁队列](https://www.cnblogs.com/VxGaaagaa/p/11110492.html)
- [单生产者/单消费者 的 FIFO 无锁队列](https://blog.csdn.net/weixin_30783913/article/details/97859864)
- [开发笔记(21) : 无锁消息队列](https://blog.codingnow.com/2012/06/dev_note_21.html)
- [C++ 环形缓冲区的实现](https://www.cnblogs.com/tangxin-blog/p/6131952.html)
- [一个生产者一个消费者的无锁队列，多个生产者多个消费者的无锁队列](https://blog.csdn.net/qq51931373/article/details/37693347?depth_1-utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)
- [Lock-Free Single-Producer - Single Consumer Circular Queue](https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular#_articleTop)
- [读Linux内核(4.9.9)之环形缓冲区实现kfifo](https://blog.csdn.net/idwtwt/article/details/79442046?depth_1.utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)
- [线程安全的环形缓冲区实现](https://blog.csdn.net/lezhiyong/article/details/7879558?depth_1.utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)
- [并发无锁环形队列的实现](https://www.linuxidc.com/Linux/2016-12/137937.htm)
- [C++ 无锁环形缓冲区实现](https://blog.csdn.net/lifexx/article/details/51764141)
## 2. 数据扩展
_参考连接：_

- [C 语言变长数组 struct 中 char data[0] 的用法](https://blog.csdn.net/xy010902100449/article/details/46522533);

```c++
#include <iostream>
#include <memory>
#include <string>
#include <cstdint>
#include <cstring>
using namespace std;

struct MyData 
{
int nLen;
char data[0];
};
 
int main()
{
    int nLen = 10;
    char str[10] = "123456789";
    
    cout << "Size of MyData: " <<sizeof(MyData) << endl;
    
    MyData *myData = (MyData*)malloc(sizeof(MyData) +10);
    std::cout<<sizeof(myData)<<std::endl;
    memcpy(myData->data, str, 10);
    
    cout << "myData's Data is: " << myData->data << endl;
    
    free(myData);
    
    return 0;
}
/*
输出：
Size of MyData: 4
8
myData's Data is: 123456789

*/
```

## 3. 读写锁

- [一个写优先的读写锁实现](http://blog.chinaunix.net/uid-14028890-id-1988527.html)
- [无锁编程:c++11基于atomic实现共享读写锁(写优先)](https://blog.csdn.net/10km/article/details/49641691?depth_1.utm_source=distribute.pc_relevant.none-task&utm_source=distribute.pc_relevant.none-task)

## 4 无锁编程RCU

- [Linux RCU 机制详解](https://www.cnblogs.com/linhaostudy/p/8463529.html)