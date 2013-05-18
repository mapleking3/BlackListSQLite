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

static pthread_t tid;

void *thread(void *pArg)
{
    pArg = NULL;

    return NULL;
}



int main(void)
{

    if (-1 == bwl_init_database("./test.db"))
    {
        LOG("Init error!");
        return 0;
    }

    if (pthread_create(&tid, NULL, thread, NULL) != 0)
    {
        LOG("Create Thread Error:%s",strerror(errno));
        exit(-1);
    }

    STATICS_START("Import");
    if (-1 == bl_import("black_list.txt", ";"))
    {
        LOG("import blacklist error!");
    }
    else 
    {
        LOG("Import blackList success!");
    }
    STATICS_STOP();

    char *szPlateNumber = "皖A-fffff";

    PLATE_RECORD_T *pPlateRecord = (PLATE_RECORD_T *)malloc(sizeof(PLATE_RECORD_T));
    pPlateRecord->szPlateNumber = (char *)malloc(sizeof(char)*12);
    pPlateRecord->szCommentStr = (char *)malloc(sizeof(char)*12);

    if (pPlateRecord == NULL || pPlateRecord->szPlateNumber == NULL
            || pPlateRecord->szCommentStr == NULL)
    {
        free(pPlateRecord->szPlateNumber);
        free(pPlateRecord->szCommentStr);
        free(pPlateRecord);
        exit(0);
    }

    STATICS_START("QUERY");
    int ret = bl_query(szPlateNumber, pPlateRecord);

    if (ret == 0)
    {
        LOG("Plate:%s not found!", szPlateNumber);
    }
    else if (ret == 1)
    {
        LOG("Plate:%s is in blacklist!", szPlateNumber);
        LOG("PlateNumber:%s<***>PlateType:%d<***>Comment:%s",
                pPlateRecord->szPlateNumber,
                pPlateRecord->PlateType,
                pPlateRecord->szCommentStr);
    }
    else 
    {
        LOG("Query error!");
        return -1;
    }
    STATICS_STOP();

    pthread_join(tid, NULL);

    return 0;
}

