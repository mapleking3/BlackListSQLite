#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>
#include <semaphore.h>

#include "plate_bw_list.h"
#include "common.h"

#define FAILED  -1
#define SUCCESS 0

#define CMD_MSG_PATH    "/plate-cmd-msg"
#define CMD_MSG_OFLAG   (O_CREAT|O_RDWR|O_EXCL)
#define CMD_MSG_MODE    (0700)
#define MAX_MSGQ_NUMS   8
#define MAX_MSGQ_LENGTH 1
#define DEFAULT_MQ_PRIO 0

#define BL_CMD_IMPORT      1
#define BL_CMD_EXPORT      2
#define BL_CMD_INSERT      3
#define BL_CMD_CLEAR       4
#define BL_CMD_MODIFY      5
#define BL_CMD_DELETE_BY_PLATE_NUMBER 6
#define BL_CMD_DELETE_BY_PLATE_TYPE 7
#define WL_CMD_IMPORT      8
#define WL_CMD_EXPORT      9
#define WL_CMD_INSERT      10
#define WL_CMD_CLEAR       11
#define WL_CMD_MODIFY      12
#define WL_CMD_DELETE_BY_PLATE_NUMBER 13
#define WL_CMD_DELETE_BY_PLATE_TYPE 14
#define CMD_MAX             WL_CMD_DELETE_BY_PLATE_TYPE

typedef struct {
    void *p_arg1;
    void *p_arg2;
} CMD_ARG_T;

static CMD_ARG_T CmdArgment[CMD_MAX] = {{0}};


/*local static variables */
static mqd_t CmdMsgQ = -1;
static sem_t cmd_sem;
static pthread_t process_tid = -1;
static sqlite3 *db = NULL;
static const char *szBlackListTable = "BlackList";
static const char *szWhiteListTable = "WhiteList";


