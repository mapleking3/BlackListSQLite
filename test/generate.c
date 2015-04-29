#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static const char *szBaseSting = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void generate(int cnt)
{
    char filename[32] = {0};
    sprintf(filename, "plate_%d.txt", cnt);

    int fd = open(filename, O_CREAT | O_RDWR, 0777);
    if (fd < 0)
    {
        fprintf(stderr, "Open File Error!\n");
        return;
    }

    int a, b, c, d, e, f;
    //char plateBuff[100] = {0};
    char plateBuff[64] = {0};
    printf("%p %p\n", filename, plateBuff);
    int stat = 0;

    for(a = 0; a < 26; ++a)
    {
        for (b = 0; b < 36; ++b)
        {
            for (c = 0; c < 36; ++c)
            {
                for (d = 0; d < 36; ++d)
                {
                    for (e = 0; e < 36; ++e)
                    {
                        for (f = 0; f < 36; ++f)
                        {
                            memset(plateBuff, 0, 64);
                            snprintf(plateBuff, 64, "\"皖%c%c%c%c%c%c\";1;1\n",
                                    szBaseSting[a],
                                    szBaseSting[b],
                                    szBaseSting[c],
                                    szBaseSting[d],
                                    szBaseSting[e],
                                    szBaseSting[f]);
                            if (fd < 0)
                            {
                                goto RET;
                            }
                            write(fd, plateBuff, strlen(plateBuff));
                            if (++stat == cnt)
                            {
                                goto RET;
                            }
                        }
                    }
                }
            }
        }
    }
RET: 
    fsync(fd);

    close(fd);
    fd = -1;
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("plz input the plate count:\n");
    }

    generate(atoi(argv[1]));

    return 0;
}
