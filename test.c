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

#define SUCCESS 0
#define FAILED -1

static char *local_getline(char *zPrompt, FILE *in, int csvFlag)
{
  char *zLine;
  int nLine;
  int n;
  int inQuote = 0;

  if( zPrompt && *zPrompt )
  {
        printf("%s",zPrompt);
        fflush(stdout);
  }

  nLine = 100;
  zLine = malloc(nLine);

  if(zLine==0) 
  {
      return SUCCESS;
  }

  n = 0;

  for(;;)
  {
        if(n+100 > nLine)
        {
            nLine = nLine*2 + 100;
            zLine = realloc(zLine, nLine);
            if(0 == zLine) 
            {
                return SUCCESS;
            }
        }

        if(0 == fgets(&zLine[n], nLine-n, in))
        {
            if(0 == n)
            {
                free(zLine);
                return SUCCESS;
            }
            zLine[n] = 0;
            break;
        }

        while(zLine[n])
        {
          if('"' == zLine[n]) 
          {
              inQuote = !inQuote;
          }
          n++;
        }
        if(n>0 && '\n'==zLine[n-1] && (!inQuote || !csvFlag))
        {
          n--;
          if(n>0 && '\r'==zLine[n-1]) 
          {
              n--;
          }
          zLine[n] = 0;
          break;
        }
  }

  zLine = realloc( zLine, n+1 );
  return zLine;
}

int linear_search(const char *szImportFileName, const char *szPlateNumber, const char *szRecordSeparator)
{
    FILE *fp = fopen(szImportFileName, "r");

    char *zLine;

    if (NULL == fp)
    {
        LOG("Open FIle failed:%s.", strerror(errno));
        return -1;
    }

    int lineno = 0;
    int nSep = strlen(szRecordSeparator);
    while ((zLine = local_getline(0, fp, 1)) != 0)
    {
        char *p = zLine;
        char c;
        lineno++;

        unsigned int i=0;
        for (i = 0; i < strlen(zLine); ++i)
        {
            c = *p;
            if (c==szRecordSeparator[0] && strncmp(p, szRecordSeparator, nSep)==0)
            {
                *p = 0;
                break;
            }
            ++p;
        }

        p = zLine;

        int inQuote = 0;
        char *sPlate = NULL;
        for (i = 0; i < strlen(zLine); ++i)
        {
            if (*p == '"')
            {
                if (0 == inQuote) 
                {
                    inQuote = 1;
                    sPlate = p+1;
                }
                else 
                {
                    *p = 0;
                }
            }
            ++p;
        }



        if (strcmp(sPlate, szPlateNumber) == 0)
        {
            printf("Find it!\n");
            return 1;
        }
    }
    return FAILED;
}

int main(int argc, char *argv[])
{
    if (-1 == bwl_init_database("./test.db"))
    {
        LOG("Init error!");
        return 0;
    }


    if (argc >= 2)
    {
        const char *szImportFileName = argv[1];

        STATICS_START("IMPORT");
        if (-1 == bl_import(szImportFileName,";"))
        {
            LOG("import blacklist error!");
        }
        STATICS_STOP();
    }

    const char *PlateStart          = "皖A-aaaaa";
    const char *PlateQuarter        = "皖A-bbbbb";
    const char *PlateAHalf          = "皖A-ccccc";
    const char *PlateThreeQuarter   = "皖A-ddddd";
    const char *PlateEnd            = "皖A-eeeee";
    const char *PlateNotIn          = "皖不存在";

    int cnt = 0;

    while (1)
    {
        printf("Split Line^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        if (bl_query(PlateNotIn, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateNotIn);
        }

        if (bl_query(PlateStart, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateStart);
        }

        if (bl_query(PlateQuarter, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateQuarter);
        }

        if (bl_query(PlateAHalf, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateAHalf);
        }

        if (bl_query(PlateThreeQuarter, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateThreeQuarter);
        }

        if (bl_query(PlateEnd, NULL) == 1)
        {
            printf("Find %s In\n\n", PlateEnd);
        }

#if 0
        STATICS_START("Query");
        if (bl_query(PlateNotIn, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateNotIn);
        }
        STATICS_STOP();

        STATICS_START("Query Begin");
        if (bl_query(PlateStart, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateStart);
        }
        STATICS_STOP();

        STATICS_START("Query Quarter");
        if (bl_query(PlateQuarter, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateQuarter);
        }
        STATICS_STOP();

        STATICS_START("Query A Half");
        if (bl_query(PlateAHalf, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateAHalf);
        }
        STATICS_STOP();

        STATICS_START("Query Three Quarter");
        if (bl_query(PlateThreeQuarter, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateThreeQuarter);
        }
        STATICS_STOP();

        STATICS_START("Query End");
        if (bl_query(PlateEnd, NULL) == 1)
        {
            printf("\n\nFind %s In\n", PlateEnd);
        }
        STATICS_STOP();
#endif

        //sleep(2);
        if (++cnt == 5)
        {
            break;
        }
    }

#if 0
    STATICS_START("Delete one record");
    bl_delete_record_by_plate_number(PlateStart);
    STATICS_STOP();

    STATICS_START("CLEAR");
    bl_clear_records();
    STATICS_STOP();
#endif

    //char *szPlateNumber = "皖A-11111";

    //PLATE_RECORD_T PlateRecord = {
    //    .PlateType = BLUE,
    //    .szPlateNumber = szPlateNumber,
    //    .szCommentStr = "hello insert",
    //};
    //PLATE_RECORD_T *pPlateRecord = &PlateRecord;
    //
    //if (-1 == bl_insert_record(pPlateRecord))
    //{
    //    LOG("Insert record error!");
    //}
    //else 
    //{
    //    LOG("Insert record success!");
    //}

    //STATICS_START("Query");
    //int ret = bl_query(szPlateNumber, pPlateRecord);

    //if (ret == 0)
    //{
    //    LOG("Plate:%s not found!", szPlateNumber);
    //}
    //else if (ret == 1)
    //{
    //    LOG("Plate:%s is in blacklist!", szPlateNumber);
    //    LOG("PlateNumber:%s<***>PlateType:%d<***>Comment:%s",
    //            pPlateRecord->szPlateNumber,
    //            pPlateRecord->PlateType,
    //            pPlateRecord->szCommentStr);
    //}
    //else 
    //{
    //    LOG("Query error!");
    //    return -1;
    //}
    //STATICS_STOP();

    //char *szPlateNumber_1 = "皖A-53333";
    //if (-1 == bl_delete_record_by_plate_number(szPlateNumber_1))
    //{
    //    LOG("delete record error!");
    //}
    //else 
    //{
    //    LOG("delete record success!");
    //}

    //char *szPlateNumber_2 = "皖A-43333";
    //char *szComment = "Modify comment str";
    //if (-1 == bl_modify_record_comment(szPlateNumber_2, szComment))
    //{
    //    LOG("modify comment failed!");
    //}
    //else
    //{
    //    LOG("modify comment success!");
    //}

    return 0;
}

