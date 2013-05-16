#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "plate_bw_list.h"
#include "common.h"

int main(void)
{

    if (-1 == bwl_init_database("./test.db"))
    {
        LOG("Init error!");
        return 0;
    }

    if (-1 == bl_import("black_list.txt", ";"))
    {
        LOG("import blacklist error!");
    }
    else 
    {
        LOG("Import blackList success!");
    }

    bl_export("./export.txt", ";");


    return 0;
}

