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

    if (-1 == bl_delete_records_by_plate_type(WHITE))
    {
        LOG("delete record by type failed!");
    }
    else
    {
        LOG("delete record by type success!");
    }
    usleep(1000);
    bl_export("./export.txt", ";");

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

    if (-1 == bl_import("black_list.txt", ";"))
    {
        LOG("import blacklist error!");
    }
    else 
    {
        LOG("Import blackList success!");
    }

    char *szPlateNumber = "皖A-11111";

    PLATE_RECORD_T PlateRecord = {
        .PlateType = BLUE,
        .szPlateNumber = szPlateNumber,
        .szCommentStr = "hello insert",
    };
    PLATE_RECORD_T *pPlateRecord = &PlateRecord;
    
    if (-1 == bl_insert_record(pPlateRecord))
    {
        LOG("Insert record error!");
    }
    else 
    {
        LOG("Insert record success!");
    }

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

    char *szPlateNumber_1 = "皖A-53333";
    if (-1 == bl_delete_record_by_plate_number(szPlateNumber_1))
    {
        LOG("delete record error!");
    }
    else 
    {
        LOG("delete record success!");
    }

    char *szPlateNumber_2 = "皖A-43333";
    char *szComment = "Modify comment str";
    if (-1 == bl_modify_record_comment(szPlateNumber_2, szComment))
    {
        LOG("modify comment failed!");
    }
    else
    {
        LOG("modify comment success!");
    }

    pthread_join(tid, NULL);

    return 0;
}

