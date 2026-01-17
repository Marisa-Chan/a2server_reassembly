// testAsmInit.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

extern "C" {
    volatile int32_t testvar = 5;
};

class TestClass
{
public:
    int TestField();
};

int main(int argc, const char *argv[])
{
    TestClass a;
    printf("%x\n", testvar);
    printf("%x\n", a.TestField());
    return 0;
}
