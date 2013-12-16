/** 
 * @filename:   plate_bw_list.h
 * @brief:      plate blacklist and whitelist module header file
 * @author:     Retton
 * @version:    V1.0.0
 * @date:       2013-05-17
 * Copyright:   2012-2038 Anhui CHAOYUAN Info Technology Co.Ltd
 */
#ifndef _PLATE_BW_LIST_H_
#define _PLATE_BW_LIST_H_

#define MAX_PLATE_NUMBER    16
#define MAX_COMMENT_LENGTH  128

#define BWLIST_ERROR        -1
#define BWLIST_OK           0

/** 
 * @name:   PLATE_TYPE
 * @brief:  plate type differnce in plate color
 */
typedef enum {
    BLUE    = 1,
    YELLOW  = 2,
    WHITE   = 3,
    BLACK   = 4,
    GREEN   = 5,
    UNKNOW  = 6,
} PLATE_TYPE;

/** 
 * @name:   PLATE_RECORD_T
 * @brief:  plate record contain plate color, plate number and comment string
 */
typedef struct {
    PLATE_TYPE PlateType;
    char szPlateNumber[MAX_PLATE_NUMBER];
    char szCommentStr[MAX_COMMENT_LENGTH];
} PLATE_RECORD_T;

/** 
 * @fn:     extern int bwl_init_database(const char *szDatabaseFilePath);
 * @brief:  initialize blacklist and whitelist database
 * @param:  szDatabaseFilePath
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bwl_init_database(const char *szDatabaseFilePath);

/** 
 * @fn:     extern int bwl_backup_database(const char *szBackupDBFilePath);
 * @brief:  backup blacklist and whitelist database
 * @param:  szBackupDBFilePath
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bwl_backup_database(const char *szBackupDBFilePath);

/** 
 * @fn:     extern int bwl_close_database(void);
 * @brief:  close database connection and release res
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bwl_close_database(void);

/** 
 * @fn:     extern int bl_import(const char *szImportFileName, const char *szRecordSeparator);
 * @brief:  import records to blacklist from specified file
 * @param:  szImportFileName: import text file name
 * @param:  szRecordSeparator: separator of each record in import text file
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_import(const char *szImportFileName, const char *szRecordSeparator);

/** 
 * @fn:     extern int wl_import(const char *szImportFileName, const char *szRecordSeparator);
 * @brief:  import records to whitelist from specified file
 * @param:  szImportFileName: import text file name
 * @param:  szRecordSeparator: separator of each record in import text file
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_import(const char *szImportFileName, const char *szRecordSeparator);

/** 
 * @fn:     extern int bl_export(const char *szExportFileName, const char *szRecordSeparator);
 * @brief:  export blacklist records to specified file
 * @param:  szExportFileName: export file name
 * @param:  szRecordSeparator: separator in a record
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_export(const char *szExportFileName, const char *szRecordSeparator);

/** 
 * @fn:     extern int wl_export(const char *szExportFileName, const char *szRecordSeparator);
 * @brief:  export whitelist records to specified file
 * @param:  szExportFileName: export file name
 * @param:  szRecordSeparator: separator in a record
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_export(const char *szExportFileName, const char *szRecordSeparator);

#if 0
/** 
 * @fn:     extern int bl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
 * @brief:  query record whether in blacklist or not by plate number
 * @param:  szPlateNumber: plate number
 * @param:  pPlateRecord: pointer to plate record
 * @return: BWLIST_ERROR:error 0:not in 1:in 
 */
extern int bl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
#endif

/** 
 * @fn:     extern int bl_query(const char *szPlateNumber);
 * @brief:  put recognized plate number to plate buffer
 * @param:  szPlateNumber: plate number
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_query(const char *szPlateNumber);

/** 
 * @fn:     extern int wl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);
 * @brief:  query record wheter in whitelist or not by plate number
 * @param:  szPlateNumber: plate number
 * @param:  pPlateRecord: pointer to plate record
 * @return: BWLIST_ERROR:failed 0:not in 1:in
 */
extern int wl_query(const char *szPlateNumber, PLATE_RECORD_T *pPlateRecord);

/** 
 * @fn:     extern int bl_insert_record(PLATE_RECORD_T *pPlateRecord);
 * @brief:  insert one record to blacklist
 * @param:  pPlateRecord: pointer to a one plate record
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_insert_record(PLATE_RECORD_T *pPlateRecord);

/** 
 * @fn:     extern int wl_insert_record(PLATE_RECORD_T *pPlateRecord);
 * @brief:  insert one record to whitelist
 * @param:  pPlateRecord: pointer to a none plate record
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_insert_record(PLATE_RECORD_T *pPlateRecord);

/** 
 * @fn:     extern int bl_delete_record_by_plate_number(const char *szPlateNumber);
 * @brief:  delete a record in blacklist by platenumber
 * @param:  szPlateNumber: platenumber
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_delete_record_by_plate_number(const char *szPlateNumber);

/** 
 * @fn:     extern int wl_delete_record_by_plate_number(const char *szPlateNumber);
 * @brief:  delete a record in whitelist by platenumber
 * @param:  szPlateNumber: platenumber
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_delete_record_by_plate_number(const char *szPlateNumber);

/** 
 * @fn:     extern int bl_delete_records_by_plate_type(PLATE_TYPE PlateType);
 * @brief:  delete records in blacklist by plate color
 * @param:  PlateType: plate color type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_delete_records_by_plate_type(PLATE_TYPE PlateType);

/** 
 * @fn:     extern int wl_delete_records_by_plate_type(PLATE_TYPE PlateType);
 * @brief:  delete records in whitelist by plate color
 * @param:  PlateType: plate color type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_delete_records_by_plate_type(PLATE_TYPE PlateType);

/** 
 * @fn:     extern int bl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr);
 * @brief:  modify a record by plate number
 * @param:  szPlateNumber: plate number
 * @param:  PlateType: plate type
 * @param:  szCommentStr: comment string
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr);

/** 
 * @fn:     extern int wl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr);
 * @brief:  modify a record by plate number
 * @param:  szPlateNumber: plate number
 * @param:  PlateType: plate type
 * @param:  szCommentStr: comment string
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_modify_record_by_plate_number(const char *szPlateNumber, PLATE_TYPE PlateType, const char *szCommentStr);

/** 
 * @fn:     extern int bl_clear_records(void);
 * @brief:  clear all records in blacklist
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_clear_records(void);

/** 
 * @fn:     extern int wl_clear_records(void);
 * @brief:  clear all records in whitelist
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_clear_records(void);

#endif
