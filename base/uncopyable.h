/**
 * @file uncopyable.h
 * @brief 不可拷贝原始继承类和可拷贝基础类
 * @details 不可拷贝类，将其拷贝构造函数私有化，防止拷贝现象发生
 * @author wangpengcheng  (wangpengcheng2018@gmail.com)
 * @version 1.0
 * @date 2020-12-15 15:20:19
 * @copyright Copyright (c) 2020  IRLSCU
 * 
 * @par 修改日志:
 * <table>
 * <tr>
 *    <th>Commit date</th>
 *    <th>Version</th> 
 *    <th>Author</th>  
 *    <th>Description</th>
 * <tr>
 *    <td> 2020-12-15 15:20:19 </td>
 *    <td> 1.0 </td>
 *    <td> wangpengcheng </td>
 *    <td> 添加注释 </td>
 * </tr>
 * </table>
 */
#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H
#include "base_define.h"
NAMESPACE_START
/**
 * @brief 不可拷贝基础类，构造函数为protected 便于子类继承
 */
class Uncopyable
{
protected:
    Uncopyable() = default;
    ~Uncopyable() = default; ///< 将析构函数设置为protected，保证只有堆上才能创建内存

private:
    Uncopyable(const Uncopyable &) = delete;
    Uncopyable &operator=(const Uncopyable &) = delete;
};
typedef Uncopyable noncopyable;
/**
 * @brief 可拷贝类
 */
class copyable
{
protected:
    copyable() = default;
    ~copyable() = default;
};

NAMESPACE_END

#endif //UNCOPYABLE_H
