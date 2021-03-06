/*
 ******************************************************************************
 *                     SOURCE FILE
 *
 *     Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipcom_strtok_r.c,v $ $Revision: 1.7 $
 *     $Source: /home/interpeak/CVSRoot/ipcom/port/src/ipcom_strtok_r.c,v $
 *     $State: Exp $ $Locker:  $
 *
 *     INTERPEAK_COPYRIGHT_STRING
 *     Design and implementation by Firstname Lastname <email@interpeak.se>
 ******************************************************************************
 */

/*
 ****************************************************************************
 * 1                    DESCRIPTION
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 2                    CONFIGURATION
 ****************************************************************************
 */

#include "ipcom_config.h"


/*
 ****************************************************************************
 * 3                    INCLUDE FILES
 ****************************************************************************
 */

#define IPCOM_USE_CLIB_PROTO
#include "ipcom_clib.h"


/*
 ****************************************************************************
 * 4                    EXTERN PROTOTYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 5                    DEFINES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 6                    TYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 7                    LOCAL PROTOTYPES
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 8                    DATA
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 9                    LOCAL FUNCTIONS
 ****************************************************************************
 */

/*
 ****************************************************************************
 * 10                   PUBLIC FUNCTIONS
 ****************************************************************************
 */

/*
 *===========================================================================
 *                    ipcom_strtok_r
 *===========================================================================
 * Description:
 * Parameters:
 * Returns:
 */
#if defined(IPCOM_STRTOK_R) && IPCOM_STRTOK_R == 1
IP_PUBLIC char *
ipcom_strtok_r(char *s, const char *st, char **last)
{
    char *start;
    char *stop;
    
    start = s ? s : *last;
    while (*start != '\0' && ipcom_strchr(st, *start) != IP_NULL)
        start++;
    stop = start;
    while (*stop != '\0' && ipcom_strchr(st, *stop) == IP_NULL)
        stop++;
    if (*stop != '\0')
        *stop++ = '\0';
    *last = stop;
    
    return ipcom_strlen(start) ? start : IP_NULL;
}

#else
int ipcom_strtok_r_empty_file;
#endif


/*
 ****************************************************************************
 *                      END OF FILE
 ****************************************************************************
 */
