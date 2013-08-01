#ifndef _PLATE_BW_LIST_H_
#define _PLATE_BW_LIST_H_

#define MAX_PLATE_NUMBER_LENGTH 16

/**@struct  PLATE_TYPE
 *@brief    Author/Data Retton/20130513
 *@brief    plate type based on plate color
 */
typedef enum {
    BLUE    = 1,
    YELLOW  = 2,
    WHITE   = 3,
    BLACK   = 4,
} PLATE_TYPE;

/**@struct  PLATE_RECORD_T
 *@brief    Author/Data Retton/20130513
 *@brief    plate record contain plate type, plate number, comment string
 */
typedef struct {
    PLATE_TYPE PlateType;
    char szPlateNumber[MAX_PLATE_NUMBER_LENGTH];
    char *szCommentStr;
} PLATE_RECORD_T;

/**@fn      int bwl_init_database(const char *szDatabaseFilePath)
 *@brief    initialize blacklist and whitelist database
 *@brief    Author/Data Retton/20130513
 *@param    [in]szDatabaseFilePath: database file path
 *@return   -1:success 0:failed
 */
extern int bwl_init_database(const char *szDatabaseFilePath);

/**@fn      int bl_import(const char *szImportFileName, const char *szRecordSeparator)
 *@brief    import records to blacklist from sepcified file
 *@brief    Author/Data Retton/20130513
 *@param    [in]szImportFileName: import file pathname
 *@param    [in]szRecordSeparator: separator in a record
 *@return   -1:success 0:failed
 */
extern int bl_import(const char *szImportFileName, const char *szRecordSeparator);

/**@fn      int wl_import(const char *szImportFileName, const char *szRecordSeparator)
 *@brief    import records to whitelist from specified file
 *@brief    Author/Data Retton/20130513
 *@param    [in]szImportFileName: import file pathname
 *@param    [in]szRecordSeparator: separator in a record
 *@return   -1:success 0:failed
 */
extern int wl_import(const char *szImportFileName, const char *szRecordSeparator);

/**@fn      int bl_export(const char *szExportFileName, const char *szRecordSeparator)
 *@brief    export records from blacklist to specified file
 *@brief    Author/Data Retton/20130513
 *@param    [in]szExportFileName: export file pathname
 *@param    [in]szRecordSeparator: separator in a record
 *@return   -1:success 0:failed
 */
extern int bl_export(const char *szExportFileName, const char *szRecordSeparator);

/**@fn      int wl_export(const char *szExportFileName, const char *szRecordSeparator)
 *@brief    export records from whitelist to specified file
 *@brief    Author/Data Retton/20130513
 *@param    [in]szExportFileName: export file pathname
 *@param    [in]szRecordSeparator: separator in a record
 *@return   -1:success 0:failed
 */
extern int wl_export(const char *szExportFileName, const char *szRecordSeparator);

/**@fn      int bl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
 *@brief    query record in blacklist by Plate number or not
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: Plate Number
 *@param    [in]pPlateRecord: pointer to plate record
 *@return   1:in blacklist 0: not in blacklist -1: error 
 */
extern int bl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);

/**@fn      int wl_query(char *szPlateNumber, PLATE_RECORD_T *pPlateRecord)
 *@brief    query record in whitelist by Plate number or not
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: Plate Number
 *@param    [in]pPlateRecord: pointer to plate record
 *@return   1:in whitelist 0: not in blacklist -1: error 
 */
extern int wl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);

/**@fn      int bl_insert_record(PLATE_RECORD_T *pPlateRecord);
 *@brief    insert a record to blacklist
 *@brief    Author/Data Retton/20130513
 *@param    [in]pPlateRecord: pointer to plate record
 *@return   -1:success 0:failed
 */
extern int bl_insert_record(PLATE_RECORD_T *pPlateRecord);

/**@fn      int wl_insert_record(PLATE_RECORD_T *pPlateRecord);
 *@brief    insert a record to whitelist
 *@brief    Author/Data Retton/20130513
 *@param    [in]pPlateRecord: pointer to plate record
 *@return   -1:success 0:failed
 */
extern int wl_insert_record(PLATE_RECORD_T *pPlateRecord);

/**@fn      int bl_delete_record_by_plate_number(char *szPlateNumber)
 *@brief    delete record in blacklist by plate number 
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: plate number
 *@return   -1:success 0:failed
 */
extern int bl_delete_record_by_plate_number(const char *szPlateNumber);

/**@fn      int bl_delete_record_by_plate_number(char *szPlateNumber)
 *@brief    delete record in blacklist by plate number 
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: plate number
 *@return   -1:success 0:failed
 */
extern int wl_delete_record_by_plate_number(const char *szPlateNumber);

/**@fn      int bl_delete_records_by_plate_type(PLATE_TYPE PlateType)
 *@brief    delete records in blacklist by plate color
 *@brief    Author/Data Retton/20130513
 *@param    [in]PlateType: plate color
 *@return   -1:success 0:failed
 */
extern int bl_delete_records_by_plate_type(PLATE_TYPE PlateType);

/**@fn      int wl_delete_records_by_plate_type(PLATE_TYPE PlateType)
 *@brief    delete records in whitelist by plate color
 *@brief    Author/Data Retton/20130513
 *@param    [in]PlateType: plate color
 *@return   -1:success 0:failed
 */
extern int wl_delete_records_by_plate_type(PLATE_TYPE PlateType);

/**@fn      int bl_modify_record_comment(char *szPlateNumber, char *szCommentStr)
 *@brief    modify record comment by plate number
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: plate number
 *@param    [in]szCommentStr: comment string
 *@return   -1:success 0:failed
 */
extern int bl_modify_record_comment(const char *szPlateNumber, const char *szCommentStr);

/**@fn      int wl_modify_record_comment(char *szPlateNumber, char *szCommentStr)
 *@brief    modify record comment by plate number
 *@brief    Author/Data Retton/20130513
 *@param    [in]szPlateNumber: plate number
 *@param    [in]szCommentStr: comment string
 *@return   -1:success 0:failed
 */
extern int wl_modify_record_comment(const char *szPlateNumber, const char *szCommentStr);

/**@fn      int bl_clear_records(void)
 *@brief    clear all record in blacklist
 *@brief    Author/Data Retton/20130513
 *@param    [in]void
 *@return   -1:success 0:failed
 */
extern int bl_clear_records(void);

/**@fn      int bl_clear_records(void)
 *@brief    clear all record in whitelist
 *@brief    Author/Data Retton/20130513
 *@param    [in]void
 *@return   -1:success 0:failed
 */
extern int wl_clear_records(void);

#endif
