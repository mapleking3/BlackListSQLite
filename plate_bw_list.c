#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "plate_bw_list.h"

#define FAILED  -1
#define SUCCESS 0

#define LOG(format, ...)                                            \
        do {printf("%s in %s Line[%d]:"format"\n",                   \
                __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);   \
        } while(0)

/*local static variables */
static sqlite3 *db;
static const char *szBlackListTable = "BlackList";
static const char *szWhiteListTable = "WhiteList";

static int import(const char *szTableName, const char *szImportFileName, const char *szRecordSeparator);
static int export(const char *szTableName, const char *szExportFileName, const char *szRecordSeparator);
static int query(const char *szTableName, const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
static int insert_record(const char *szTableName, PLATE_RECORD_T *pPlateRecord);
static int clear_record(const char *szTableName);
static int modify_record_comment(const char *szTableName, const char *szPlateNumber, const char *szCommentStr);
static int delete_record_by_plate_number(const char *szTableName, const char *szPlateNumber);
static int delete_records_by_plate_type(const char *szTableName, PLATE_TYPE PlateType);

int bwl_init_database(const char *szDatabaseFilePath)
{
    if (SQLITE_OK != sqlite3_config(SQLITE_CONFIG_SERIALIZED))
    {
        LOG("Configure Sqlite3 error:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    if (SQLITE_OK != sqlite3_open(szDatabaseFilePath, &db))
    {
        LOG("Can't open database:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    if (SQLITE_OK != sqlite3_exec(db, "PRAGMA page_size=4096;", 0, 0, NULL))
    {
        LOG("Can't set page_size:%s.", sqlite3_errmsg(db));
        goto ErrReturn;
    }

    if (SQLITE_OK != sqlite3_exec(db, "PRAGMA cache_size=8000;", 0, 0, NULL))
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

    return SUCCESS;

ErrReturn:
    sqlite3_close(db);
    return FAILED;
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

        //if(rc!=SQLITE_OK)
        //SQLITE_CONSTRAINT: abort due to constraint violation
        if (SQLITE_OK != rc && SQLITE_CONSTRAINT != rc)
        {
            LOG("Error:%s", sqlite3_errmsg(db));
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
    FILE *fp = fopen(szExportFileName, "a");

    if (NULL == fp)
    {
        LOG("Open export file error:%s", strerror(errno));
        return FAILED;
    }

    char *sql_select = sqlite3_mprintf("SELECT * FROM %q;", szTableName);

    sqlite3_stmt *stmt = NULL;

    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL))
    {
        if (NULL != stmt)
        {
            sqlite3_finalize(stmt);
        }
        return FAILED;
    }

    int fieldCount = sqlite3_column_count(stmt);
    const int LENGTH = 20;
    int currField = 0;
    char *szTemp = NULL;
    char *temp = NULL;
    int len;

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
                        strcat(str, szTemp);
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
                    len = strlen(str);
                    str[len] = ';';
                    str[len+1] = 0;
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
    stmt = NULL;
    fclose(fp);
    return SUCCESS;

ErrReturn:
    sqlite3_finalize(stmt);
    stmt = NULL;
    fclose(fp);
    return FAILED;
}

static int query(const char *szTableName, const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
{
    sqlite3_stmt *stmt_select = NULL;
    
    char *sql_select = sqlite3_mprintf("SELECT * FROM %q where PlateNumber=%Q;", szTableName, szPlateNumber);

    if (SQLITE_OK != sqlite3_prepare_v2(db, sql_select, -1, &stmt_select, 0))
    {
        if (NULL != stmt_select)
        {
            sqlite3_finalize(stmt_select);
        }
        LOG("Sqlite3 prepare v2 error:%s", sqlite3_errmsg(db));
        sqlite3_free(sql_select);
        return FAILED;
    }

    sqlite3_free(sql_select);

    int fieldCount = sqlite3_column_count(stmt_select);

    int ret = sqlite3_step(stmt_select);

    if (SQLITE_DONE == ret)
    {
        printf("%s is not in BlackList.\n", szPlateNumber);
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

            pPlateRecord->szPlateNumber = (char *)szPlateNumber;
            pPlateRecord->PlateType = iTempPlateType;
            pPlateRecord->szCommentStr = (char *)sTempCommentStr;
        }
        return 1;
    }
    else 
    {
        return FAILED;
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

static int modify_record_comment(const char *szTableName, const char *szPlateNumber, const char *szCommentStr)
{
    char *sql_modify = sqlite3_mprintf("UPDATE %q set Comment=%Q where PlateNumber=%Q;", 
                                        szTableName, szCommentStr, szPlateNumber);

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
