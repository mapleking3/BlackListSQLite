#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "plate_bw_list.h"


int main(void)
{

    if (-1 == bwl_init_database("./test.db"))
    {
        printf("Init error!\n");
        return 0;
    }

    if (-1 == bl_import("black_list.txt", ";"))
    {
        printf("import blacklist error!\n");
        return 0;
    }

    char *szPlateNumber = "皖A-11111";

    PLATE_RECORD_T PlateRecord = {
        .PlateType_t = BLUE,
        .szPlateNumber = szPlateNumber,
        .szCommentStr = "hello insert",
    };
    PLATE_RECORD_T *pPlateRecord = &PlateRecord;
    
    if (-1 == bl_insert_record(pPlateRecord))
    {
        printf("Insert record error!\n");
    }
    else 
    {
        printf("Insert record success!\n");
    }

    int ret = bl_query(szPlateNumber, pPlateRecord);

    if (ret == 0)
    {
        printf("Plate:%s not found!\n", szPlateNumber);
    }
    else if (ret == 1)
    {
        printf("Plate:%s is in blacklist!\n", szPlateNumber);
        printf("PlateNumber:%s<***>PlateType:%d<***>Comment:%s\n",
                pPlateRecord->szPlateNumber,
                pPlateRecord->PlateType_t,
                pPlateRecord->szCommentStr);
    }
    else 
    {
        printf("Query error!\n");
        return -1;
    }

    char *szPlateNumber_1 = "皖A-53333";
    if (-1 == bl_delete_record_by_plate_number(szPlateNumber_1))
    {
        printf("delete record error!\n");
    }
    else 
    {
        printf("delete record success!\n");
    }

    if (-1 == bl_delete_records_by_plate_type(WHITE))
    {
        printf("delete record by type failed!\n");
    }
    else
    {
        printf("delete record by type success!\n");
    }

    char *szPlateNumber_2 = "皖A-43333";
    char *szComment = "Modify comment str";
    if (-1 == bl_modify_record_comment(szPlateNumber_2, szComment))
    {
        printf("modify comment failed!\n");
    }
    else
    {
        printf("modify comment success!\n");
    }

    return 0;
}

