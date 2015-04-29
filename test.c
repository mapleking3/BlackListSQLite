#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>
#include "plate_bw_list.h"
#include "common.h"

int bRun = 1;

static void signal_handler(int signum)
{
    switch (signum) {
    case SIGCHLD:
    case SIGABRT:
    case SIGKILL:
            break;
    case SIGINT:
    case SIGTERM:
            printf("SetQuit!\n");
            bRun = 0;
            break;
    default:
            break;
    }

    return;
}

static void SetSignalHandler(void)
{
    struct sigaction sigAction;
    sigAction.sa_flags = 0;
    sigemptyset(&sigAction.sa_mask);
    sigaddset(&sigAction.sa_mask, SIGINT);
    sigAction.sa_handler = signal_handler;

    sigaction(SIGCHLD, &sigAction, NULL);
    sigaction(SIGABRT, &sigAction, NULL);
    sigaction(SIGTERM, &sigAction, NULL);
    sigaction(SIGKILL, &sigAction, NULL);
    sigaction(SIGINT, &sigAction, NULL);

    return;
}

int main(void)
{
    SetSignalHandler();

    if (-1 == bwl_init_database("./test.db"))
    {
        LOG("Init error!");
        return 0;
    }

#if 0
    if (-1 == bl_import("PlateList.txt", ";"))
    {
        LOG("import blacklist error!");
    }
    else 
    {
        LOG("Import blackList success!\n#####################\n\n");
    }
#endif

#if 1
    char PlateAtHead[]  = "皖AAAAAA";
    char PlateAtMid[]   = "皖AAKZ26";
    char PlateAtTail[]  = "皖AAVPV1";
    char PlateNoIn[]    = "皖CCCCCC";
    int cnt = 0;

    while (bRun == 1 && cnt++ < 100000 )
    {
        blist_query(PlateAtHead);
        usleep(50);
        
        blist_query(PlateAtMid);
        usleep(50);

        blist_query(PlateAtTail);
        usleep(50);

        blist_query(PlateNoIn);
        usleep(50);
    }
    fprintf(stderr, "Query Over!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    bRun = 0;
#endif


#if 0
    PLATE_RECORD_T PlateRecord = {
        .PlateType = BLUE,
        .szPlateNumber = "皖AE1212",
        .szCommentStr = "hello insert",
    };
    PLATE_RECORD_T *pPlateRecord = &PlateRecord;
    
    int cnt = 0;

    while (cnt++ < 100000 && bRun == 1)
    {
        STATICS_START("INSERT");
        if (-1 == bl_insert_record(pPlateRecord))
        {
            LOG("Insert record error!");
        }
        else 
        {
            LOG("Insert record success!");
        }
        STATICS_STOP();



        char *szComment = "Modify comment str";
        STATICS_START("MOD");
        if (-1 == bl_modify_record_by_plate_number(pPlateRecord->szPlateNumber, BLUE, szComment))
        {
            LOG("modify comment failed!");
        }
        else
        {
            LOG("modify comment success!");
        }
        STATICS_STOP();

        STATICS_START("DEL");
        if (-1 == bl_delete_record_by_plate_number(pPlateRecord->szPlateNumber))
        {
            LOG("delete record error!");
        }
        else 
        {
            LOG("delete record success!");
        }
        STATICS_STOP();
    }
#endif
#if 0
    STATICS_START("EXPORT");
    bl_export("./export.txt", ";");
    STATICS_STOP();

    //STATICS_START("EXPORT");
    //bwl_backup_database("backup.dat");
    //STATICS_STOP();
#endif

    bwl_close_database();
    printf("*********************************************\n");

    return 0;
}

