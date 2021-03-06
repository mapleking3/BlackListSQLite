/**
 * @filename:   plate_bw_list.c
 * @brief:      plate blacklist and whitelist realized in sqlite3
 * @author:     Retton
 * @version:    V1.0.0
 * @date:       2013-05-17
 * Copyright:   2012-2038 Anhui CHAOYUAN Info Technology Co.Ltd
 */
#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "plate_bw_list.h"

#define BACKUP_PAGECOUNT 10
#define PLATE_BUFFER_CNT 100

#define LOG(format, ...)                                            \
        do {printf("%s in %s Line[%d]:"format"\n",                   \
                __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);   \
        } while(0)
#define STATICS_START(name)                             \
        {                                               \
        struct timeval start;                           \
        struct timeval end;                             \
        const char *statics_name = name;                \
        gettimeofday(&start, NULL);                     \
        {
#define STATICS_STOP()                                  \
        }                                               \
        gettimeofday(&end, NULL);                       \
        printf("%s : ms[ %ld ] or  us[ %ld ]\n", statics_name,  \
               (end.tv_sec  - start.tv_sec)  * 1000 +                   \
               (end.tv_usec - start.tv_usec) / 1000,                    \
                (end.tv_sec - start.tv_sec) * 1000 * 1000 +             \
                (end.tv_usec - start.tv_usec));                         \
        }

extern int bRun;
/*local static variables */
static sqlite3 *db = NULL;
static sqlite3_mutex *db_mutex = NULL;
static const char *szBlackListTable = "BlackList";
static const char *szWhiteListTable = "WhiteList";
static char PlateBuffer[MAX_PLATE_NUMBER*PLATE_BUFFER_CNT] = {0};
static pthread_mutex_t cnt_mutex;
static int PlateCnt = 0;
static int OverCnt = 0;
static pthread_t query_tid = -1;

