
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
// syscall(syscall_id, params_va_list);
char shorter[5];
char longer[100];
int main()
{
    long ret;
    printf("When length is enough:\n");
    ret = syscall(548, longer, 100);
    if (ret == 0)
        printf("System call succeeded!\n%s\n", longer);
    else
        printf("Errno %ld: Buffer Overflow!\n", ret);

    printf("When length is not enough:\n");
    ret = syscall(548, shorter, 5);
    if (ret == 0)
        printf("System call succeeded!\n%s\n", shorter);
    else
        printf("Errno %ld: Buffer Overflow!\n", ret);

    while (1)
        ;
    return 0;
}