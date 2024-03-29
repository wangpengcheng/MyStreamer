#include "base_json_parser.h"

using namespace std;

/* 声明函数 */
/**
 * @brief  
 * @param  ptr              My Param doc
 * @param  str              My Param doc
 * @return true             解析成功 
 * @return false            解析失败
 */
static bool ExtractString(const char **ptr, string &str);
static bool ExtractValue(const char **ptr, string &str, bool addQuotesToStrings);
static bool ExtractArrayAsString(const char **ptr, string &str);
/* 解析对象字符串 */
static bool ExtractObjectAsString(const char **ptr, string &str);
/* 跳过空格的快速方法 */

#define SKIP_SPACES(strptr)                       \
    while ((*strptr != '\0') && (*strptr == ' ')) \
    {                                             \
        strptr++;                                 \
    }

bool SimpleJsonParser(const string &jsonStr, map<string, string> &values)
{
    /* 获取字符串指针 */
    const char *ptr = jsonStr.c_str();
    /* 定义键值属性对 */
    string strName, strValue;

    values.clear();
    /* 跳过开头的空格 */
    SKIP_SPACES(ptr);
    //检查开头
    if (*ptr != '{')
    {
        return false;
    }
    /* 开始检查字符串内容 */
    for (;;)
    {
        ptr++;
        /* 获取第一个字符值 */
        if (!ExtractString(&ptr, strName))
        {
            break;
        }
        /* 检查中间是否为： */
        if (*ptr != ':')
        {
            return false;
        }

        ptr++;
        /* 获取值 */
        if (!ExtractValue(&ptr, strValue, false))
        {
            return false;
        }
        /* 将属性和值进行添加 */
        values.insert(pair<string, string>(strName, strValue));
        /* 如果遇到};直接结束*/
        if (*ptr == '}')
        {
            // all done
            break;
        }
        /* 检查是否有下一个属性值，有则继续，没有则返回false */
        if (*ptr != ',')
        {
            // broken JSON
            return false;
        }
    }
    /* 检查尾部 */
    if (*ptr != '}')
    {
        return false;
    }

    return true;
}

/* 获取连续的字符串，直到遇到 */
bool ExtractString(const char **ptr, string &str)
{
    const char *p = *ptr;

    SKIP_SPACES(p);

    if (*p != '"')
    {
        return false;
    }
    else
    {
        string s;

        p++;
        /* 检查字符串尾部 */
        while ((*p != '\0') && (*p != '"'))
        {
            /* 检查特殊字符 */
            if (*p != '\\')
            {
                s.push_back(*p);
                p++;
            }
            else
            {
                p++;
                if (*p == '\0')
                {
                    return false;
                }
                else
                {
                    char esc = *p;

                    p++;

                    switch (esc)
                    {
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    case '/':
                        s.push_back('/');
                        break;
                    case 'b':
                        s.push_back('\b');
                        break;
                    case 'f':
                        s.push_back('\f');
                        break;
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'u':
                        // unicode is not supported - just skip 4 hex digits
                        for (int i = 0; i < 4; i++)
                        {
                            if (*p == '\0')
                            {
                                return false;
                            }
                            p++;
                        }
                        break;
                    default:
                        return false;
                    }
                }
            }
        }

        if (*p != '"')
        {
            return false;
        }

        // move to the next character after the string and spaces
        p++;
        SKIP_SPACES(p);

        *ptr = p;
        str = s;
    }

    return true;
}

// 对json值的解析
bool ExtractValue(const char **ptr, string &str, bool addQuotesToStrings)
{
    const char *p = *ptr;
    string s;

    SKIP_SPACES(p);

    switch (*p)
    {
    case '"': // string value
        if (!ExtractString(&p, s))
        {
            return false;
        }
        if (addQuotesToStrings)
        {
            s.insert(0, 1, '"');
            s.push_back('"');
        }
        break;

    case '{': // 如果检测到{;表示是一个对象，进行对象值的解析
        if (!ExtractObjectAsString(&p, s))
        {
            return false;
        }
        break;

    case '[': // 对数组进行解析
        if (!ExtractArrayAsString(&p, s))
        {
            return false;
        }
        break;
        /* 默认情况下的解析 */
    default:
        if ((*p == 't') || (*p == 'f') || (*p == 'n') || (*p == '-') || ((*p >= '0') && (*p <= '9')))
        {
            // number or true/false/null  - extract everything up to the next space or ,}] - no checking for valid numbers
            while ((*p != '\0'))
            {
                if ((*p == ' ') || (*p == ',') || (*p == '}') || (*p == ']'))
                {
                    break;
                }
                s.push_back(*p);
                p++;
            }
            /* 检查是否为t,f,n */
            if (((s[0] == 't') && (s != "true")) ||
                ((s[0] == 'f') && (s != "false")) ||
                ((s[0] == 'n') && (s != "null")))
            {
                return false;
            }

            SKIP_SPACES(p);
        }
        else
        {
            // broken value
            return false;
        }
    }

    *ptr = p;
    str = s;

    return true;
}

// Extract JSPN array as string；解析数组
static bool ExtractArrayAsString(const char **ptr, string &str)
{
    const char *p = *ptr;

    SKIP_SPACES(p);

    if (*p != '[')
    {
        return false;
    }
    else
    {
        string s, strValue;

        for (;;)
        {
            p++;

            if (!ExtractValue(&p, strValue, true))
            {
                break;
            }
            //注意这里使用，进行分割
            if (!s.empty())
            {
                s.append(",");
            }
            s.append(strValue);

            if (*p == ']')
            {
                break;
            }

            if (*p != ',')
            {
                return false;
            }
        }

        if (*p != ']')
        {
            return false;
        }
        //前后添加新值
        s.insert(0, 1, '[');
        s.push_back(']');

        p++; // move to the next character after the array and spaces
        SKIP_SPACES(p);

        *ptr = p;
        str = s;
    }

    return true;
}

// Extract JSON object as string
static bool ExtractObjectAsString(const char **ptr, string &str)
{
    const char *p = *ptr;

    SKIP_SPACES(p);

    if (*p != '{')
    {
        return false;
    }
    else
    {
        string s, strName, strValue;

        for (;;)
        {
            p++;
            //检查属性
            if (!ExtractString(&p, strName))
            {
                break;
            }

            if (*p != ':')
            {
                return false;
            }

            p++;
            //检查，注意这里因为是对象，因此，需要在首位添加""作为序列化区分
            if (!ExtractValue(&p, strValue, true))
            {
                return false;
            }
            //
            if (!s.empty())
            {
                s.append(",");
            }
            //重新整合
            s.append(strName);
            s.push_back(':');
            s.append(strValue);

            if (*p == '}')
            {
                break;
            }

            if (*p != ',')
            {
                return false;
            }
        }

        if (*p != '}')
        {
            return false;
        }
        /* 重新将对象值添加{} */
        s.insert(0, 1, '{');
        s.push_back('}');

        // move to the next character after the object and spaces
        p++;
        SKIP_SPACES(p);

        *ptr = p;
        str = s;
    }

    return true;
}