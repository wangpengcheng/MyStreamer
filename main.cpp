#include <iostream>
#include "base_error.h"
using namespace std;

int main(int argc,char* argv[])
{
    MY_NAME_SPACE::BaseError my_error;

    std::cout<<"hello word"<<my_error.ToString()<<std::endl;
    return 0;
}