static int import(const char *szTableName, const char *szImportFileName, const char *szRecordSeparator);
static int export(const char *szTableName, const char *szExportFileName, const char *szRecordSeparator);
static int query(const char *szTableName, char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
static int insert_record(const char *szTableName, PLATE_RECORD_T *pPlateRecord);
static int clear_record(const char *szTableName);
static int modify_record_comment(const char *szTableName, char *szPlateNumber, char *szCommentStr);
static int delete_record_by_plate_number(const char *szTableName, char *szPlateNumber);
static int delete_records_by_plate_type(const char *szTableName, PLATE_TYPE PlateType);

void *ExternCmdTask(void *pArg)
{
    pArg = NULL;
    LOG("go into Extern Cmd Task.");

    for (;;)
    {
        unsigned char cmd;
        if (-1 == mq_receive(CmdMsgQ, (char *)&cmd, sizeof(cmd), NULL))
        {
            LOG("mq receive error:%s.", strerror(errno));
            continue;
        }

//        if (retVal != sizeof(cmd))
//        {
//            continue;
//        }
//
//        LOG("MessageQueue receive is %d", cmd);
//
        switch(cmd) 
        {
            case BL_CMD_IMPORT:
                {
                const char *szImportFileName    = (const char *)CmdArgment[BL_CMD_IMPORT].p_arg1;
                const char *szRecordSeparator   = (const char *)CmdArgment[BL_CMD_IMPORT].p_arg2;
                import(szBlackListTable, szImportFileName, szRecordSeparator);
                LOG("Import BlackList Success!");
                break;
                }
            case BL_CMD_EXPORT:
                {
                const char *szExportFileName    = (const char *)CmdArgment[BL_CMD_EXPORT].p_arg1;
                const char *szRecordSeparator   = (const char *)CmdArgment[BL_CMD_EXPORT].p_arg2;
                export(szBlackListTable, szExportFileName, szRecordSeparator);
                LOG("Export BlackList Success!");
                break;
                }
                break;
            case BL_CMD_INSERT:
                break;
            case BL_CMD_CLEAR:
                break;
            case BL_CMD_MODIFY:
                break;
            case BL_CMD_DELETE_BY_PLATE_NUMBER:
                break;
            case BL_CMD_DELETE_BY_PLATE_TYPE:
                break;
            case WL_CMD_IMPORT:
                {
                const char *szImportFileName    = (const char *)CmdArgment[WL_CMD_IMPORT].p_arg1;
                const char *szRecordSeparator   = (const char *)CmdArgment[WL_CMD_IMPORT].p_arg2;
                import(szWhiteListTable, szImportFileName, szRecordSeparator);
                LOG("Import WhiteList Success!");
                break;
                }
            case WL_CMD_EXPORT:
                {
                const char *szExportFileName    = (const char *)CmdArgment[WL_CMD_EXPORT].p_arg1;
                const char *szRecordSeparator   = (const char *)CmdArgment[WL_CMD_EXPORT].p_arg2;
                export(szWhiteListTable, szExportFileName, szRecordSeparator);
                LOG("Import WhiteList Success!");
                break;
                }
            case WL_CMD_INSERT:
                break;
            case WL_CMD_CLEAR:
                break;
            case WL_CMD_MODIFY:
                break;
            case WL_CMD_DELETE_BY_PLATE_NUMBER:
                break;
            case WL_CMD_DELETE_BY_PLATE_TYPE:
                break;
            default:
                LOG("DEFAULT");
                break;
        }
    }

    return NULL;
}

int bwl_init_database(const char *szDatabaseFilePath)
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    if (sqlite3_open(szDatabaseFilePath, &db) != SQLITE_OK)
    {
        LOG("Can't open database:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    if (sqlite3_exec(db, "PRAGMA page_size=4096;", 0, 0, NULL) != SQLITE_OK)
    {
        LOG("Can't set page_size:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    if (sqlite3_exec(db, "PRAGMA cache_size=25000;", 0, 0, NULL) != SQLITE_OK)
    {
        LOG("Can't set cache_size:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

#if 0
    char *sSqlCreateBlacklist = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %Q\
            (PlateNumber TEXT NOT NULL PRIMARY KEY, PlateType INTEGER, Comment TEXT);", szBlackListTable);
#endif 
    char *sSqlCreateBlacklist = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %Q\
            (PlateNumber TEXT NOT NULL, PlateType INTEGER, Comment TEXT);", szBlackListTable);
    char *sSqlCreateWhitelist = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %Q\
            (PlateNumber TEXT NOT NULL PRIMARY KEY, PlateType INTEGER, Comment TEXT);", szWhiteListTable);

    int rc_bl = sqlite3_exec(db, sSqlCreateBlacklist, 0, 0, NULL);
    int rc_wl = sqlite3_exec(db, sSqlCreateWhitelist, 0, 0, NULL);
    sqlite3_free(sSqlCreateBlacklist);
    sqlite3_free(sSqlCreateWhitelist);

    if (rc_bl != SQLITE_OK || rc_wl != SQLITE_OK)
    {
        LOG("Create BlackList error:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    struct mq_attr mq_attr;
    mq_attr.mq_maxmsg   = MAX_MSGQ_NUMS;
    mq_attr.mq_msgsize  = MAX_MSGQ_LENGTH;
    mq_unlink(CMD_MSG_PATH);
    CmdMsgQ = mq_open(CMD_MSG_PATH, CMD_MSG_OFLAG, CMD_MSG_MODE, &mq_attr);
    if (CmdMsgQ == -1)
    {
        LOG("Create Message Queue failed!\n");
        goto ErrReturn;
    }

    if (-1 == sem_init(&cmd_sem, 0, 0))
    {
        LOG("Create Semaphore error:%s", strerror(errno));
        goto ErrReturn;
    }

    if (pthread_create(&process_tid, NULL, ExternCmdTask, NULL) != 0)
    {
        LOG("Create pthread faile!\n");
        goto ErrReturn;
    }
    if (pthread_detach(process_tid) != 0)
    {
        LOG("detach pthread failed:%s", strerror(errno));
        goto ErrReturn;
    }

    return SUCCESS;

ErrReturn:
    sqlite3_close(db);
    return FAILED;
}

int bl_import(const char *szImportFileName, const char *szRecordSeparator)
{
    CmdArgment[BL_CMD_IMPORT].p_arg1 = (void *)szImportFileName;
    CmdArgment[BL_CMD_IMPORT].p_arg2 = (void *)szRecordSeparator;
    unsigned char cmd = BL_CMD_IMPORT;
    if (mq_send(CmdMsgQ, (char *)&cmd, 1, DEFAULT_MQ_PRIO) == -1)
    {
        LOG("MQ send error:%s\n", strerror(errno));
        return FAILED;
    }
    return SUCCESS;
}

int wl_import(const char *szImportFileName, const char *szRecordSeparator)
{
    CmdArgment[WL_CMD_IMPORT].p_arg1 = (void *)szImportFileName;
    CmdArgment[WL_CMD_IMPORT].p_arg2 = (void *)szRecordSeparator;
    unsigned char cmd = WL_CMD_IMPORT;
    if (mq_send(CmdMsgQ, (char *)&cmd, 1, DEFAULT_MQ_PRIO) == -1)
    {
        LOG("MQ send error:%s\n", strerror(errno));
        return FAILED;
    }
    return SUCCESS;
}

int bl_export(const char *szExportFileName, const char *szRecordSeparator)
{
    CmdArgment[BL_CMD_EXPORT].p_arg1 = (void *)szExportFileName;
    CmdArgment[BL_CMD_EXPORT].p_arg2 = (void *)szRecordSeparator;
    unsigned char cmd = WL_CMD_IMPORT;
    if (mq_send(CmdMsgQ, (char *)&cmd, 1, DEFAULT_MQ_PRIO) == -1)
    {
        LOG("MQ send error:%s\n", strerror(errno));
        return FAILED;
    }
    return SUCCESS;
}

int wl_export(const char *szExportFileName, const char *szRecordSeparator)
{
    CmdArgment[WL_CMD_EXPORT].p_arg1 = (void *)szExportFileName;
    CmdArgment[WL_CMD_EXPORT].p_arg2 = (void *)szRecordSeparator;
    unsigned char cmd = WL_CMD_EXPORT;
    if (mq_send(CmdMsgQ, (char *)&cmd, 1, DEFAULT_MQ_PRIO) == -1)
    {
        LOG("MQ send error:%s\n", strerror(errno));
        return FAILED;
    }
    return SUCCESS;
}

int bl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    return query(szBlackListTable, szPlateNumber, pPlateRecord);
}

int wl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
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

int bl_delete_record_by_plate_number(char *szPlateNumber)
{
    return delete_record_by_plate_number(szBlackListTable, szPlateNumber);
}

int wl_delete_record_by_plate_number(char *szPlateNumber)
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

int bl_modify_record_comment(char *szPlateNumber, char *szCommentStr)
{
    return modify_record_comment(szBlackListTable, szPlateNumber, szCommentStr);
}

int wl_modify_record_comment(char *szPlateNumber, char *szCommentStr)
{
    return modify_record_comment(szWhiteListTable, szPlateNumber, szCommentStr);
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
static int exec_sql_not_select(char *sql)
{
    int ret = sqlite3_exec(db, sql, 0, 0, NULL);
    if (ret != SQLITE_OK)
    {
        LOG("exec sql error:%s", sqlite3_errmsg(db));
        return FAILED;
    }
    return SUCCESS;
}

static char *local_getline(char *zPrompt, FILE *in, int csvFlag)
{
  char *zLine;
  int nLine;
  int n;
  int inQuote = 0;

  if( zPrompt && *zPrompt ){
    printf("%s",zPrompt);
    fflush(stdout);
  }
  nLine = 100;
  zLine = malloc( nLine );
  if( zLine==0 ) return SUCCESS;
  n = 0;
  while( 1 ){
    if( n+100>nLine ){
      nLine = nLine*2 + 100;
      zLine = realloc(zLine, nLine);
      if( zLine==0 ) return SUCCESS;
    }
    if( fgets(&zLine[n], nLine - n, in)==0 ){
      if( n==0 ){
        free(zLine);
        return SUCCESS;
      }
      zLine[n] = 0;
      break;
    }
    while( zLine[n] ){
      if( zLine[n]=='"' ) inQuote = !inQuote;
      n++;
    }
    if( n>0 && zLine[n-1]=='\n' && (!inQuote || !csvFlag) ){
      n--;
      if( n>0 && zLine[n-1]=='\r' ) n--;
      zLine[n] = 0;
      break;
    }
  }
  zLine = realloc( zLine, n+1 );
  return zLine;
}

static int strlen30(const char *z)
{
    const char *z2 = z;
    while (*z2) 
    {
        z2++;
    }
    return 0x3fffffff & (int)(z2-z);
}

static int import(const char *szTableName, const char *szImportFileName, const char *szRecordSeparator)
{
    const char *zFile = szImportFileName; /* The file from which to extract data */
    const char *zTable = szTableName;     /* Insert data into this table */
    sqlite3_stmt *pStmt = NULL;     /* A statement */
    int nCol;                       /* Number of columns in the table */
    int nByte;                      /* Number of bytes in an SQL string */
    int i, j;                       /* Loop counters */
    int nSep;                       /* Number of bytes in p->separator[] */
    char *zSql;                     /* An SQL statement */
    char *zLine;                    /* A single line of input from the file */
    char **azCol;                   /* zLine[] broken up into columns */
    char *zCommit;                  /* How to commit changes */   
    FILE *in;                       /* The input file */
    int lineno = 0;                 /* Line number of input file */
    const char *separator = szRecordSeparator;

    nSep = strlen30(separator);
    if( nSep==0 )
    {
        LOG("Error: non-null separator required for import.");
        return FAILED;
    }

    zSql = sqlite3_mprintf("SELECT * FROM %s", zTable);
    if( zSql==0 )
    {
        LOG("Error: out of memory.");
        return FAILED;
    }
    nByte = strlen30(zSql);
    int rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
    if( rc )
    {
        if (pStmt) 
        {
            sqlite3_finalize(pStmt);
        }
        LOG("Error: %s.", sqlite3_errmsg(db));
        return FAILED;
    }

    nCol = sqlite3_column_count(pStmt);
    sqlite3_finalize(pStmt);
    pStmt = 0;
    if( nCol==0 ) 
    {
        return SUCCESS; /* no columns, no error */
    }
    zSql = malloc( nByte + 20 + nCol*2 );
    if( zSql==0 )
    {
        LOG("Error: out of memory!.");
        return FAILED;
    }
    sqlite3_snprintf(nByte+20, zSql, "INSERT INTO %s VALUES(?", zTable);
    j = strlen30(zSql);
    for(i=1; i<nCol; i++)
    {
        zSql[j++] = ',';
        zSql[j++] = '?';
    }
    zSql[j++] = ')';
    zSql[j] = 0;
    rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
    free(zSql);
    if(rc)
    {
        LOG("Error: %s.", sqlite3_errmsg(db));
        if (pStmt)
        {
            sqlite3_finalize(pStmt);
        }
        return FAILED;
    }
    in = fopen(zFile, "rb");
    if( in==0 )
    {
        LOG("Error: cannot open \"%s\".", zFile);
        sqlite3_finalize(pStmt);
        return FAILED;
    }
    azCol = malloc(sizeof(azCol[0])*(nCol+1) );
    if( azCol==0 )
    {
        LOG("Error: out of memory.");
        fclose(in);
        sqlite3_finalize(pStmt);
        return FAILED;
    }

    //BEGIN IMMEDIATE avoid deadlock
    sqlite3_exec(db, "BEGIN IMMEDIATE", 0, 0, 0);
    zCommit = "COMMIT";

    while( (zLine = local_getline(0, in, 1))!=0 )
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
            if(!inQuote && c==separator[0] && strncmp(z,separator,nSep)==0)
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
            LOG("Error: %s line %d: expected %d columns of data bud found %d.",
                    zFile, lineno, nCol, i+1);
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

        sqlite3_step(pStmt);
        rc = sqlite3_reset(pStmt);
        free(zLine);
        if(rc!=SQLITE_OK)
        {
            LOG("Error:%s.", sqlite3_errmsg(db));
            zCommit = "ROLLBACK";
            rc = 1;
            break; /* from while */
        }
    } /* end while */

    free(azCol);
    fclose(in);
    sqlite3_finalize(pStmt);
    if (sqlite3_exec(db, zCommit, 0, 0, 0) != SQLITE_OK)
    {
        return FAILED;
    }
    if (strcmp(zCommit, "COMMIT") == 0)
    {
        return SUCCESS;
    }
    else
    {
        return FAILED;
    }
}

static int export(const char *szTableName, const char *szExportFileName, const char *szRecordSeparator)
{
    int nrow = 0;
    int ncolumn = 0;
    char **Result = 0;
    char *sql_select = sqlite3_mprintf("SELECT * FROM %q;", szTableName);
    sqlite3_get_table(db, sql_select, &Result, &nrow, &ncolumn, NULL);
    sqlite3_free(sql_select);

    FILE *fp = fopen(szExportFileName, "a");

    int i, j;
    for (i = 1; i < nrow+1; ++i)
    {
        for (j = 0; j < ncolumn; ++j)
        {
            fwrite(Result[i*ncolumn+j], strlen(Result[i*ncolumn+j]), 1, fp);
            if (ncolumn > 1 && j < ncolumn-1) 
            {
                fwrite(szRecordSeparator, strlen(szRecordSeparator), 1, fp);
            }
        }
        fwrite("\n", 1, 1, fp);
    }
    //int i, j;
    //for (i = 1; i < nrow+1; ++i)
    //{
    //    char buf[50] = {0};
    //    for (j = 0; j < ncolumn; ++j)
    //    {
    //        strcat(buf, szRecordSeparator);
    //        strcat(buf, Result[ncolumn*i+j]);
    //    }
    //    strcat(buf, "\n");
    //    fwrite(buf, sizeof(buf), 1, fp);
    //}
    sqlite3_free_table(Result);

    return SUCCESS;
}

static int query(const char *szTableName, char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    sqlite3_stmt *stmt_select = NULL;
    
    char *sql_select = sqlite3_mprintf("SELECT * FROM %Q where PlateNumber=%Q", szTableName, szPlateNumber);

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt_select, 0) != SQLITE_OK)
    {
        if (stmt_select)
        {
            sqlite3_finalize(stmt_select);
        }
        LOG("Sqlite3 prepare v2 error:%s", sqlite3_errmsg(db));
        return FAILED;
    }

    sqlite3_free(sql_select);

    int fieldCount = sqlite3_column_count(stmt_select);

    int ret = sqlite3_step(stmt_select);

    if (ret == SQLITE_DONE)
    {
        printf("%s is not in BlackList.\n", szPlateNumber);
        return 0;
    }
    else if (ret == SQLITE_ROW)
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

        pPlateRecord->szPlateNumber = szPlateNumber;
        pPlateRecord->PlateType = iTempPlateType;
        pPlateRecord->szCommentStr = (char *)sTempCommentStr;

        return 1;
    }
    else 
    {
        return FAILED;
    }
}

static int insert_record(const char *szTableName, PLATE_RECORD_T *pPlateRecord)
{
    char *sql_insert = sqlite3_mprintf("INSERT INTO %q VALUES('%q','%d','%q');", szTableName, pPlateRecord->szPlateNumber,
                                        pPlateRecord->PlateType, pPlateRecord->szCommentStr);
    int rc = exec_sql_not_select(sql_insert);

    sqlite3_free(sql_insert);

    return rc;
}

static int clear_record(const char *szTableName)
{

    char *sql_drop = sqlite3_mprintf("DROP TABLE %q;", szTableName);
    int rc = exec_sql_not_select(sql_drop);

    sqlite3_free(sql_drop);

    if (rc == FAILED)
    {
        return FAILED;
    }

    char *sql_create = sqlite3_mprintf("CREATE TABLE %q(PlateNumber TEXT PRIMARY KEY, PlateType INTEGER,\
                                            Comment TEXT);", szTableName);
    rc = exec_sql_not_select(sql_create);

    sqlite3_free(sql_create);

    return rc;
}

static int modify_record_comment(const char *szTableName, char *szPlateNumber, char *szCommentStr)
{
    char *sql_modify = sqlite3_mprintf("UPDATE %s set Comment='%q' where PlateNumber='%q';", szTableName, szCommentStr, szPlateNumber);

    int ret = exec_sql_not_select(sql_modify);

    sqlite3_free(sql_modify);

    return ret;
}

static int delete_record_by_plate_number(const char *szTableName, char *szPlateNumber)
{
    char *sql_delete = sqlite3_mprintf("DELETE FROM %q where PlateNumber='%q';", szTableName, szPlateNumber);

    int ret = exec_sql_not_select(sql_delete);

    sqlite3_free(sql_delete);

    return ret;
}

static int delete_records_by_plate_type(const char *szTableName, PLATE_TYPE ePlateType_t)
{
    char *sql_delete = sqlite3_mprintf("DELETE FROM %q where PlateType=%d;", szTableName, ePlateType_t);

    int ret = exec_sql_not_select(sql_delete);

    sqlite3_free(sql_delete);

    return ret;
}