static int import(const char *szTableName, const char *szImportFileName, const char *szRecordSeparator);
static int export(const char *szTableName, const char *szExportFileName, const char *szRecordSeparator);
static int query(const char *szTableName, const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
static int insert_record(const char *szTableName, PLATE_RECORD_T *pPlateRecord);
static int clear_record(const char *szTableName);
static int modify_record_by_plate_number(const char *TableName, const char *PlateNumber, PLATE_TYPE PlateType, const char *szCommentStr);
static int delete_record_by_plate_number(const char *szTableName, const char *szPlateNumber);
static int delete_records_by_plate_type(const char *szTableName, PLATE_TYPE PlateType);
static void *query_thread(void *);

void db_change_hook(void *pArg, int actionMode, const char *dbName, const char *tableName, long long affectRow)
{
    pArg = pArg;
    char action[16] = {0};
    switch (actionMode)
    {
        case SQLITE_INSERT:
            strcpy(action, "insert");
            break;
        case SQLITE_UPDATE:
            strcpy(action, "modify");
            break;
        case SQLITE_DELETE:
            strcpy(action, "delete");
            break;
        default:
            printf("Unknow Change\n");
            return;
    }
    LOG("%s in %s: %s in %lld\n", tableName, dbName, action, affectRow);
    return;

}

void plate_buffer_init()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutex_init(&cnt_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    memset(PlateBuffer, 0, MAX_PLATE_NUMBER*PLATE_BUFFER_CNT);
    PlateCnt = 0;
    OverCnt = 0;
}

void plate_buffer_put(const char *PlateNumber)
{
    if (PlateCnt < PLATE_BUFFER_CNT)
    {
        pthread_mutex_lock(&cnt_mutex);
        //memcpy(PlateBuffer+PlateCnt*MAX_PLATE_NUMBER, PlateNumber, strlen(PlateNumber)+1);
        strncpy(PlateBuffer+PlateCnt*MAX_PLATE_NUMBER, PlateNumber, strlen(PlateNumber)+1);
        PlateCnt++;
        pthread_mutex_unlock(&cnt_mutex);
    }
    else if (PlateCnt == PLATE_BUFFER_CNT)
    {
        printf("PlateBuffer OverFlow!\n");
        if (OverCnt == PLATE_BUFFER_CNT)
        {
            OverCnt = 0;
        }
        pthread_mutex_lock(&cnt_mutex);
        //memcpy(PlateBuffer+OverCnt*MAX_PLATE_NUMBER, PlateNumber, strlen(PlateNumber)+1);
        strncpy(PlateBuffer+OverCnt*MAX_PLATE_NUMBER, PlateNumber, strlen(PlateNumber)+1);
        OverCnt++;
        pthread_mutex_unlock(&cnt_mutex);
    }
}

char *plate_buffer_get(void)
{
    if (PlateCnt > 0)
    {
        pthread_mutex_lock(&cnt_mutex);
        char *PlateNumber = (char *)(PlateBuffer+(PlateCnt-1)*MAX_PLATE_NUMBER);
        printf("Plate:%s BufferCnt:%d\n",PlateNumber, PlateCnt);
        PlateCnt--;
        pthread_mutex_unlock(&cnt_mutex);
        return PlateNumber;
    }
    else
    {
        return NULL;
    }
}

int bwl_init_database(const char *szDatabaseFilePath)
{
    if (SQLITE_OK != sqlite3_config(SQLITE_CONFIG_SERIALIZED))
    {
        LOG("Configure Sqlite3 error:%s.", sqlite3_errmsg(db));
        return BWLIST_ERROR;
    }

    if (SQLITE_OK != sqlite3_open(szDatabaseFilePath, &db))
    {
        LOG("Can't open database:%s.", sqlite3_errmsg(db));
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    sqlite3_busy_timeout(db, 1);

    //sqlite3_update_hook(db, db_change_hook, NULL);

    if (NULL == (db_mutex = sqlite3_db_mutex(db)))
    {
        LOG("SQLITE IS NOT IN SERIALIZED MOD");
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    sqlite3_mutex_enter(db_mutex);
    if (SQLITE_OK != sqlite3_exec(db, "PRAGMA page_size=4096;", 0, 0, NULL))
    {
        LOG("Can't set page_size:%s.", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    if (SQLITE_OK != sqlite3_exec(db, "PRAGMA cache_size=8000;", 0, 0, NULL))
    {
        LOG("Can't set cache_size:%s.", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    // if blacklist not exists, create it
    char *sSqlCreateBlacklist = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %Q"
            "(PlateNumber TEXT NOT NULL PRIMARY KEY, PlateType INTEGER, Comment TEXT);", szBlackListTable);
    int rc_bl = sqlite3_exec(db, sSqlCreateBlacklist, 0, 0, NULL);
    sqlite3_free(sSqlCreateBlacklist);
    if (SQLITE_OK != rc_bl)
    {
        LOG("Create BlackList Error:%s", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    // if whitelist not exists, create it
    char *sSqlCreateWhitelist = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %Q"
            "(PlateNumber TEXT NOT NULL PRIMARY KEY, PlateType INTEGER, Comment TEXT);", szWhiteListTable);
    int rc_wl = sqlite3_exec(db, sSqlCreateWhitelist, 0, 0, NULL);
    sqlite3_free(sSqlCreateWhitelist);
    if (SQLITE_OK != rc_wl)
    {
        LOG("Create WhiteList error:%s.", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    sqlite3_mutex_leave(db_mutex);

    if (0 != pthread_create(&query_tid, NULL, query_thread, NULL))
    {
        LOG("%s", strerror(errno));
        sqlite3_close(db);
        return BWLIST_ERROR;
    }

    return BWLIST_OK;
}

int bwl_backup_database(const char *szBackupDBFilePath)
{
#if SQLITE_VERSION_NUMBER >= 3006011
    if (NULL == db)
    {
        LOG("Blacklist Whitelist Database File Has Been Closed!");
        return BWLIST_ERROR;
    }

    sqlite3* pDBBake;
    sqlite3_backup* pBackup;
    int rc;
    rc = sqlite3_open(szBackupDBFilePath, &pDBBake);
    if (rc != SQLITE_OK)
    {
        LOG("Open Backup DB error:%s.", sqlite3_errmsg(pDBBake));
        sqlite3_close(pDBBake);
        return BWLIST_ERROR;
    }

    pBackup = sqlite3_backup_init(pDBBake, "main", db, "main");
    if (pBackup == 0)
    {
        LOG("Backup Init Error:%s.", sqlite3_errmsg(pDBBake));
        sqlite3_close(pDBBake);
        return BWLIST_ERROR;
    }

    do
    {
        rc = sqlite3_backup_step(pBackup, BACKUP_PAGECOUNT);
        if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
        {
          sqlite3_sleep(250);
        }
    }
    while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);

    sqlite3_backup_finish(pBackup);
    if (rc == SQLITE_DONE)
    {
        sqlite3_close(pDBBake);
        return BWLIST_OK;
    }
    else
    {
        LOG("Backup Step Error:%s.", sqlite3_errmsg(pDBBake));
        sqlite3_close(pDBBake);
        return BWLIST_ERROR;
    }

#else

    LOG("Backup not supported in this SQLite Version!");
    return BWLIST_ERROR;

#endif
}

int bwl_close_database(void)
{
    int ret = sqlite3_close(db);
    if (SQLITE_OK != ret)
    {
        if (SQLITE_BUSY == ret)
        {
            LOG("unfinalized prepared statements or unfinished sqlite3_backup objects");
        }
        return BWLIST_ERROR;
    }
    else
    {
        return BWLIST_OK;
    }
}


int bl_import(const char *szImportFileName, const char *szRecordSeparator)
{
    return import(szBlackListTable, szImportFileName, szRecordSeparator);
}

int wl_import(const char *szImportFileName, const char *szRecordSeparator)
{
    return import(szWhiteListTable, szImportFileName, szRecordSeparator);
}

int bl_export(const char *szExportFileName, const char *szRecordSeparator)
{
    return export(szBlackListTable, szExportFileName, szRecordSeparator);
}

int wl_export(const char *szExportFileName, const char *szRecordSeparator)
{
    return export(szWhiteListTable, szExportFileName, szRecordSeparator);
}

#if 0
int blist_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    return query(szBlackListTable, szPlateNumber, pPlateRecord); 
}
#else
int blist_query(const char *szPlateNumber)
{
    plate_buffer_put(szPlateNumber);
    return 0;
}
#endif

int wl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    return query(szWhiteListTable, szPlateNumber, pPlateRecord);
}

int bl_insert_record(PLATE_RECORD_T *pPlateRecord)
{
    return insert_record(szBlackListTable, pPlateRecord);
}

int wl_insert_record(PLATE_RECORD_T *pPlateRecord)
{
    return insert_record(szWhiteListTable, pPlateRecord);
}

int bl_delete_record_by_plate_number(const char *szPlateNumber)
{
    return delete_record_by_plate_number(szBlackListTable, szPlateNumber);
}

int wl_delete_record_by_plate_number(const char *szPlateNumber)
{
    return delete_record_by_plate_number(szWhiteListTable, szPlateNumber);
}

int bl_delete_records_by_plate_type(PLATE_TYPE ePlateType_t)
{
    return delete_records_by_plate_type(szBlackListTable, ePlateType_t);
}

int wl_delete_records_by_plate_type(PLATE_TYPE ePlateType_t)
{
    return delete_records_by_plate_type(szWhiteListTable, ePlateType_t);
}

int bl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr)
{
    return modify_record_by_plate_number(szBlackListTable, szPlateNumber, PlateType, szCommentStr);
}

int wl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr)
{
    return modify_record_by_plate_number(szWhiteListTable, szPlateNumber, PlateType,  szCommentStr);
}

int bl_clear_records(void)
{
    return clear_record(szBlackListTable);
}

int wl_clear_records(void)
{
    return clear_record(szWhiteListTable);
}

/* end of extern api */

/* local function */
static int exec_sql_not_select(const char *sql)
{
    sqlite3_mutex_enter(db_mutex);
    int ret = sqlite3_exec(db, sql, 0, 0, NULL);
    if (ret != SQLITE_OK)
    {
        LOG("exec sql error:%s", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        return BWLIST_ERROR;
    }
    sqlite3_mutex_leave(db_mutex);
    return BWLIST_OK;
}

static char *local_getline(FILE *pFileIn, int csvFlag)
{
    int nLine = 100;
    int n = 0;
    int inQuote = 0;
    char *zLine = malloc(nLine);

    if(NULL == zLine)
    {
        return BWLIST_OK;
    }

    for (;;)
    {
        if(n+100 > nLine)
        {
            nLine = nLine*2 + 100;
            zLine = realloc(zLine, nLine);
            if(0 == zLine)
            {
                return BWLIST_OK;
            }
        }

        if(0 ==  fgets(&zLine[n], nLine - n, pFileIn))
        {
            if( n==0 )
            {
                free(zLine);
                return BWLIST_OK;
            }
            zLine[n] = 0;
            break;
        }

        while(zLine[n])
        {
            if( zLine[n]=='"' ) inQuote = !inQuote;
            n++;
        }

        if(n>0 && zLine[n-1]=='\n' && (!inQuote || !csvFlag))
        {
            n--;
            if( n>0 && zLine[n-1]=='\r' )
            {
                n--;
            }
            zLine[n] = 0;
            break;
        }
    }

    zLine = realloc(zLine, n+1);
    return zLine;
}

static int strlen30(const char *str)
{
    const char *temp = str;
    while (*temp)
    {
        temp++;
    }
    return 0x3fffffff & (int)(temp-str);
}

static int import(const char *szTableName, const char *szImportFileName, const char *szRecordSeparator)
{
    sqlite3_stmt *pStmt = NULL;
    int nCol;
    int i, j;
    char *zLine;
    char **azCol;
    char *zCommit;
    FILE *pFileIn;
    int lineno = 0;
    int nSep = strlen30(szRecordSeparator);

    if(0 == nSep)
    {
        LOG("Error: non-null separator required for import.");
        return BWLIST_ERROR;
    }

    char *SqlString = sqlite3_mprintf("SELECT * FROM %s", szTableName);
    if(NULL == SqlString)
    {
        LOG("Error: out of memory.");
        return BWLIST_ERROR;
    }
    int nByte = strlen30(SqlString);

    sqlite3_mutex_enter(db_mutex);
    int rc = sqlite3_prepare_v2(db, SqlString, -1, &pStmt, 0);
    sqlite3_free(SqlString);
    SqlString = NULL;
    if( rc )
    {
        sqlite3_finalize(pStmt);
        sqlite3_mutex_leave(db_mutex);
        LOG("Error: %s.", sqlite3_errmsg(db));
        return BWLIST_ERROR;
    }
    sqlite3_mutex_leave(db_mutex);

    nCol = sqlite3_column_count(pStmt);
    sqlite3_finalize(pStmt);
    pStmt = NULL;

    if(0 == nCol)
    {
        return BWLIST_OK; /* no columns, no error */
    }

    SqlString = malloc( nByte + 20 + nCol*2 );
    if(NULL == SqlString)
    {
        LOG("Error: out of memory!.");
        return BWLIST_ERROR;
    }
    sqlite3_snprintf(nByte+20, SqlString, "INSERT INTO %s VALUES(?", szTableName);
    j = strlen30(SqlString);

    for(i = 1; i < nCol; i++)
    {
        SqlString[j++] = ',';
        SqlString[j++] = '?';
    }
    SqlString[j++] = ')';
    SqlString[j] = 0;

    sqlite3_mutex_enter(db_mutex);
    rc = sqlite3_prepare_v2(db, SqlString, -1, &pStmt, 0);
    free(SqlString);
    SqlString = NULL;
    if(rc)
    {
        LOG("Error: %s.", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        sqlite3_mutex_leave(db_mutex);
        return BWLIST_ERROR;
    }
    sqlite3_mutex_leave(db_mutex);
    pFileIn = fopen(szImportFileName, "rb");
    if( pFileIn==0 )
    {
        LOG("Error: cannot open \"%s\".", szImportFileName);
        sqlite3_finalize(pStmt);
        return BWLIST_ERROR;
    }
    azCol = malloc(sizeof(azCol[0])*(nCol+1) );
    if( azCol==0 )
    {
        LOG("Error: out of memory.");
        fclose(pFileIn);
        sqlite3_finalize(pStmt);
        return BWLIST_ERROR;
    }

    sqlite3_mutex_enter(db_mutex);
    //BEGIN IMMEDIATE avoid deadlock
    if (SQLITE_OK != sqlite3_exec(db, "BEGIN IMMEDIATE", 0, 0, 0))
    {
        LOG("BEGIN TRANSACTION Error:%s", sqlite3_errmsg(db));
        fclose(pFileIn);
        sqlite3_mutex_leave(db_mutex);
        sqlite3_finalize(pStmt);
        return BWLIST_ERROR;
    }
    sqlite3_mutex_leave(db_mutex);

    zCommit = "COMMIT";

    while( (zLine = local_getline(pFileIn, 1))!=0 )
    {
        char *z, c;
        int inQuote = 0;
        lineno++;
        azCol[0] = zLine;

        for(i=0, z=zLine; (c = *z)!=0; z++)
        {
            if(c=='"')
            {
                inQuote = !inQuote;
            }
            if(c=='\n')
            {
                lineno++;
            }
            if(!inQuote && c==szRecordSeparator[0] && strncmp(z,szRecordSeparator,nSep)==0)
            {
                *z = 0;
                i++;
                if(i<nCol)
                {
                    azCol[i] = &z[nSep];
                    z += nSep-1;
                }
            }
        } /* end for */

        *z = 0;
        if( i+1!=nCol )
        {
            LOG("Error: %s line %d: expected %d columns of data bud found %d.", szImportFileName, lineno, nCol, i+1);
            zCommit = "ROLLBACK";
            free(zLine);
            rc = 1;
            break; /* from while */
        }

        for(i=0; i<nCol; i++)
        {
            if( azCol[i][0]=='"' )
            {
                int k;
                for(z=azCol[i], j=1, k=0; z[j]; j++)
                {
                    if( z[j]=='"' )
                    {
                        j++;
                        if(z[j]==0)
                        {
                            break;
                        }
                    }
                    z[k++] = z[j];
                }
                z[k] = 0;
            }
            sqlite3_bind_text(pStmt, i+1, azCol[i], -1, SQLITE_STATIC);
        }

        sqlite3_mutex_enter(db_mutex);
        sqlite3_step(pStmt);
        rc = sqlite3_reset(pStmt);
        free(zLine);

        //if(rc!=SQLITE_OK)
        //SQLITE_CONSTRAINT: abort due to constraint violation
        if (SQLITE_OK != rc && SQLITE_CONSTRAINT != rc)
        {
            LOG("Error:%s", sqlite3_errmsg(db));
            sqlite3_mutex_leave(db_mutex);
            zCommit = "ROLLBACK";
            rc = 1;
            break; /* from while */
        }
        sqlite3_mutex_leave(db_mutex);
    } /* end while */

    free(azCol);
    fclose(pFileIn);
    sqlite3_finalize(pStmt);
    if (sqlite3_exec(db, zCommit, 0, 0, 0) != SQLITE_OK)
    {
        return BWLIST_ERROR;
    }
    if (strcmp(zCommit, "COMMIT") == 0)
    {
        return BWLIST_OK;
    }
    else
    {
        return BWLIST_ERROR;
    }
}

static int export(const char *szTableName, const char *szExportFileName, const char *szRecordSeparator)
{
    FILE *fp = fopen(szExportFileName, "a");

    if (NULL == fp)
    {
        LOG("Open export file error:%s", strerror(errno));
        return BWLIST_ERROR;
    }

    char *sql_select = sqlite3_mprintf("SELECT * FROM %q;", szTableName);

    sqlite3_stmt *stmt = NULL;

    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL))
    {
        goto ErrReturn;
    }

    int fieldCount = sqlite3_column_count(stmt);
    const unsigned int LENGTH = 50;
    int currField = 0;
    char *szTemp = NULL;
    char *temp = NULL;
    int len = 0;

    for (;;)
    {
        int ret = sqlite3_step(stmt);
        if (SQLITE_ROW == ret)
        {
            char *str = (char *)malloc(LENGTH);
            if (NULL == str)
            {
                LOG("malloc error");
                goto ErrReturn;
            }
            memset(str, 0, LENGTH);

            for (currField = 0; currField < fieldCount; ++currField)
            {
                int type = sqlite3_column_type(stmt, currField);
                switch(type)
                {
                    case SQLITE_INTEGER:
                        szTemp = sqlite3_mprintf("%d", sqlite3_column_int(stmt, currField));
                        if (strlen(szTemp)+strlen(str)+2 > LENGTH)
                        {
                            temp = realloc(str, strlen(szTemp)+strlen(str)+2);
                            if (NULL == temp)
                            {
                                LOG("realloc error");
                                free(str);
                                goto ErrReturn;
                            }
                            str = temp;
                            temp = NULL;
                        }
                        str = strcat(str, szTemp);
                        sqlite3_free(szTemp);
                        break;
                    case SQLITE_FLOAT:
                        szTemp = sqlite3_mprintf("%lf", sqlite3_column_double(stmt, currField));
                        if (strlen(szTemp)+strlen(str)+2 > LENGTH)
                        {
                            temp = realloc(str, strlen(szTemp)+strlen(str)+2);
                            if (NULL == temp)
                            {
                                LOG("realloc error");
                                free(str);
                                goto ErrReturn;
                            }
                            str = temp;
                            temp = NULL;
                        }
                        str = strcat(str, szTemp);
                        sqlite3_free(szTemp);
                        szTemp = NULL;
                        break;
                    case SQLITE_TEXT:
                        szTemp = (char *)sqlite3_column_text(stmt, currField);
                        if (strlen(szTemp)+strlen(str)+4 > LENGTH)
                        {
                            temp = realloc(str, strlen(szTemp)+strlen(str)+4);
                            if (NULL == temp)
                            {
                                LOG("realloc error");
                                free(str);
                                goto ErrReturn;
                            }
                            str = temp;
                            temp = NULL;
                        }
                        len = strlen(str);
                        str[len] = '"';
                        str[len+1] = 0;
                        str = strcat(str, szTemp);
                        len = strlen(str);
                        str[len] = '"';
                        str[len+1] = 0;
                        szTemp = NULL;
                        break;
                    default:
                        break;
                }

                if (currField < fieldCount -1)
                {
                    str = strcat(str, szRecordSeparator);
                }
            }

            len = strlen(str);
            str[len] = '\n';
            str[len+1] = 0;
            fwrite(str, len+1, 1, fp);
            fflush(fp);
            free(str);
            str = NULL;
        }
        else if (SQLITE_DONE == ret)
        {
            break;
        }
        else
        {
            goto ErrReturn;
        }
    }

    sqlite3_finalize(stmt);
    fclose(fp);
    stmt = NULL;
    fp = NULL;
    return BWLIST_OK;

ErrReturn:
    sqlite3_finalize(stmt);
    fclose(fp);
    stmt = NULL;
    fp = NULL;
    return BWLIST_ERROR;
}

static int query(const char *szTableName, const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    sqlite3_stmt *stmt_select = NULL;

    char *sql_select = sqlite3_mprintf("SELECT * FROM %q where PlateNumber=%Q;", szTableName, szPlateNumber);

    sqlite3_mutex_enter(db_mutex);
    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_select, -1, &stmt_select, 0))
    {
        LOG("Sqlite3 prepare v2 error:%s", sqlite3_errmsg(db));
        sqlite3_mutex_leave(db_mutex);
        sqlite3_finalize(stmt_select);
        sqlite3_free(sql_select);
        return BWLIST_ERROR;
    }
    sqlite3_mutex_leave(db_mutex);

    sqlite3_free(sql_select);

    int fieldCount = sqlite3_column_count(stmt_select);

    int ret = sqlite3_step(stmt_select);

    if (SQLITE_DONE == ret)
    {
        LOG("%s is not in BlackList.\n", szPlateNumber);
        sqlite3_finalize(stmt_select);
        stmt_select = NULL;
        return 0;
    }
    else if (SQLITE_ROW == ret)
    {
        if (NULL != pPlateRecord)
        {
            int iTempPlateType = 0;
            char *sTempCommentStr = 0;
            int currField;

            for (currField = 0; currField < fieldCount; ++currField)
            {
                const char *szTemp = sqlite3_column_name(stmt_select, currField);

                if (strcmp(szTemp, "PlateType") == 0)
                {
                    iTempPlateType = sqlite3_column_int(stmt_select, currField);
                }
                else if (strcmp(szTemp, "Comment") == 0)
                {
                    sTempCommentStr = (char *)sqlite3_column_text(stmt_select, currField);
                }
            }

            memcpy(pPlateRecord->szPlateNumber, szPlateNumber, MAX_PLATE_NUMBER);
            pPlateRecord->PlateType = iTempPlateType;
            memcpy(pPlateRecord->szCommentStr, sTempCommentStr, MAX_COMMENT_LENGTH);
        }
        sqlite3_finalize(stmt_select);
        stmt_select = NULL;
        return 1;
    }
    else if (SQLITE_INTERRUPT == ret)
    {
        LOG("SQLITE_INTERRUPT Query!");
        return BWLIST_ERROR;
    }
    else
    {
        sqlite3_finalize(stmt_select);
        stmt_select = NULL;
        return BWLIST_ERROR;
    }
}

static int insert_record(const char *szTableName, PLATE_RECORD_T *pPlateRecord)
{
    char *sql_insert = sqlite3_mprintf("INSERT INTO %q VALUES(%Q,%d,%Q);",
                                        szTableName, pPlateRecord->szPlateNumber,
                                        pPlateRecord->PlateType, pPlateRecord->szCommentStr);

    int ret = exec_sql_not_select(sql_insert);

    sqlite3_free(sql_insert);

    return ret;
}

static int clear_record(const char *szTableName)
{
    char *sql_clear = sqlite3_mprintf("DELETE FROM %q;", szTableName);

    int ret = exec_sql_not_select(sql_clear);

    sqlite3_free(sql_clear);

    return ret;
}

static int modify_record_by_plate_number(const char *TableName, const char *PlateNumber, PLATE_TYPE PlateType, const char *CommentStr)
{
    char *sql_modify = sqlite3_mprintf("UPDATE %q set PlateType=%d,Comment=%Q where PlateNumber=%Q;",
                                        TableName, PlateType, CommentStr, PlateNumber);

    int ret = exec_sql_not_select(sql_modify);

    sqlite3_free(sql_modify);

    return ret;
}

static int delete_record_by_plate_number(const char *szTableName, const char *szPlateNumber)
{
    char *sql_delete = sqlite3_mprintf("DELETE FROM %q where PlateNumber=%Q;",
                                        szTableName, szPlateNumber);

    int ret = exec_sql_not_select(sql_delete);

    sqlite3_free(sql_delete);

    return ret;
}

static int delete_records_by_plate_type(const char *szTableName, PLATE_TYPE PlateType)
{
    char *sql_delete = sqlite3_mprintf("DELETE FROM %q where PlateType=%d;",
                                        szTableName, PlateType);

    int ret = exec_sql_not_select(sql_delete);

    sqlite3_free(sql_delete);

    return ret;
}

void *query_thread(void *pArg)
{
    pArg = pArg;
    int cnt = 0;
    for (;;)
    {
        PLATE_RECORD_T PlateRecord;
        char *PlateNumber = plate_buffer_get();
        if (PlateNumber == NULL)
        {
            usleep(100);
            if (bRun == 0)
            {
                printf("Query Count:%d\n", cnt);
                return NULL;
            }
            continue;
        }
        cnt++;
        int ret = 0; 
        STATICS_START(PlateNumber);
        ret = query(szBlackListTable, PlateNumber, &PlateRecord);
        STATICS_STOP();

        if (ret < 0)
        {
            LOG("Query Error!");
        }
        else if (ret == 0)
        {
        }
        else if (ret == 1)
        {
            printf("Find Int\n");
        }
    }
    return NULL;
}
