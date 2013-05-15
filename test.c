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

    STATICS_START("Query");
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

    pause();

    return 0;
}

