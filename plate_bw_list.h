#ifndef _PLATE_BW_LIST_H_
#define _PLATE_BW_LIST_H_

typedef enum {
    BLUE    = 1,
    YELLOW  = 2,
    WHITE   = 3,
    BLACK   = 4,
} PLATE_TYPE;

typedef struct {
    PLATE_TYPE PlateType_t;
    char *szPlateNumber;
    char *szCommentStr;
} PLATE_RECORD_T;


extern int bwl_init_database(const char *szDatabaseFilePath);

/* 黑名单导入 */
extern int bl_import(const char *szImportFileName, const char *szRecordSeparator);
extern int wl_import(const char *szImportFileName, const char *szRecordSeparator);

extern int bl_export(const char *szExportFileName, const char *szRecordSeparator);
extern int wl_export(const char *szExportFileName, const char *szRecordSeparator);

extern int bl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
extern int wl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);

extern int bl_insert_record(PLATE_RECORD_T *pPlateRecord);
extern int wl_insert_record(PLATE_RECORD_T *pPlateRecord);

extern int bl_delete_record_by_plate_number(char *szPlateNumber);
extern int wl_delete_record_by_plate_number(char *szPlateNumber);

extern int bl_delete_records_by_plate_type(PLATE_TYPE PlateType);
extern int wl_delete_records_by_plate_type(PLATE_TYPE PlateType);

extern int bl_modify_record_comment(char *szPlateNumber, char *szCommentStr);
extern int wl_modify_record_comment(char *szPlateNumber, char *szCommentStr);

extern int bl_clear_records(void);
extern int wl_clear_records(void);

#endif
