/** 
 * @filename:   bwlist.h
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
 * @name:   PLATE_COLOR_E
 * @brief:  plate type differnce in plate color
 */
typedef enum {
    EPLATE_BLUE    = 1,
    EPLATE_YELLOW  = 2,
    EPLATE_WHITE   = 3,
    EPLATE_BLACK   = 4,
    EPLATE_GREEN   = 5,
    EPLATE_UNKNOW  = 6,
} PLATE_COLOR_E;

/** 
 * @name:   SUSPICION_CAR_SRC_E
 * @brief:  嫌疑车辆来源
 */
typedef enum {
    ENOT_BLIST_DATA = 0,        ///< 非黑名单数据
    ETEMP_INSPECT_DATA,         ///< 临时布控数据
    EVEHICLE_ADMIN_OFFICE_DATA, ///< 车管所数据
    EIILEGAL_TRAFFFICE_DATA,    ///< 交通违法数据
    EMPS_BLIST_DATA,            ///< 公安部黑名单数据
    EUNKNOW_SRC_DATA,           ///< 未知来源
} SUSPICION_CAR_SRC_E;

/** 
 * @name:   SUSPICION_TYPE_E
 * @brief:  嫌疑类别
 */
typedef enum {
    EDECK_VEHICLE = 0,          ///< 套牌车辆
    EOFFENCE_NO_HANDLE,         ///< 违法未处理
    ESTEAL_OR_ROB_VEHICLE,      ///< 盗抢车辆
    ENOT_YEARLY_CHECK_VEHICLE,  ///< 超期未年检
    EHIT_AND_RUN_VEHICLE,       ///< 肇事逃逸车辆
    EEND_LIFE_VEHICLE,          ///< 报废车辆
    ESUSPICION_VEHICLE,         ///< 嫌疑车辆
} SUSPICION_TYPE_E;

/** 
 * @name:   PLATE_RECORD_T
 * @brief:  车牌记录
 */
typedef struct {
    PLATE_COLOR_E ePlateColor;
    SUSPICION_TYPE_E eSuspicionType;
    char szPlateNumber[MAX_PLATE_NUMBER];
} PLATE_RECORD_T;

/** 
 * @name:   pf_handle_inspected
 * @brief:  callback function called when plate in blacklist
 */
typedef void (*pf_handle_inspected)(PLATE_RECORD_T *pPlateRecord, const char
        *szFileName);

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
 *          judge columns by szRecordSeparator
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
 * @fn:     extern int bl_query(const char *szPlateNumber, const char *szJpgName);
 * @brief:  put recognized plate number to plate buffer
 * @param:  szPlateNumber: plate number
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_query(const char *szPlateNumber, const char *szJpgName);

/** 
 * @fn:     extern void bl_set_handle_callback(pf_handle_inspected
            pfHandleInspected);
 * @brief:  set black list data handle callback function
 * @brief:  Author/Date: retton/2013-12-21
 * @param:  pfHandleInspected
 * @return: 
 */
extern void bl_set_handle_callback(pf_handle_inspected pfHandleInspected);

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
 * @fn:     extern int bl_delete_records_by_plate_type(PLATE_COLOR_E ePlateColor);
 * @brief:  delete records in blacklist by plate color
 * @param:  ePlateColor: plate color type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_delete_records_by_plate_type(PLATE_COLOR_E PlateType);

/** 
 * @fn:     extern int wl_delete_records_by_plate_type(PLATE_COLOR_E ePlateColor);
 * @brief:  delete records in whitelist by plate color
 * @param:  ePlateColor: plate color type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_delete_records_by_plate_type(PLATE_COLOR_E ePlateColor);

/** 
 * @fn:     extern int bl_modify_record_by_plate_number(
 *              const char *szPlateNumber, PLATE_COLOR_E PlateType, 
 *              SUSPICION_TYPE_E eSuspicionType);
 * @brief:  modify a record by plate number
 * @param:  szPlateNumber: plate number
 * @param:  PlateType: plate type
 * @param:  eSuspicionType: suspicion type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int bl_modify_record_by_plate_number(const char *szPlateNumber, 
        PLATE_COLOR_E PlateType, SUSPICION_TYPE_E eSuspicionType);

/** 
 * @fn:     extern int wl_modify_record_by_plate_number(
 *              const char *szPlateNumber, PLATE_COLOR_E PlateType, 
 *              SUSPICION_TYPE_E eSuspicionType);
 * @brief:  modify a record by plate number
 * @param:  szPlateNumber: plate number
 * @param:  PlateType: plate type
 * @param:  eSuspicionType: suspicion type
 * @return: BWLIST_ERROR:failed BWLIST_OK:success
 */
extern int wl_modify_record_by_plate_number(const char *szPlateNumber, 
        PLATE_COLOR_E PlateType, SUSPICION_TYPE_E eSuspicionType);

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

/** 
 * @fn:     extern int get_import_percent(void);
 * @brief:  get import handle percent
 * @brief:  Author/Date: retton/2014-01-09
 * @return: import handle percent
 */
extern int get_import_percent(void);

#endif
