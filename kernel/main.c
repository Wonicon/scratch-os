int main(void)
{
    char *vmem = (char *)0xa0000;
    for (;;) *vmem++ = 0xff;
    return 0;
}
