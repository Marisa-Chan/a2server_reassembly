// testAsmInit.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

extern "C" {
    volatile int32_t testvar = 5;
};

int main(int argc, const char *argv[])
{
    printf("%x\n", testvar);
    return 0;
}
