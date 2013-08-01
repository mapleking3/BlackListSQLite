#ifndef _COMMON_H_
#define _COMMON_H_
#include <sys/time.h>

/**@macro   LOG(format, ...)
 *@brief    printf message
 *@brief    Author/Data Retton/20130513
 */
#define LOG(format, ...)                                            \
        do{printf("%s in %s Line[%d]:"format"\n",                   \
                __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__);   \
        } while(0)


/**@macro   
 *@brief    caculate time spend
 *@brief    Author/Data Retton/20130513
 */
#if 0
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
        printf("%s : %ldms\n", statics_name,            \
               (end.tv_sec  - start.tv_sec)  * 1000 +   \
               (end.tv_usec - start.tv_usec) / 1000);   \
        }

#endif

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
        printf("%s : [%ldms] [%ldus]\n",    \
                statics_name,                                               \
                (end.tv_sec - start.tv_sec) * 1000 +                        \
                (end.tv_usec - start.tv_usec) / 1000,                       \
                (end.tv_sec - start.tv_sec) * 1000 * 1000 +                 \
                (end.tv_usec - start.tv_usec));                             \
        }

#endif //_COMMON_H_
