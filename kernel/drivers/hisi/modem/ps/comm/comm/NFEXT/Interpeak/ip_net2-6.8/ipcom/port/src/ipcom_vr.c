/*
 ******************************************************************************
 *                     SOURCE FILE
 *
 *     Document no: @(#) $Name: VXWORKS_ITER18A_FRZ10 $ $RCSfile: ipcom_vr.c,v $ $Revision: 1.3 $
 *     $Source: /home/interpeak/CVSRoot/ipcom/port/src/ipcom_vr.c,v $
 *     $State: Exp $ $Locker:  $
 *
 *     INTERPEAK_COPYRIGHT_STRING
 *     Design and implementation by Lennart Bang <lob@interpeak.se>
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

#ifndef IP_PORT_LKM

#define IPCOM_USE_CLIB_PROTO
#include "ipcom_os.h"
#include "ipcom_clib.h"

#if defined(IP_PORT_OSE) || defined(IP_PORT_OSE5)
#include <ose.h>
#endif


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

#if !defined(IP_PORT_LAS)
/*
 *===========================================================================
 *                         ipcom_proc_vr_get
 *===========================================================================
 * Description:  Get VR id for this process.
 * Parameters:   -
 * Returns:      VR
 */
IP_PUBLIC int
ipcom_proc_vr_get(void)
{
    return ipcom_getenv_as_int("VR", 0);
}
#endif


#if !defined(IP_PORT_LAS)
/*
 *===========================================================================
 *                         ipcom_proc_vr_set
 *===========================================================================
 * Description:  Set VR id for this process.
 * Parameters:   VR id
 * Returns:      -
 */
IP_PUBLIC void
ipcom_proc_vr_set(int vr)
{
    (void)ipcom_setenv_as_int("VR", vr, 1);
}
#endif

/*
 *===========================================================================
 *                         ipcom_proc_vr_get_for
 *===========================================================================
 * Description:  Get VR id for specified process.
 * Parameters:   Process ID.
 * Returns:      VR
 */

#if defined(IP_PORT_OSE) || defined(IP_PORT_OSE5)
IP_PUBLIC int
ipcom_proc_vr_get_for(Ip_pid_t pid)
{
#if defined(IP_PORT_OSE) || defined(IP_PORT_OSE5)
    char *env;
    int   vr = 0;

    env = get_env((PROCESS)pid, "VR");
    if (env != IP_NULL)
    {
        vr = ipcom_atoi(env);
        free_buf((union SIGNAL **)&env);
    }
    return vr;
#else
    IP_PANIC();
    return 0;
#endif
}
#endif


#else
int ipcom_vr_empty_file;
#endif /*! IP_PORT_LKM */


/*
 ****************************************************************************
 *                      END OF FILE
 ****************************************************************************
 */

