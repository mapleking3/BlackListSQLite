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

static pthread_t tid;

static sem_t sem_db;

static int bRun = 1;

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
}

void *thread(void *pArg)
{
    pArg = pArg;

    char PlateAtHead[] = "皖A11111";
    char PlateAtMid[] = "皖A1APS3";
    char PlateAtTail[] = "皖A1LFLR";
    int ret = 0;
    int cnt = 0;

    sem_wait(&sem_db);

    while (bRun == 1 && cnt++ < 1000000 )
    {
        printf("SPLIT####################################\n");

        STATICS_START("SELECT HEAD");
        ret = bl_query(PlateAtHead, NULL);
        if (-1 == ret)
        {
            printf("Select Error!\n");
        }
        else if (0 == ret)
        {
        }
        else if (1 == ret)
        {
            printf("Find It\n");
        }
        STATICS_STOP();

        STATICS_START("SELECT MIDDLE");
        ret = bl_query(PlateAtMid, NULL);
        if (-1 == ret)
        {
            printf("Select Error!\n");
        }
        else if (0 == ret)
        {
        }
        else if (1 == ret)
        {
            printf("Find It\n");
        }
        STATICS_STOP();

        STATICS_START("SELECT TAIL");
        ret = bl_query(PlateAtTail, NULL);
        if (-1 == ret)
        {
            printf("Select Error!\n");
        }
        else if (0 == ret)
        {
        }
        else if (1 == ret)
        {
            printf("Find It\n");
        }
        STATICS_STOP();

        STATICS_START("SELECT UNIN");
        ret = bl_query("皖R55555", NULL);
        if (-1 == ret)
        {
            printf("Select Error!\n");
        }
        else if (0 == ret)
        {
        }
        else if (1 == ret)
        {
            printf("Find It\n");
        }
        STATICS_STOP();

        //usleep(500000);
    }
    return NULL;
}

int main(void)
{
    SetSignalHandler();

    if (-1 == sem_init(&sem_db, 0, 0))
    {
        LOG("Sem Init Error:%s", strerror(errno));
        return -1;
    }

    if (-1 == bwl_init_database("./test.db"))
    {
        LOG("Init error!");
        return 0;
    }

#if 1
    if (pthread_create(&tid, NULL, thread, NULL) != 0)
    {
        LOG("Create Thread Error:%s",strerror(errno));
        exit(-1);
    }
#endif

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

    sem_post(&sem_db);

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
    printf("*********************************************\n");

    pthread_join(tid, NULL);

#if 0
    STATICS_START("EXPORT");
    bl_export("./export.txt", ";");
    STATICS_STOP();

    STATICS_START("EXPORT");
    bwl_backup_database("backup.dat");
    STATICS_STOP();
#endif

    bwl_close_database();

    return 0;
}

