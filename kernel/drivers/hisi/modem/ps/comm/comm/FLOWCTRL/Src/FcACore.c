/******************************************************************************

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : FcACore.c
  �� �� ��   : ����
  ��������   : 2011��12��21��
  ����޸�   :
  ��������   : ACPU�����ش����������ڴ桢CPU���ء���ӦCDS��GPRS��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2011��12��21��
    �޸�����   : �����ļ�

******************************************************************************/



/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "Fc.h"
#include "FcInterface.h"
#include "FcCdsInterface.h"
#include "FcCstInterface.h"
#include "AtFcInterface.h"
#include "AdsFcInterface.h"
#include "ACpuLoadInterface.h"
#include "ImmInterface.h"
#include "NFExtInterface.h"
#include "FcIntraMsg.h"
#include "PsTypeDef.h"
#include "pslog.h"
#include "PsCommonDef.h"
#include "PsLib.h"
#include "TTFTaskPriority.h"
#include "NVIM_Interface.h"
#include "FcACoreCReset.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_ACORE_FLOW_CTRL_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/* ��������ͳ��ʹ�õı��� */
FC_BRIDGE_RATE_STRU                     g_stFcBridgeRate;

/* CDS����ʹ�õ�ȫ�ֱ�����FC IdRABӳ���ϵ */

FC_RAB_MAPPING_INFO_SET_STRU            g_astFcRabMappingInfoSet[MODEM_ID_BUTT];

/* CPU����ʹ�ã�����ƽ����������ʱ�� */
FC_CPU_CTRL_STRU                        g_stFcCpuACtrl;

FC_CPU_DRV_ASSEM_CONFIG_PARA_STRU       g_stCpuDriverAssePara =
{
    6,
    10,
    5,
    0,
    {
    /* cpuload, PC�������ʱ�ӣ�UE����TX�������, UE����TXʱ�ӣ�UE����RX���������UE����RX���ʱ�ӣ������� */
        {
            85,{20, 40, 10, 10, 10, 56, {0, 0}}
        },
        {
            70,{15, 20, 10, 10, 10, 20, {0, 0}}
        },
        {
            60,{10, 10, 10, 10, 10, 5, {0, 0}}
        },
        {
            40,{5,  5,  10, 10, 10, 1, {0, 0}}
        },
        {
            0, {0,  1,  1,  1,  1, 1, {0, 0}}
        }
    }
};

/* �����������ʵ��ṹ */
FC_CPU_DRV_ASSEM_PARA_ENTITY_STRU  g_stDrvAssemParaEntity = {0, 0, 0, 0, 0, {100, {0, 0, 0, 0, 0, 0, {0, 0}}}, VOS_NULL_PTR, VOS_NULL_PTR};

VOS_SPINLOCK                       g_stFcMemSpinLock;

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/
#if(FEATURE_ON == FEATURE_NFEXT)
/*****************************************************************************
 �� �� ��  : FC_CalcBridgeRate
 ��������  : ���������ϵ�����,��λ:bps
 �������  : ulPeriod -- ��������ͳ�����ڣ���λ����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_BRIDGE_CalcRate( VOS_UINT32 ulPeriod )
{
    VOS_UINT32                          ulCurrentByteCnt;
    VOS_UINT32                          ulLastByteCnt;
    VOS_UINT32                          ulRate;
    VOS_UINT32                          ulDeltaPacketCnt;


    if (0 == ulPeriod)
    {
        g_stFcBridgeRate.ulRate = 0;
        return;
    }

    ulLastByteCnt       = g_stFcBridgeRate.ulLastByteCnt;
    ulCurrentByteCnt    = NFExt_GetBrBytesCnt();
    ulDeltaPacketCnt    = (ulCurrentByteCnt - ulLastByteCnt);

    /* �����bps,ע���ֹ������� */
    if (ulDeltaPacketCnt < ulPeriod)
    {
        ulRate = (ulDeltaPacketCnt * 1000 * 8) / ulPeriod ;
    }
    else
    {
        ulRate = ((ulDeltaPacketCnt * 8) / ulPeriod) * 1000;
    }


    g_stFcBridgeRate.ulLastByteCnt      = ulCurrentByteCnt;
    g_stFcBridgeRate.ulRate             = ulRate;

    return;
}


/*****************************************************************************
 �� �� ��  : FC_ResetBridgeRate
 ��������  : �����������ͳ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_BRIDGE_ResetRate( VOS_VOID )
{
    g_stFcBridgeRate.ulLastByteCnt      = NFExt_GetBrBytesCnt();
    g_stFcBridgeRate.ulRate             = 0;
}


/*****************************************************************************
 �� �� ��  : FC_GetBridgeRate
 ��������  : ��ȡ��������ͳ��ֵ����λ:bps
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��������
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_BRIDGE_GetRate( VOS_VOID )
{
    return(g_stFcBridgeRate.ulRate);
}


/*****************************************************************************
 �� �� ��  : FC_RmRateJudge
 ��������  : Rm�����Ƿ�ﵽ����CPU���ص�����
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE:�ﵽ���ޣ�VOS_FALSE:û�дﵽ����
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_RmRateJudge( VOS_VOID )
{
    /* ����������ʳ������ޣ�����Ϊ�����ʸ�����CPU�� */
    if (g_stFcBridgeRate.ulRate > g_stFcCfg.stFcCfgCpuA.ulRmRateThreshold)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
#else
/*****************************************************************************
 �� �� ��  : FC_CalcBridgeRate
 ��������  : ���������ϵ�����,��λ:bps
 �������  : ulPeriod -- ��������ͳ�����ڣ���λ����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_BRIDGE_CalcRate( VOS_UINT32 ulPeriod )
{
    return;
}


/*****************************************************************************
 �� �� ��  : FC_ResetBridgeRate
 ��������  : �����������ͳ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_BRIDGE_ResetRate( VOS_VOID )
{
    return;
}
#endif

/*****************************************************************************
 �� �� ��  : FC_ACORE_RegDrvAssemFunc
 ��������  : ע������������������ص�����
 �������  : FC_ACORE_DRV_ASSEMBLE_PARA_FUNC pFcDrvSetAssemParaFuncUe  ����UE��������ص�����
             FC_ACORE_DRV_ASSEMBLE_PARA_FUNC pFcDrvSetAssemParaFuncPc  ����PC��������ص�����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 FC_ACORE_RegDrvAssemFunc(FC_ACORE_DRV_ASSEMBLE_PARA_FUNC pFcDrvSetAssemParaFuncUe, FC_ACORE_DRV_ASSEMBLE_PARA_FUNC pFcDrvSetAssemParaFuncPc)
{
    FC_ADS_DRV_ASSEM_STRU stFcAdsAssemEntity;

    g_stDrvAssemParaEntity.pDrvSetAssemParaFuncUe = pFcDrvSetAssemParaFuncUe;
    g_stDrvAssemParaEntity.pDrvSetAssemParaFuncPc = pFcDrvSetAssemParaFuncPc;

    stFcAdsAssemEntity.ucEnableMask         = g_stCpuDriverAssePara.ucEnableMask;
    stFcAdsAssemEntity.ulDlRateUpLev        = FC_ADS_DL_RATE_UP_LEV;
    stFcAdsAssemEntity.ulDlRateDownLev      = FC_ADS_DL_RATE_DOWN_LEV;
    stFcAdsAssemEntity.ulExpireTmrLen       = FC_ADS_TIMER_LEN;
    stFcAdsAssemEntity.pDrvAssemFunc        = FC_JudgeDrvToMaxPara;

    ADS_DL_RegDrvAssemFunc(&stFcAdsAssemEntity);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : FC_ShowDrvAssemPara
 ��������  : ������ά�ɲ��ӡ��Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_ShowDrvAssemPara(VOS_VOID)
{
    FC_CPU_DRV_ASSEM_PARA_STRU *pstCpuDrvAssemPara  = VOS_NULL_PTR;
    VOS_INT i = 0;

    vos_printf("g_stCpuDriverAssePara.ucEnableMask = %d\n", g_stCpuDriverAssePara.ucEnableMask);
    vos_printf("g_stDrvAssemParaEntity.stCurAssemLevPara.ulLev = %d\n", g_stDrvAssemParaEntity.ulCurLev);
    vos_printf("g_stDrvAssemParaEntity.stCurAssemPara.ucEthTxMinNum = %d\n", g_stDrvAssemParaEntity.stCurAssemPara.stDrvAssemPara.ucEthTxMinNum);
    vos_printf("g_stDrvAssemParaEntity.stCurAssemPara.ucCdsGuDlThres = %d\n", g_stDrvAssemParaEntity.stCurAssemPara.stDrvAssemPara.ucCdsGuDlThres);
    vos_printf("g_stDrvAssemParaEntity.ucSmoothCntUp = %d\n", g_stDrvAssemParaEntity.ucSmoothCntUp);
    vos_printf("g_stDrvAssemParaEntity.ucSmoothCntDown = %d\n", g_stDrvAssemParaEntity.ucSmoothCntDown);
    vos_printf("g_stCpuDriverAssePara.ucSmoothCntUpLev = %d\n", g_stCpuDriverAssePara.ucSmoothCntUpLev);
    vos_printf("g_stCpuDriverAssePara.ucSmoothCntDownLev = %d\n", g_stCpuDriverAssePara.ucSmoothCntDownLev);
    vos_printf("g_stDrvAssemParaEntity.pDrvSetAssemParaFunc = 0x%x\n", g_stDrvAssemParaEntity.pDrvSetAssemParaFuncUe);
    vos_printf("g_stDrvAssemParaEntity.ucSetDrvFailCnt = 0x%x\n", g_stDrvAssemParaEntity.ucSetDrvFailCnt);

    for (i = 0; i < FC_ACPU_DRV_ASSEM_LEV_BUTT; i++)
    {
        pstCpuDrvAssemPara = &g_stCpuDriverAssePara.stCpuDrvAssemPara[i];

        vos_printf("pstCpuDrvAssemPara->ulCpuLoad = %d\n", pstCpuDrvAssemPara->ulCpuLoad);
        vos_printf("pstCpuDrvAssemPara->stDrvAssemPara.ucEthRxMinNum = %d\n", pstCpuDrvAssemPara->stDrvAssemPara.ucEthRxMinNum);
        vos_printf("pstCpuDrvAssemPara->stDrvAssemPara.ucEthRxTimeout = %d\n", pstCpuDrvAssemPara->stDrvAssemPara.ucEthRxTimeout);
        vos_printf("pstCpuDrvAssemPara->stDrvAssemPara.ucEthTxMinNum = %d\n", pstCpuDrvAssemPara->stDrvAssemPara.ucEthTxMinNum);
        vos_printf("pstCpuDrvAssemPara->stDrvAssemPara.ucEthTxTimeout = %d\n", pstCpuDrvAssemPara->stDrvAssemPara.ucEthTxTimeout);
        vos_printf("pstCpuDrvAssemPara->stDrvAssemPara.ucHostOutTimeout = %d\n", pstCpuDrvAssemPara->stDrvAssemPara.ucHostOutTimeout);
    }
}

/*****************************************************************************
 �� �� ��  : FC_DrvAssemInit
 ��������  : �������������ʼ������NV���ж���ֵ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_DrvAssemInit(VOS_VOID)
{
    FC_CPU_DRV_ASSEM_PARA_NV_STRU   stCpuDrvAssemPara;
    VOS_UINT32                      ulRst;

    ulRst = NV_ReadEx(MODEM_ID_0, en_NV_Item_FC_ACPU_DRV_ASSEMBLE_PARA, &stCpuDrvAssemPara, sizeof(FC_CPU_DRV_ASSEM_PARA_NV_STRU));

    /* NV�����ȡʧ��ʹ��Ĭ��ֵ */
    if (NV_OK != ulRst)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_DrvAssemInit Read NV fail!\n");

        return;
    }

    g_stDrvAssemParaEntity.stCurAssemPara.ulCpuLoad = 100;
    g_stDrvAssemParaEntity.ulCurLev                 = FC_ACPU_DRV_ASSEM_LEV_1;
    g_stDrvAssemParaEntity.ucSetDrvFailCnt          = 0;

    /* NV�ĳ�����ṹ��ĳ��ȶ��岻һ��ֻ��NV�Ĳ��� */
    PS_MEM_CPY(&g_stCpuDriverAssePara, &stCpuDrvAssemPara, sizeof(FC_CPU_DRV_ASSEM_PARA_NV_STRU));
}

/*****************************************************************************
 �� �� ��  : FC_JudgeAssemSmoothFactor
 ��������  : ����ƽ��ϵ��
 �������  : FC_CPU_DRV_ASSEM_PARA_STRU pstDrvAssemPara ���������Ϣ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_JudgeAssemSmoothFactor(FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara)
{
    /* ƽ��ϵ�����㣬������γ������������� */
    if (g_stDrvAssemParaEntity.stCurAssemPara.ulCpuLoad > pstDrvAssemPara->ulCpuLoad)
    {
        /* �µ���λƽ�� */
        g_stDrvAssemParaEntity.ucSmoothCntDown++;
        g_stDrvAssemParaEntity.ucSmoothCntUp = 0;
    }
    else if (g_stDrvAssemParaEntity.stCurAssemPara.ulCpuLoad < pstDrvAssemPara->ulCpuLoad)
    {
        /* �ϵ���λƽ�� */
        g_stDrvAssemParaEntity.ucSmoothCntUp++;
        g_stDrvAssemParaEntity.ucSmoothCntDown = 0;
    }
    else
    {
        /* �����Ҫ�������Ѿ��뵱ǰ��ͬ��ƽ��ϵ������ */
        g_stDrvAssemParaEntity.ucSmoothCntUp    = 0;
        g_stDrvAssemParaEntity.ucSmoothCntDown  = 0;
    }
}


/*****************************************************************************
 �� �� ��  : FC_GetCpuDrvAssemPara
 ��������  : ���ݵ�ǰ��λ��ȡ����
 �������  : VOS_UINT32 ulLev ��λֵ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
FC_CPU_DRV_ASSEM_PARA_STRU *FC_GetCpuDrvAssemPara(VOS_UINT32 ulLev)
{
    if (ulLev >= FC_ACPU_DRV_ASSEM_LEV_BUTT)
    {
        return VOS_NULL_PTR;
    }

    return &g_stCpuDriverAssePara.stCpuDrvAssemPara[ulLev];
}

/*****************************************************************************
 �� �� ��  : FC_JudgeCdsDlThres
 ��������  : ��CDS������Ϣ������Ϊ:CdsGuDlThres
 �������  : VOS_UINT8 ucThres
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��06��14��
    �޸�����   : �����ɺ���
*****************************************************************************/
VOS_UINT32 FC_JudgeCdsDlThres(VOS_UINT8 ucThres)
{
    FC_CDS_THRES_CHG_IND_STRU *pstFcCdsThresChgMsg = VOS_NULL_PTR;

    pstFcCdsThresChgMsg = (FC_CDS_THRES_CHG_IND_STRU *)PS_ALLOC_MSG_ALL_CHECK(UEPS_PID_FLOWCTRL, sizeof(FC_CDS_THRES_CHG_IND_STRU));

    if (VOS_NULL_PTR == pstFcCdsThresChgMsg)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_JudgeCdsDlThres, Send Msg Fail\n");

        return VOS_ERR;
    }

    pstFcCdsThresChgMsg->enMsgName  = ID_FC_CDS_DL_THRES_CHG_IND;
    pstFcCdsThresChgMsg->ucThres    = ucThres;
    pstFcCdsThresChgMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstFcCdsThresChgMsg->ulReceiverPid   = UEPS_PID_CDS;

    if (VOS_OK != PS_SEND_MSG(UEPS_PID_FLOWCTRL, pstFcCdsThresChgMsg))
    {
        FC_LOG(PS_PRINT_ERROR, "FC_JudgeCdsDlThres, Send Msg Fail\n");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : FC_GetCurrentAssemPara
 ��������  : ��ȡ��ǰASSEM����
 �������  : VOS_UINT32 ulAssemLev ���ݵ�ǰCPUѡ���ĵ�λֵ
 �������  : pstDrvAssemPara
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��06��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
FC_CPU_DRV_ASSEM_PARA_STRU* FC_GetCurrentAssemPara(VOS_UINT32 ulAssemLev)
{
    FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara = VOS_NULL_PTR;

    /* ���ϵ���һ������λ */
    if (g_stDrvAssemParaEntity.ucSmoothCntUp >= g_stCpuDriverAssePara.ucSmoothCntUpLev)
    {
        g_stDrvAssemParaEntity.ulCurLev = ulAssemLev;
    }
    /* ���µ���һ����λһ����λ���� */
    else if (g_stDrvAssemParaEntity.ucSmoothCntDown >= g_stCpuDriverAssePara.ucSmoothCntDownLev)
    {
        if (g_stDrvAssemParaEntity.ulCurLev < (FC_ACPU_DRV_ASSEM_LEV_BUTT - 1))
        {
             g_stDrvAssemParaEntity.ulCurLev++;
        }
    }
    else
    {
        return VOS_NULL_PTR;
    }

    pstDrvAssemPara = FC_GetCpuDrvAssemPara(g_stDrvAssemParaEntity.ulCurLev);

    return pstDrvAssemPara;
}

/*****************************************************************************
 �� �� ��  : FC_DoJudgeDrvAssem
 ��������  : ���������е�������
 �������  : FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara ѡ����λ����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_DoJudgeDrvAssem(FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara)
{
#if 0
    if (FC_ACPU_CDS_GU_ASSEM_ON_MASK == (FC_ACPU_CDS_GU_ASSEM_ON_MASK & g_stCpuDriverAssePara.ucEnableMask))
    {
        FC_JudgeCdsDlThres(pstDrvAssemPara->stDrvAssemPara.ucCdsGuDlThres);
    }

    /* ʹ�ܿ��ش� */
    if (FC_ACPU_DRV_ASSEM_UE_ON_MASK == (FC_ACPU_DRV_ASSEM_UE_ON_MASK & g_stCpuDriverAssePara.ucEnableMask))
    {
        if (VOS_NULL_PTR != g_stDrvAssemParaEntity.pDrvSetAssemParaFuncUe)
        {
            /* ���ûص��������������������,����ط��ǵ���UE��������� */
            if (VOS_OK != g_stDrvAssemParaEntity.pDrvSetAssemParaFuncUe(&(pstDrvAssemPara->stDrvAssemPara)))
            {
                FC_LOG(PS_PRINT_ERROR, "Set Driver Assemble parameter fail!\n");
                /* ����ʧ�ܼ��� */
                g_stDrvAssemParaEntity.ucSetDrvFailCnt++;

                return;
            }
        }
    }
#endif

    /* ʹ�ܿ��ش� */
    if (FC_ACPU_DRV_ASSEM_PC_ON_MASK == (FC_ACPU_DRV_ASSEM_PC_ON_MASK & g_stCpuDriverAssePara.ucEnableMask))
    {
        if (VOS_NULL_PTR != g_stDrvAssemParaEntity.pDrvSetAssemParaFuncPc)
        {
            /* ���ûص�����������������������������PC��������� */
            if (VOS_OK != g_stDrvAssemParaEntity.pDrvSetAssemParaFuncPc(&(pstDrvAssemPara->stDrvAssemPara)))
            {
                FC_LOG(PS_PRINT_ERROR, "Set Driver Assemble parameter fail!\n");
                /* ����ʧ�ܼ��� */
                g_stDrvAssemParaEntity.ucSetDrvFailCnt++;

                return;
            }
        }
    }

    /* ���ϵ������ */
    g_stDrvAssemParaEntity.ucSmoothCntUp    = 0;
    g_stDrvAssemParaEntity.ucSmoothCntDown  = 0;

    /* ����ʧ�ܴ������� */
    g_stDrvAssemParaEntity.ucSetDrvFailCnt  = 0;

    /* ���浱ǰ��λ��Ϣ */
    PS_MEM_CPY(&g_stDrvAssemParaEntity.stCurAssemPara, pstDrvAssemPara, sizeof(FC_CPU_DRV_ASSEM_PARA_STRU));

    /* ��ά�ɲ� */
    FC_MNTN_TraceDrvAssemPara(&(pstDrvAssemPara->stDrvAssemPara));

}

/*****************************************************************************
 �� �� ��  : FC_JudgeDrvAssemAction
 ��������  : ���������е���
 �������  : VOS_UINT32 ulAssemLev ���ݵ�ǰCPUѡ���ĵ�λֵ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_JudgeDrvAssemAction(VOS_UINT32 ulAssemLev)
{

    FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara = VOS_NULL_PTR;

    pstDrvAssemPara = FC_GetCurrentAssemPara(ulAssemLev);

    /* ����ƽ��ϵ����ˮ��֮������������ */
    if ( VOS_NULL_PTR != pstDrvAssemPara)
    {
        FC_DoJudgeDrvAssem(pstDrvAssemPara);
    }
}

/*****************************************************************************
 �� �� ��  : FC_JudgeDrvToMaxPara
 ��������  : ���������������λ
 �������  : VOS_VOID
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_JudgeDrvToMaxPara(VOS_VOID)
{
    FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara = VOS_NULL_PTR;

    /* ֱ�ӵ��������λ */
    g_stDrvAssemParaEntity.ulCurLev = FC_ACPU_DRV_ASSEM_LEV_1;

    pstDrvAssemPara =  FC_GetCpuDrvAssemPara(FC_ACPU_DRV_ASSEM_LEV_1);

    FC_DoJudgeDrvAssem(pstDrvAssemPara);
}

/*****************************************************************************
 �� �� ��  : FC_SelectDrvAssemParaRule
 ��������  : ͨ��CPU LOAD�����ѡ�������λ
 �������  : VOS_UINT32 ulCpuLoad ��ǰCPU LOAD���
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
FC_CPU_DRV_ASSEM_PARA_STRU* FC_SelectDrvAssemParaRule(VOS_UINT32 ulCpuLoad, VOS_UINT32 *pulAssemLev)
{
    FC_CPU_DRV_ASSEM_PARA_STRU      *pstCpuDrvAssemParaRst  = VOS_NULL_PTR;
    FC_CPU_DRV_ASSEM_PARA_STRU      *pstCpuDrvAssemPara     = VOS_NULL_PTR;
    VOS_INT                          i                      = 0;

    /* ѡ����ǰ��Ҫ������λ */
    for (i = 0; i < FC_ACPU_DRV_ASSEM_LEV_BUTT; i++)
    {
        pstCpuDrvAssemPara = &g_stCpuDriverAssePara.stCpuDrvAssemPara[i];
        if (ulCpuLoad >= pstCpuDrvAssemPara->ulCpuLoad)
        {
            pstCpuDrvAssemParaRst   = pstCpuDrvAssemPara;
           *pulAssemLev             = (VOS_UINT32)i;

            break;
        }
    }

    return pstCpuDrvAssemParaRst;
}


/*****************************************************************************
 �� �� ��  : FC_JudgeDrvAssemPara
 ��������  : ͨ��CPU LOAD����������������������
 �������  : VOS_UINT32 ulCpuLoad ��ǰCPU LOAD���
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��05��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_JudgeDrvAssemPara(VOS_UINT32 ulCpuLoad)
{
    FC_CPU_DRV_ASSEM_PARA_STRU *pstDrvAssemPara  = VOS_NULL_PTR;
    /* Ĭ����ߵ� */
    VOS_UINT32                  ulAssemLev       = FC_ACPU_DRV_ASSEM_LEV_1;

    /* ʹ�ܿ���δ��ʱ�����ټ������沽�� */
    if (0 == g_stCpuDriverAssePara.ucEnableMask)
    {
        return;
    }

    if (0 != (g_stDrvAssemParaEntity.ucSetDrvFailCnt % CPU_MAX_SET_DRV_FAIL_SMOOTH_CNT))
    {
        g_stDrvAssemParaEntity.ucSetDrvFailCnt++;

        return;
    }

    /* ����CPU�������ѡ���Ӧ��λ */
    pstDrvAssemPara = FC_SelectDrvAssemParaRule(ulCpuLoad, &ulAssemLev);

    if (VOS_NULL_PTR == pstDrvAssemPara)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_JudgeDrvAssemPara pstDrvAssemPara is NULL!\n");

        return;
    }

    /* ���ݵ�ǰ��λ����ƽ������ */
    FC_JudgeAssemSmoothFactor(pstDrvAssemPara);

    /* �������������� */
    FC_JudgeDrvAssemAction(ulAssemLev);
}

/*****************************************************************************
 �� �� ��  : FC_UmRateOverThreshold
 ��������  : �տ������Ƿ�ﵽ����CPU���ص�����
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE:�ﵽ���ޣ�VOS_FALSE:û�дﵽ����
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_UmRateOverThreshold( VOS_VOID )
{
    VOS_UINT32                          ulUlRate;
    VOS_UINT32                          ulDlRate;


    /* ��ȡUM������ */
    ADS_GetCurrentRate(&ulUlRate, &ulDlRate);


    /* ������л��������ʳ������ޣ�����Ϊ�����ʸ�����CPU�� */
    if ( (ulUlRate > g_stFcCfg.stFcCfgCpuA.ulUmUlRateThreshold)
        || (ulDlRate > g_stFcCfg.stFcCfgCpuA.ulUmDlRateThreshold) )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


/*****************************************************************************
 �� �� ��  : FC_PsRateJudge
 ��������  : �ж��Ƿ�������
 �������  : ��
 �������  : ���ʳ���VOS_TRUE��������VOS_FALSE
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_PsRateJudge( VOS_VOID )
{
    /*
    STICK��̬�жϿտ�����
    E5��̬�ж���������
    */
#if(FEATURE_ON == FEATURE_NFEXT)
    return (FC_RmRateJudge());
#else
    return (FC_UmRateOverThreshold());
#endif
}


/*****************************************************************************
 �� �� ��  : FC_GetPsRate
 ��������  : ��ȡ��������
 �������  : None
 �������  : pulUlRate -- �����������ָ�룬bps
              pulDlRate -- �����������ָ�룬bps
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_GetPsRate( VOS_UINT32 *pulUlRate, VOS_UINT32 *pulDlRate )
{
    /* E5��̬�£���ȡ�������� */
#if(FEATURE_ON == FEATURE_NFEXT)
   /* �����ϣ����������ʶ���ֵ���������� */
   *pulUlRate   = FC_BRIDGE_GetRate();
   *pulDlRate   = *pulUlRate;
#else
    /* STICK��̬�£���ȡ�������� */
    ADS_GetCurrentRate(pulUlRate, pulDlRate);
#endif
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_UpJudge
 ��������  : �Ƿ񴥷�CPU���أ���һ�λ���ƽ������
 �������  : ulCpuLoad   -- ��ǰCPU������0~100
              pstFcCfgCpu -- CPU����������
              pstFcPolicy -- CPU���ز���ʵ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE:��Ҫ�������أ�VOS_FALSE:����Ҫ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��5��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 FC_CPUA_UpJudge
(
    VOS_UINT32       ulCpuLoad,
    FC_CFG_CPU_STRU *pstFcCfgCpu,
    FC_POLICY_STRU  *pstFcPolicy
)
{
    VOS_UINT32                          ulResult;


    if ( ulCpuLoad < pstFcCfgCpu->ulCpuOverLoadVal )
    {
        g_stFcCpuACtrl.ulSmoothTimerLen = 0;
        return VOS_FALSE;
    }

    /* �Ѿ��ﵽCPU���ص���߼��𣬲����ٽ�һ������ */
    if (pstFcPolicy->enHighestPri <= pstFcPolicy->enDonePri)
    {
        return VOS_FALSE;
    }


    /* �Ƿ����ƽ���������ж�  */
    /* ����Ѿ���ʼ����CPU���أ�������������أ����ٽ���ƽ�������ʵ��ж� */
    if ( FC_PRI_NULL != pstFcPolicy->enDonePri )
    {
        return VOS_TRUE;
    }

    if ( 0 == g_stFcCpuACtrl.ulSmoothTimerLen )
    {
        FC_BRIDGE_ResetRate();
    }

    g_stFcCpuACtrl.ulSmoothTimerLen++;

    if (g_stFcCpuACtrl.ulSmoothTimerLen < pstFcCfgCpu->ulSmoothTimerLen)
    {
        return VOS_FALSE;
    }

    /* A��ulSmoothTimerLen >= 2�����FC_CFG_CheckParam������ע�� */
    FC_BRIDGE_CalcRate((pstFcCfgCpu->ulSmoothTimerLen - 1) * CPULOAD_GetRegularTimerLen());

    g_stFcCpuACtrl.ulSmoothTimerLen  = 0;


    /* ���������Ƿ���CPU�ߵ�ԭ�� */
    ulResult    = FC_PsRateJudge();
    if (VOS_FALSE == ulResult)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_DownJudge
 ��������  : �Ƿ񴥷�CPU�������
 �������  : ulCpuLoad   -- ��ǰCPU������0~100
              pstFcCfgCpu -- CPU����������
              pstFcPolicy -- CPU���ز���ʵ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE:��Ҫ������أ�VOS_FALSE:����Ҫ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��5��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 FC_CPUA_DownJudge
(
    VOS_UINT32       ulCpuLoad,
    FC_CFG_CPU_STRU *pstFcCfgCpu,
    FC_POLICY_STRU  *pstFcPolicy
)
{
    if ( (ulCpuLoad < pstFcCfgCpu->ulCpuUnderLoadVal)
         && (FC_PRI_NULL < pstFcPolicy->enDonePri) )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


/*****************************************************************************
 �� �� ��  : FC_RcvCpuLoad
 ��������  : A������ģ���ṩ��CPU���ش����ӿ�
 �������  : ulCpuLoad  --  ��ǰCPU����0~100
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��5��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_CPUA_RcvCpuLoad(VOS_UINT32  ulCpuLoad)
{
    FC_CFG_CPU_STRU                    *pstFcCfgCpu;
    FC_POLICY_STRU                     *pstFcPolicy;
    VOS_UINT32                          ulStartCtrl;
    VOS_UINT32                          ulUlRate;
    VOS_UINT32                          ulDlRate;


    /* ͨ��CPU LOAD���������������������CPU���ؿ��ؿ��� */
    FC_JudgeDrvAssemPara(ulCpuLoad);


    /* ������� */
    if ( FC_POLICY_MASK_CPU_A != (FC_POLICY_MASK_CPU_A & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* CPU����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_CPU_ProcessLoad, INFO, CPU FlowCtrl is disabled %d\n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return;
    }

    /* �����ά�ɲ� */
    FC_MNTN_TraceCpuLoad(ID_FC_MNTN_CPU_A_CPULOAD, ulCpuLoad);

    if ( 100 < ulCpuLoad )
    {
        /* �����Ƿ� */
        FC_LOG1(PS_PRINT_WARNING, "FC_CPU_ProcessLoad, WARNING, Invalid Cpu Load %d\n",
                (VOS_INT32)ulCpuLoad);
        return;
    }


    pstFcCfgCpu = &(g_stFcCfg.stFcCfgCpuA);
    pstFcPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CPU_A_MODEM_0);

    /* �Ƿ�Ҫ����CPU���� */
    ulStartCtrl = FC_CPUA_UpJudge(ulCpuLoad, pstFcCfgCpu, pstFcPolicy);
    if (VOS_TRUE == ulStartCtrl)
    {
        FC_GetPsRate(&ulUlRate, &ulDlRate);
        FC_SndCpuMsg(ID_FC_CPU_A_OVERLOAD_IND, ulCpuLoad, ulUlRate, ulDlRate);
        return;
    }


    /* ����CPU�������о� */
    ulStartCtrl = FC_CPUA_DownJudge(ulCpuLoad, pstFcCfgCpu, pstFcPolicy);
    if ( VOS_TRUE ==  ulStartCtrl )
    {
        FC_GetPsRate(&ulUlRate, &ulDlRate);
        FC_SndCpuMsg(ID_FC_CPU_A_UNDERLOAD_IND, ulCpuLoad, ulUlRate, ulDlRate);
        return;
    }

    return;
}


/*****************************************************************************
 �� �� ��  : FC_POLICY_CpuStopFcAttempt
 ��������  : ���Խ��CPU����
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CPUA_StopFcAttempt( VOS_UINT32 ulParam1, VOS_UINT32 ulParam2 )
{
    FC_CFG_CPU_STRU                     *pstCfgCpu;


    pstCfgCpu   = &(g_stFcCfg.stFcCfgCpuA);
    if (0 == pstCfgCpu->ulStopAttemptTimerLen)
    {
        return VOS_OK;
    }


    CPULOAD_ResetUserDefLoad();

    /*�����ʱ���Ѿ�����������Ҫ�ٴ�����*/
    if (VOS_NULL_PTR == g_stFcCpuACtrl.pstStopAttemptTHandle)
    {
        if ( VOS_OK != VOS_StartRelTimer(&g_stFcCpuACtrl.pstStopAttemptTHandle,
                                            UEPS_PID_FLOWCTRL,
                                            pstCfgCpu->ulStopAttemptTimerLen,
                                            FC_TIMER_STOP_CPU_ATTEMPT,
                                            0,
                                            VOS_RELTIMER_NOLOOP,
                                            VOS_TIMER_PRECISION_0))
        {
            FC_LOG(PS_PRINT_WARNING, "FC_CPUA_StopFcAttempt, VOS_StartRelTimer Fail!\n");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_UpProcess
 ��������  : ACPU���ص����ش���
 �������  : ��
 �������  : ��
 �� �� ֵ  : �ɹ�VOS_OK,ʧ��VOS_ERR
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CPUA_UpProcess( VOS_VOID )
{
    FC_POLICY_STRU                     *pFcPolicy;


    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pFcPolicy   = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CPU_A_MODEM_0);
    FC_POLICY_Up(pFcPolicy);


    /* �Ѿ��ǵ�ǰ���Ե���߼����ˣ�ִ�лص����� */
    if (pFcPolicy->enDonePri == pFcPolicy->enHighestPri)
    {
        if (VOS_NULL_PTR != pFcPolicy->pPostFunc)
        {
            (VOS_VOID)pFcPolicy->pPostFunc(FC_POLICY_ID_CPU_A, 0);
        }
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_DownProcess
 ��������  : ACPU���صĽ�����ش���
 �������  : ��
 �������  : ��
 �� �� ֵ  : �ɹ�VOS_OK,ʧ��VOS_ERR
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CPUA_DownProcess( VOS_VOID )
{
    FC_POLICY_STRU                      *pPolicy;


    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CPU_A_MODEM_0);
    FC_POLICY_Down(pPolicy);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_StopFlowCtrl
 ��������  : ����CPU���أ��ж��Ƿ������ǰ���CPU����
 �������  : ��
 �������  : ��
 �� �� ֵ  : �ɹ�VOS_OK,ʧ��VOS_ERR
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CPUA_StopFlowCtrl( VOS_VOID )
{
    FC_CFG_CPU_STRU                    *pstFcCfgCpu;
    FC_POLICY_STRU                     *pstFcPolicy;
    VOS_UINT32                          ulCpuLoad;


    pstFcCfgCpu = &(g_stFcCfg.stFcCfgCpuA);
    pstFcPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CPU_A_MODEM_0);

    ulCpuLoad   = CPULOAD_GetUserDefLoad();

    /* ��ǰCPUС�ڽ���������ֵ,������ */
    if ( (ulCpuLoad <= pstFcCfgCpu->ulCpuUnderLoadVal)
         && (FC_PRI_NULL < pstFcPolicy->enDonePri) )
    {
        FC_CPUA_DownProcess();
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CPUA_Init
 ��������  : ACPU�������صĳ�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��22��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CPUA_Init( VOS_VOID )
{
    /* ����ʹ�ú꿪���ж��Ƿ�ע��ص����� */
#if(FEATURE_ON == FEATURE_ACPU_STAT)
    /* ��CPU���ģ��ע��ص����� */
    if ( VOS_OK != CPULOAD_RegRptHook((CPULOAD_RPT_HOOK_FUNC)FC_CPUA_RcvCpuLoad) )
    {
        FC_LOG(PS_PRINT_ERROR, "FC_CPUA_Init, ERROR, CPULOAD_RegRptHook return error!\n");
        return VOS_ERR;
    }

    /* A��CPU���ز��Գ�ʼ�� */
    g_astFcPolicy[FC_POLICY_ID_CPU_A].pPostFunc   = FC_CPUA_StopFcAttempt;
#endif

    VOS_MemSet(&g_stFcBridgeRate, 0, sizeof(FC_BRIDGE_RATE_STRU));
    VOS_MemSet(&g_stFcCpuACtrl, 0, sizeof(g_stFcCpuACtrl));

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_POLICY_CalcUpTargetFcPris
 ��������  : �ڴ����ʱ�������ڴ�ռ������������ڴ����ص�Ŀ�����ؼ���
              �ڴ����ֻ��һ�����ȼ�ʱ������Lev3����
 �������  : pPolicy    -- �ڴ����ز���
              ulMemValue -- ��ǰ�ڴ�ռ����
 �������  : ��
 �� �� ֵ  : Ŀ�����ؼ���
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_CalcUpTargetFcPri
(
    FC_POLICY_STRU                     *pPolicy,
    VOS_UINT32                          ulMemValue
)
{
    FC_PRI_ENUM_UINT32                  enTargetPri;
    FC_CFG_MEM_STRU                    *pstMemCfg;


    pstMemCfg   = &(g_stFcCfg.stFcCfgMem);
    enTargetPri = pPolicy->enDonePri;

    /* �ڴ����ֻע����һ�����ȼ�������Lev3���� */
    if (1 == pPolicy->ucPriCnt)
    {
        /* �����Ŀ���������ȼ� */
        if (ulMemValue <= pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_3].ulSetThreshold)
        {
            enTargetPri     = pPolicy->enHighestPri;
        }
    }
    /* �ڴ����ֻע���˶�����ȼ� */
    else
    {
        /* �����Ŀ���������ȼ� */
        if (ulMemValue <= pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_4].ulSetThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_4;
        }
        else if (ulMemValue <= pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_3].ulSetThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_3;
        }
        else if (ulMemValue <= pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_2].ulSetThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_2;
        }
        else if (ulMemValue <= pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_1].ulSetThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_1;
        }
        else
        {
            ;
        }
    }

    return enTargetPri;
} /* FC_MEM_CalcUpTargetFcPri */


/*****************************************************************************
 �� �� ��  : FC_MEM_CalcDownTargetFcPri
 ��������  : �ڴ�ݼ�ʱ�������ڴ�ռ������������ڴ����ص�Ŀ������ؼ���
              �ڴ����ֻ��һ�����ȼ�ʱ������Lev3����
 �������  : pPolicy    -- �ڴ����ز���
              ulMemValue -- ��ǰ�ڴ�ռ����
 �������  : ��
 �� �� ֵ  : Ŀ������ؼ���
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��9��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_CalcDownTargetFcPri
(
    FC_POLICY_STRU                     *pPolicy,
    VOS_UINT32                          ulMemValue
)
{
    FC_PRI_ENUM_UINT32                  enTargetPri;
    FC_CFG_MEM_STRU                    *pstMemCfg;


    pstMemCfg   = &(g_stFcCfg.stFcCfgMem);
    enTargetPri = pPolicy->enDonePri;

    /* �ڴ����ֻע����һ�����ȼ� */
    if (1 == pPolicy->ucPriCnt)
    {
        /* �����Ŀ���������ȼ� */
        if (ulMemValue > pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_3].ulStopThreshold)
        {
            enTargetPri     = FC_PRI_NULL;
        }
    }
    /* �ڴ����ֻע���˶�����ȼ� */
    else
    {
        /* �����Ŀ���������ȼ� */
        if (ulMemValue > pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_1].ulStopThreshold)
        {
            enTargetPri     = FC_PRI_NULL;
        }
        else if (ulMemValue > pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_2].ulStopThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_1;
        }
        else if (ulMemValue > pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_3].ulStopThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_2;
        }
        else if (ulMemValue > pstMemCfg->astThreshold[FC_MEM_THRESHOLD_LEV_4].ulStopThreshold)
        {
            enTargetPri     = FC_PRI_FOR_MEM_LEV_3;
        }
        else
        {
            ;
        }
    }

    return enTargetPri;
} /* FC_MEM_CalcUpTargetFcPri */


/*****************************************************************************
 �� �� ��  : FC_MEM_AdjustPriForUp
 ��������  :
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_AdjustPriForUp
(
    FC_PRI_ENUM_UINT32                  enPointPri,
    FC_ID_ENUM_UINT32                   enFcId
)
{
    FC_POLICY_STRU                     *pPolicy;
    FC_PRI_ENUM_UINT32                  enTargetPri;
    VOS_UINT32                          ulMemValue;


    /* ��ȡ�ڴ����ز��Ժ��ڴ��������� */
    pPolicy         = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_MEM_MODEM_0);


    /* �����ڴ����������ˢ���ڴ�����Ŀ�����ȼ� */
    ulMemValue      = IMM_GetLocalFreeMemCnt();
    enTargetPri     = FC_MEM_CalcUpTargetFcPri(pPolicy, ulMemValue);


    /* Ŀ�����ȼ������󣬽������ش��������������µ��������ȼ� */
    if (pPolicy->enDonePri < enTargetPri)
    {
        FC_POLICY_UpToTargetPri(pPolicy, enTargetPri);
    }
    else if (pPolicy->enDonePri > enTargetPri)
    {
        FC_POLICY_DownToTargetPri(pPolicy, enTargetPri);
    }
    else
    {
        ;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_POLICY_AdjustMemPri
 ��������  :
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��14��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_AdjustPriForDown
(
    FC_PRI_ENUM_UINT32                  enPointPri,
    VOS_UINT32                          ulFcId
)
{
    FC_POLICY_STRU                     *pPolicy;
    FC_PRI_ENUM_UINT32                  enTargetPri;
    VOS_UINT32                          ulMemValue;


    /* ��ȡ�ڴ����ز��Ժ��ڴ��������� */
    pPolicy         = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_MEM_MODEM_0);


    /* �����ڴ����������ˢ���ڴ�����Ŀ�����ȼ� */
    ulMemValue      = IMM_GetLocalFreeMemCnt();
    enTargetPri     = FC_MEM_CalcDownTargetFcPri(pPolicy, ulMemValue);


    /* Ŀ�����ȼ������󣬽������ش��������������µ��������ȼ� */
    if (pPolicy->enDonePri < enTargetPri)
    {
        FC_POLICY_UpToTargetPri(pPolicy, enTargetPri);
    }
    else if (pPolicy->enDonePri > enTargetPri)
    {
        FC_POLICY_DownToTargetPri(pPolicy, enTargetPri);
    }
    else
    {
        ;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_SndMemUpToTargetPriIndMsg
 ��������  : �����ڴ����ص�Ŀ��ֵ����Ϣ֪ͨ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��20��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_SndUpToTargetPriIndMsg(FC_PRI_ENUM_UINT32 enTargetPri, VOS_UINT16 usMemFreeCnt)
{
    FC_MEM_UP_TO_TARGET_PRI_IND_STRU   *pstMsg;
    VOS_UINT32                          ulResult;


    pstMsg = (FC_MEM_UP_TO_TARGET_PRI_IND_STRU *)VOS_AllocMsg(UEPS_PID_FLOWCTRL, \
                                         sizeof(FC_MEM_UP_TO_TARGET_PRI_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstMsg)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_SndMemUpToTargetPriIndMsg, Alloc Msg Fail\n");
        return VOS_ERR;
    }

    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL;
    pstMsg->usMsgName       = ID_FC_MEM_UP_TO_TARGET_PRI_IND;
    pstMsg->enTargetPri     = enTargetPri;
    pstMsg->usMemFreeCnt    = usMemFreeCnt;

    ulResult = VOS_SendMsg(UEPS_PID_FLOWCTRL, pstMsg);

    if (VOS_OK != ulResult)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_SndMemUpToTargetPriIndMsg, Send Msg Fail\n");
        return VOS_ERR;
    }

    return VOS_OK;

}


/*****************************************************************************
 �� �� ��  : FC_SndMemDownToTargetPriIndMsg
 ��������  : �����ڴ����ص�Ŀ��ֵ����Ϣ֪ͨ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��20��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_SndDownToTargetPriIndMsg( FC_PRI_ENUM_UINT32 enTargetPri, VOS_UINT16 usMemFreeCnt)
{
    FC_MEM_DOWN_TO_TARGET_PRI_IND_STRU *pstMsg;
    VOS_UINT32                          ulResult;


    pstMsg = (FC_MEM_DOWN_TO_TARGET_PRI_IND_STRU *)VOS_AllocMsg(UEPS_PID_FLOWCTRL, \
                                         sizeof(FC_MEM_DOWN_TO_TARGET_PRI_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstMsg)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_SndMemDownToTargetPriIndMsg, Alloc Msg Fail\n");
        return VOS_ERR;
    }

    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL;
    pstMsg->usMsgName       = ID_FC_MEM_DOWN_TO_TARGET_PRI_IND;
    pstMsg->enTargetPri     = enTargetPri;
    pstMsg->usMemFreeCnt    = usMemFreeCnt;

    ulResult = VOS_SendMsg(UEPS_PID_FLOWCTRL, pstMsg);

    if (VOS_OK != ulResult)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_SndMemDownToTargetPriIndMsg, Send Msg Fail\n");
        return VOS_ERR;
    }

    return VOS_OK;

}


/*****************************************************************************
 �� �� ��  : FC_MEM_UpProcess
 ��������  : �ڴ����ز�����ں���
 �������  : ulMemValue:�ڴ����
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID FC_MEM_UpProcess( VOS_UINT32 ulMemValue  )
{
    FC_POLICY_STRU                     *pPolicy;
    FC_PRI_ENUM_UINT32                  enTargetPri;
    VOS_UINT32                          ulFlags = 0;

    /* ������� */
    if ( FC_POLICY_MASK_MEM != (FC_POLICY_MASK_MEM & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_MEM_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return;
    }

    pPolicy         = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_MEM_MODEM_0);
    enTargetPri     = FC_MEM_CalcUpTargetFcPri(pPolicy, ulMemValue);

    if ((pPolicy->enDonePri < enTargetPri) && (pPolicy->enToPri < enTargetPri))
    {
        /* ��������ж����಻�ɴ�ϵ������У��Ͳ���������Ϣ */
        /*lint -e506 -e774*/
        if (likely(preemptible()))
        /*lint +e506 +e774*/
        {
            VOS_SpinLockIntLock(&g_stFcMemSpinLock, ulFlags);
            pPolicy->enToPri = enTargetPri;
            VOS_SpinUnlockIntUnlock(&g_stFcMemSpinLock , ulFlags);

            /* ��Ϣ����ʧ�ܻᵥ�帴λ�����ٽ���enToPri�Ļָ� */
            FC_MEM_SndUpToTargetPriIndMsg(enTargetPri, (VOS_UINT16)ulMemValue);

        }
    }

    return;
}

/*****************************************************************************
 �� �� ��  : FC_MEM_DownProcess
 ��������  : �ڴ�ʹ�������٣����ܴ����������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_MEM_DownProcess( VOS_UINT32 ulMemValue )
{
    FC_POLICY_STRU                     *pPolicy;
    FC_PRI_ENUM_UINT32                  enTargetPri;
    VOS_UINT32                          ulFlags = 0;

    /* ������� */
    if ( FC_POLICY_MASK_MEM != (FC_POLICY_MASK_MEM & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_MEM_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return;
    }

    pPolicy         = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_MEM_MODEM_0);
    enTargetPri     = FC_MEM_CalcDownTargetFcPri(pPolicy, ulMemValue);

    if ((pPolicy->enDonePri > enTargetPri) && (pPolicy->enToPri > enTargetPri))
    {
        /* ��������ж����಻�ɴ�ϵ������У��Ͳ���������Ϣ */
        /*lint -e506 -e774*/
        if (likely(preemptible()))
        /*lint +e506 +e774*/
        {
            VOS_SpinLockIntLock(&g_stFcMemSpinLock, ulFlags);
            pPolicy->enToPri = enTargetPri;
            VOS_SpinUnlockIntUnlock(&g_stFcMemSpinLock, ulFlags);

            /* ��Ϣ����ʧ�ܻᵥ�帴λ�����ٽ���enToPri�Ļָ� */
            FC_MEM_SndDownToTargetPriIndMsg(enTargetPri, (VOS_UINT16)ulMemValue);
        }
    }

    return;
}


/*****************************************************************************
 �� �� ��  : FC_MEM_Init
 ��������  : �ڴ����س�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��27��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_MEM_Init( VOS_VOID )
{
    VOS_UINT32                          ulResult;

    VOS_SpinLockInit(&g_stFcMemSpinLock);

    /* V9R1��Ŀ���ж�ACPU�ڴ�����ʹ����ע��ص�����������ע�� */
    /* ע���ڴ�ص����� */
    if ( (FC_POLICY_MASK(FC_POLICY_ID_MEM) == (FC_POLICY_MASK(FC_POLICY_ID_MEM) & g_stFcCfg.ulFcEnbaleMask) ))
    {
        /* A���ڴ����ز��Գ�ʼ�� */
        g_astFcPolicy[FC_POLICY_ID_MEM].pAdjustForUpFunc    = FC_MEM_AdjustPriForUp;
        g_astFcPolicy[FC_POLICY_ID_MEM].pAdjustForDownFunc  = FC_MEM_AdjustPriForDown;

        ulResult = IMM_MemRegEventCallBack(IMM_MEM_POOL_ID_SHARE, FC_MEM_UpProcess, FC_MEM_DownProcess);

        if (IMM_SUCC != ulResult)
        {
            FC_LOG(PS_PRINT_WARNING, "FC, FC_MEM_Init, ERROR, RegMemEventcall fail\n");
            return VOS_ERR;
        }
    }
    else
    {
         /* A���ڴ����ز��Գ�ʼ�� */
        g_astFcPolicy[FC_POLICY_ID_MEM].pAdjustForUpFunc    = VOS_NULL_PTR;
        g_astFcPolicy[FC_POLICY_ID_MEM].pAdjustForDownFunc  = VOS_NULL_PTR;

        IMM_MemRegEventCallBack(IMM_MEM_POOL_ID_SHARE, VOS_NULL_PTR, VOS_NULL_PTR);
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CST_UpProcess
 ��������  : CST���ز���
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CST_UpProcess( VOS_UINT8 ucRabId )
{
    FC_POLICY_STRU                     *pPolicy;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_CST != (FC_POLICY_MASK_CST & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_CST_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }


    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CST_MODEM_0);
    FC_POLICY_UpToTargetPri(pPolicy, pPolicy->enHighestPri);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CST_DownProcess
 ��������  : CST������ز���
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CST_DownProcess( VOS_UINT8 ucRabId )
{
    FC_POLICY_STRU                     *pPolicy;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_CST != (FC_POLICY_MASK_CST & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_CST_DownProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }


    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_CST_MODEM_0);
    FC_POLICY_DownToTargetPri(pPolicy, FC_PRI_NULL);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_GetFcInfo
 ��������  : ��ȡFc����ʵ��
 �������  : VOS_UINT8 ucRabId  ��Ҫ���ҵĶ�ӦFC��Ϣ��Rab Id
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��12��
    �޸�����   : �����ɺ���

*****************************************************************************/
FC_RAB_MAPPING_INFO_STRU  *FC_CDS_GetFcInfo( VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_SET_STRU       *pstFcRabMappingInfoSet;
    FC_RAB_MAPPING_INFO_STRU           *pstFcRabMappingInfo;
    FC_ID_ENUM_UINT32                   enFcId;
    VOS_UINT32                          ulRabMask;


    pstFcRabMappingInfoSet  = &g_astFcRabMappingInfoSet[enModemId];
    ulRabMask               = (1 << ucRabId);

    /*====================================*//* ��������ӳ���ϵ������ҵ�����ָ��RAB_ID��ӳ���ϵ�����˳� */
    for (enFcId = 0; enFcId < pstFcRabMappingInfoSet->ulFcIdCnt; enFcId++)
    {
        pstFcRabMappingInfo = &(pstFcRabMappingInfoSet->astFcRabMappingInfo[enFcId]);
        if ( 0 != (ulRabMask & pstFcRabMappingInfo->ulIncludeRabMask) )
        {
            return pstFcRabMappingInfo;
        }
    }

    return VOS_NULL_PTR;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_DelFcId
 ��������  : ɾ��FC Id��Ӧ��FC��Ϣ
 �������  : FC_ID_ENUM_UINT32 enFcId
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_DelFcId( FC_ID_ENUM_UINT32 enFcId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_SET_STRU       *pstFcRabMappingInfoSet;
    FC_RAB_MAPPING_INFO_STRU           *pstFcRabMappingInfo;
    FC_ID_ENUM_UINT32                   enFcIdNum;
    FC_ID_ENUM_UINT32                   enShiftFcId;


    pstFcRabMappingInfoSet    = &g_astFcRabMappingInfoSet[enModemId];

    for (enFcIdNum = 0; enFcIdNum < pstFcRabMappingInfoSet->ulFcIdCnt; enFcIdNum++)
    {
        if (enFcId == pstFcRabMappingInfoSet->astFcRabMappingInfo[enFcIdNum].enFcId)
        {
            break;
        }
    }

    if (enFcIdNum >= pstFcRabMappingInfoSet->ulFcIdCnt)
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_CDS_DelFcId, can not find the Fc Id <1>!\n",
            (VOS_INT32)enFcId);
        return VOS_ERR;
    }


    /* �������ӳ���ϵ������������������������ */
    for (enShiftFcId = enFcIdNum + 1; enShiftFcId < pstFcRabMappingInfoSet->ulFcIdCnt; enShiftFcId++)
    {
        pstFcRabMappingInfo   = &(pstFcRabMappingInfoSet->astFcRabMappingInfo[enShiftFcId]);
        PS_MEM_CPY((pstFcRabMappingInfo - 1), pstFcRabMappingInfo, sizeof(FC_RAB_MAPPING_INFO_STRU));
    }

    /* ���һ��ӳ���ϵ�޷����������ǣ�������Ҫ�ֶ���� */
    PS_MEM_SET(&(pstFcRabMappingInfoSet->astFcRabMappingInfo[enShiftFcId-1]), 0, sizeof(FC_RAB_MAPPING_INFO_STRU));
    pstFcRabMappingInfoSet->ulFcIdCnt--;

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_AddRab
 ��������  : ����ClentId��RABӳ��
 �������  : FC_ID_ENUM_UINT32 enFcId   ��Ӧҵ�������豸��Fc Id
             VOS_UINT8 ucRabId          ��Ӧҵ���RabId
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_AddRab(FC_ID_ENUM_UINT32 enFcId, VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_SET_STRU       *pstFcRabMappingInfoSet;
    FC_RAB_MAPPING_INFO_STRU           *pstFcRabMappingInfo;
    FC_ID_ENUM_UINT32                   enFcIdNum;
    VOS_UINT32                          ulRabMask;


    pstFcRabMappingInfoSet  = &g_astFcRabMappingInfoSet[enModemId];
    ulRabMask               = (1 << ucRabId);

    if ( FC_MAX_NUM < pstFcRabMappingInfoSet->ulFcIdCnt )
    {
        FC_LOG1(PS_PRINT_ERROR, "FC_CDS_AddRab, g_astFcRabMappingInfoSet is exceed the ranger!\n",
            (VOS_INT32)pstFcRabMappingInfoSet->ulFcIdCnt);
        return VOS_ERR;
    }

    /*====================================*//* ������FC��������Ϣ������ҵ�����ָ��RAB_ID��Fc Id�����˳� */
    for (enFcIdNum = 0; enFcIdNum < pstFcRabMappingInfoSet->ulFcIdCnt; enFcIdNum++)
    {
        pstFcRabMappingInfo = &(pstFcRabMappingInfoSet->astFcRabMappingInfo[enFcIdNum]);
        if ( enFcId == pstFcRabMappingInfo->enFcId )
        {
            if ( 0 == (pstFcRabMappingInfo->ulIncludeRabMask & ulRabMask) )
            {
                pstFcRabMappingInfo->ulIncludeRabMask   |= ulRabMask;
                pstFcRabMappingInfo->ulNoFcRabMask      |= ulRabMask;
                FC_POINT_ClrFc(FC_POLICY_MASK_CDS, enFcId);
            }

            return VOS_OK;
        }
    }

    if ( FC_MAX_NUM == pstFcRabMappingInfoSet->ulFcIdCnt )
    {
        FC_LOG(PS_PRINT_ERROR, "FC_CDS_AddRab, AtClientCnt reaches the max num!\n");
        return VOS_ERR;
    }

    /*====================================*//* ������µ�FC Id�������Ӹ�Fc Id��RAB ID */
    pstFcRabMappingInfo = &(pstFcRabMappingInfoSet->astFcRabMappingInfo[pstFcRabMappingInfoSet->ulFcIdCnt]);
    pstFcRabMappingInfo->enFcId             = enFcId;
    pstFcRabMappingInfo->ulIncludeRabMask   = ulRabMask;
    pstFcRabMappingInfo->ulNoFcRabMask      = ulRabMask;

    pstFcRabMappingInfoSet->ulFcIdCnt++;

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_DelRab
 ��������  : �����RAB ID��Ӧ��������Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_DelRab( VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_STRU           *pstFcRabMappingInfo;
    VOS_UINT32                          ulRabMask;


    /*====================================*//* ͨ��Rab Id�����Ҹ�Rab Id���ڵ�FCʵ�� */
    pstFcRabMappingInfo                 = FC_CDS_GetFcInfo(ucRabId, enModemId);
    if (VOS_NULL_PTR == pstFcRabMappingInfo)
    {
        return VOS_ERR;
    }

    /* �����RabId����Ϣ�������Fc���Ѿ������ڲ���Ҫ���ص�RabId����������� */
    ulRabMask       = (1 << ucRabId);

	pstFcRabMappingInfo->ulIncludeRabMask  &= (~ulRabMask);

    if (0 != pstFcRabMappingInfo->ulNoFcRabMask)
    {
        pstFcRabMappingInfo->ulNoFcRabMask       &= (~ulRabMask);
        if ((0 == pstFcRabMappingInfo->ulNoFcRabMask) && (0 != pstFcRabMappingInfo->ulIncludeRabMask))
        {
            FC_POINT_SetFc(FC_POLICY_MASK_CDS, pstFcRabMappingInfo->enFcId);
        }
    }

    /* ���ɾ����RabId�󣬸�FC��û��������Ϣ����ɾ����FC ID��Ӧ����Ϣ */
    if (0 == pstFcRabMappingInfo->ulIncludeRabMask)
    {
        FC_CDS_DelFcId(pstFcRabMappingInfo->enFcId, enModemId);
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_Init
 ��������  : AT Client���س�ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��20��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_Init( VOS_VOID )
{
    VOS_MemSet(g_astFcRabMappingInfoSet, 0, sizeof(g_astFcRabMappingInfoSet));

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_UpProcess
 ��������  : CDS���ز���
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_UpProcess( VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_STRU               *pstFcRabMappingInfo;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_CDS != (FC_POLICY_MASK_CDS & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_CST_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }

    if ( MODEM_ID_BUTT <= enModemId )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_CDS_UpProcess ModemId Is Invalid %d\n", enModemId);
        return VOS_ERR;
    }

    /*====================================*//* ͨ��RAB id������ Client����ʵ�� */
    pstFcRabMappingInfo         = FC_CDS_GetFcInfo(ucRabId, enModemId);
    if (VOS_NULL_PTR == pstFcRabMappingInfo)
    {
        return VOS_ERR;
    }


    /*====================================*//* �����Fc����ʵ���Ѿ����أ������账����ֱ�ӷ��� */
    if (0 == pstFcRabMappingInfo->ulNoFcRabMask)
    {
        return VOS_OK;
    }


    /*====================================*/
    /* ȥ����FC��Ӧ��û�����ص�RAB���룬���Ϊ0����������RAB��Ҫ�����أ���FC�������� */
    pstFcRabMappingInfo->ulNoFcRabMask   &= (~((VOS_UINT32)1 << ucRabId));
    if (0 == pstFcRabMappingInfo->ulNoFcRabMask)
    {
        FC_POINT_SetFc(FC_POLICY_MASK_CDS, pstFcRabMappingInfo->enFcId);
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_CDS_DownProcess
 ��������  : CDS������ز���
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_CDS_DownProcess( VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16 enModemId )
{
    FC_RAB_MAPPING_INFO_STRU                  *pstFcRabMappingInfo;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_CDS != (FC_POLICY_MASK_CDS & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_CDS_DownProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }

    if ( MODEM_ID_BUTT <= enModemId )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_CDS_DownProcess ModemId Is Invalid %d\n", enModemId);
        return VOS_ERR;
    }

    /*====================================*//* ͨ��RAB id������ FCʵ�� */
    pstFcRabMappingInfo                 = FC_CDS_GetFcInfo(ucRabId, enModemId);
    if (VOS_NULL_PTR == pstFcRabMappingInfo)
    {
        return VOS_ERR;
    }


    /*====================================*/
    /* �����FCʵ���Ѿ�������أ������账����ֱ�ӷ��� */
    if (0 != pstFcRabMappingInfo->ulNoFcRabMask)
    {
        pstFcRabMappingInfo->ulNoFcRabMask   |= ((VOS_UINT32)1 << ucRabId);
        return VOS_OK;
    }


    /*====================================*/
    /* ����v��Ӧ��û�����ص�RAB���룬���Ϊ0�����FCʵ��s��Ҫ���н����� */
    pstFcRabMappingInfo->ulNoFcRabMask   |= ((VOS_UINT32)1 << ucRabId);
    FC_POINT_ClrFc(FC_POLICY_MASK_CDS, pstFcRabMappingInfo->enFcId);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_GPRS_UpProcess
 ��������  : ����GPRS��������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_GPRS_UpProcess( VOS_VOID )
{
    FC_POLICY_STRU                      *pPolicy;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_GPRS != (FC_POLICY_MASK_GPRS & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_GPRS_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }


    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_GPRS_MODEM_0);
    FC_POLICY_UpToTargetPri(pPolicy, pPolicy->enHighestPri);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_GPRS_DownProcess
 ��������  : ����GPRS�����������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_GPRS_DownProcess( VOS_VOID )
{
    FC_POLICY_STRU                      *pPolicy;


    /*====================================*//* ʹ�ܼ�� */
    if ( FC_POLICY_MASK_GPRS != (FC_POLICY_MASK_GPRS & g_stFcCfg.ulFcEnbaleMask) )
    {
        /* �ڴ�����δʹ�� */
        FC_LOG1(PS_PRINT_INFO, "FC_GPRS_UpProcess, INFO, MEM FlowCtrl is disabled %d \n",
                (VOS_INT32)g_stFcCfg.ulFcEnbaleMask);
        return VOS_OK;
    }

    /* ��ȡCPU���ز���ʵ�壬������ͨ�����ز��� */
    pPolicy = FC_POLICY_Get(FC_PRIVATE_POLICY_ID_GPRS_MODEM_0);
    FC_POLICY_DownToTargetPri(pPolicy, FC_PRI_NULL);

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_ChannelMapCreate
 ��������  : A������ģ���ṩ�ӿڸ�AT����ͨ����RABIDӳ���ϵ
 �������  : FC_ID_ENUM_UINT32 enFcId   ͨ����Ӧ��FC ID
             VOS_UINT8 ucRabId          ���ض�Ӧ��RabId
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_ChannelMapCreate(FC_ID_ENUM_UINT32 enFcId, VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16  enModemId)
{
    FC_ADD_RAB_FCID_MAP_IND_STRU       *pstMsg;
    VOS_UINT32                          ulResult;


    /* ������飬RabId��ΧΪ[5,15] */
    if ( (FC_UE_MIN_RAB_ID > ucRabId) || (FC_UE_MAX_RAB_ID < ucRabId) )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_ChannelMapCreate RabId Is Invalid %d\n", ucRabId);
        return;
    }

    if ( MODEM_ID_BUTT <= enModemId )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_ChannelMapCreate ModemId Is Invalid %d\n", enModemId);
        return;
    }

    /* ������Ϣ */
    pstMsg = (FC_ADD_RAB_FCID_MAP_IND_STRU *)VOS_AllocMsg(UEPS_PID_FLOWCTRL_A, \
                                    sizeof(FC_ADD_RAB_FCID_MAP_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstMsg)
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ChannelMapCreate, Alloc Msg Fail\n");
        return;
    }

    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL_A;
    pstMsg->usMsgName       = ID_FC_ADD_RAB_FCID_MAP_IND;
    pstMsg->enFcId          = enFcId;
    pstMsg->ucRabId         = ucRabId;
    pstMsg->enModemId       = enModemId;

    ulResult = VOS_SendMsg(UEPS_PID_FLOWCTRL_A, pstMsg);

    if (VOS_OK != ulResult)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_ChannelMapCreate, Send Msg Fail\n");
        return;
    }
}


/*****************************************************************************
 �� �� ��  : FC_ChannelMapDelete
 ��������  : ����һ��RABID�ͷ�ʱ��ATͨ���ýӿ�ɾ������ģ��ӳ���ϵ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID  FC_ChannelMapDelete( VOS_UINT8 ucRabId, MODEM_ID_ENUM_UINT16  enModemId )
{
    FC_DEL_RAB_FCID_MAP_IND_STRU       *pstMsg;
    VOS_UINT32                          ulResult;


    /* ������飬RabId��ΧΪ[5,15] */
    if ( (FC_UE_MIN_RAB_ID > ucRabId) || (FC_UE_MAX_RAB_ID < ucRabId) )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_ChannelMapDelete RabId Is Invalid %d\n", ucRabId);
        return;
    }

    if ( MODEM_ID_BUTT <= enModemId )
    {
        FC_LOG1(PS_PRINT_WARNING, "FC_ChannelMapDelete ModemId Is Invalid %d\n", enModemId);
        return;
    }

    /* ������Ϣ */
    pstMsg = (FC_DEL_RAB_FCID_MAP_IND_STRU *)VOS_AllocMsg(UEPS_PID_FLOWCTRL_A, \
                                    sizeof(FC_DEL_RAB_FCID_MAP_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstMsg)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_ChannelMapDelete, Alloc Msg Fail\n");
        return;
    }

    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL_A;
    pstMsg->usMsgName       = ID_FC_DEL_RAB_FCID_MAP_IND;
    pstMsg->ucRabId         = ucRabId;
    pstMsg->enModemId       = enModemId;

    ulResult = VOS_SendMsg(UEPS_PID_FLOWCTRL_A, pstMsg);

    if (VOS_OK != ulResult)
    {
        FC_LOG(PS_PRINT_ERROR, "FC_ChannelMapDelete, Send Msg Fail\n");
        return;
    }
}


/*****************************************************************************
 �� �� ��  : FC_RcvCstMsg
 ��������  : ����CST��Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��12��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_RcvCstMsg( MsgBlock * pMsg )
{
    switch ( FC_GET_MSG_NAME32(pMsg) )
    {
        case ID_CST_FC_SET_FLOWCTRL_REQ:
            FC_CST_UpProcess( ((CST_FC_SET_FLOWCTRL_REQ_STRU *)pMsg)->ucRabId );
            break;

        case ID_CST_FC_STOP_FLOWCTRL_REQ:
            FC_CST_DownProcess( ((CST_FC_STOP_FLOWCTRL_REQ_STRU *)pMsg)->ucRabId );
            break;

        default:
            break;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_RcvCdsMsg
 ��������  : ���մ���CDS��Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��12��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_RcvCdsMsg( MsgBlock * pMsg )
{
    switch ( FC_GET_MSG_NAME32(pMsg) )
    {
        case ID_CDS_FC_STOP_CHANNEL_IND:
            FC_CDS_UpProcess( ((CDS_FC_STOP_CHANNEL_IND_STRU *)pMsg)->ucRabId,
                               ((CDS_FC_STOP_CHANNEL_IND_STRU *)pMsg)->enModemId );
            break;

        case ID_CDS_FC_START_CHANNEL_IND:
            FC_CDS_DownProcess( ((CDS_FC_START_CHANNEL_IND_STRU *)pMsg)->ucRabId,
                                 ((CDS_FC_START_CHANNEL_IND_STRU *)pMsg)->enModemId );
            break;

        default:
            break;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_ACORE_PointReg
 ��������  : ��ʼ��ʱ��ע��һЩ�̶������ص�
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��22��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_ACORE_RegPoint( VOS_VOID )
{
/* V9R1��Ŀ������ʹ�ú꿪���ж�FCģ���Ƿ���Ҫ�������ص�ע�� */
#if(FEATURE_ON == FEATURE_NFEXT)
#if(FEATURE_ON == FEATURE_ACPU_FC_POINT_REG)
    FC_REG_POINT_STRU                   stFcRegPoint;

    PS_MEM_SET(&stFcRegPoint, 0, sizeof(FC_REG_POINT_STRU));

    /* CPU�������ص�ע�� */
    stFcRegPoint.enPolicyId = FC_POLICY_ID_CPU_A;
    stFcRegPoint.enFcPri    = FC_PRI_FOR_BRIDGE_FORWARD_DISCARD;
    stFcRegPoint.enFcId     = FC_ID_BRIDGE_FORWARD_DISACRD;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)NFExt_BrSetFlowCtrl;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)NFExt_BrStopFlowCtrl;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

    stFcRegPoint.enPolicyId = FC_POLICY_ID_CPU_A;
    stFcRegPoint.enFcPri    = FC_PRI_HIGHEST;
    stFcRegPoint.enFcId     = FC_ID_USB_ETH;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)USB_ETH_DrvSetRxFlowCtrl;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)USB_ETH_DrvClearRxFlowCtrl;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

    stFcRegPoint.enPolicyId = FC_POLICY_ID_CPU_A;
    stFcRegPoint.enFcPri    = FC_PRI_HIGHEST;
    stFcRegPoint.enFcId     = FC_ID_WIFI_ETH;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)DRV_WIFI_SET_RX_FCTL;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)DRV_WIFI_CLR_RX_FCTL;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

    /* �ڴ����ص�ע�� */
    stFcRegPoint.enPolicyId = FC_POLICY_ID_MEM;
    stFcRegPoint.enFcPri    = FC_PRI_FOR_BRIDGE_FORWARD_DISCARD;
    stFcRegPoint.enFcId     = FC_ID_BRIDGE_FORWARD_DISACRD;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)NFExt_BrSetFlowCtrl;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)NFExt_BrStopFlowCtrl;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

    stFcRegPoint.enPolicyId = FC_POLICY_ID_MEM;
    stFcRegPoint.enFcPri    = FC_PRI_FOR_MEM_LEV_4;
    stFcRegPoint.enFcId     = FC_ID_USB_ETH;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)USB_ETH_DrvSetRxFlowCtrl;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)USB_ETH_DrvClearRxFlowCtrl;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

    stFcRegPoint.enPolicyId = FC_POLICY_ID_MEM;
    stFcRegPoint.enFcPri    = FC_PRI_FOR_MEM_LEV_4;
    stFcRegPoint.enFcId     = FC_ID_WIFI_ETH;
    stFcRegPoint.ulParam1   = 0;
    stFcRegPoint.ulParam2   = 0;
    stFcRegPoint.pSetFunc   = (FC_SET_FUNC)DRV_WIFI_SET_RX_FCTL;
    stFcRegPoint.pClrFunc   = (FC_SET_FUNC)DRV_WIFI_CLR_RX_FCTL;
    stFcRegPoint.pRstFunc   = VOS_NULL_PTR;
    FC_POINT_Reg(&stFcRegPoint);

#endif
#endif

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_ACORE_RcvTimerMsg
 ��������  : ���մ�����ʱ����Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��19��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_ACORE_RcvTimerMsg(REL_TIMER_MSG *pTimerMsg)
{
    switch (pTimerMsg->ulName)
    {
        case FC_TIMER_STOP_CPU_ATTEMPT:
            FC_CPUA_StopFlowCtrl();
            break;

        default:
            break;
    }

    return VOS_OK;

}


/*****************************************************************************
 �� �� ��  : FC_IntraMsgProc
 ��������  : ����������Ϣ��������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��12��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_ACORE_RcvIntraMsg( MsgBlock * pMsg )
{

    switch( FC_GET_MSG_NAME16(pMsg) )
    {
        case ID_FC_REG_POINT_IND:
            FC_POINT_Reg( &(((FC_REG_POINT_IND_STRU *)pMsg)->stFcPoint) );
            break;

        case ID_FC_DEREG_POINT_IND:
            FC_POINT_DeReg( ((FC_DEREG_POINT_IND_STRU *)pMsg)->enFcId,
                            ((FC_DEREG_POINT_IND_STRU *)pMsg)->enModemId );
            break;

        case ID_FC_CHANGE_POINT_IND:
            FC_POINT_Change( ((FC_CHANGE_POINT_IND_STRU *)pMsg)->enFcId,
                             ((FC_CHANGE_POINT_IND_STRU *)pMsg)->enPolicyId,
                             ((FC_CHANGE_POINT_IND_STRU *)pMsg)->enPri,
                             ((FC_CHANGE_POINT_IND_STRU *)pMsg)->enModemId);
            break;

        case ID_FC_CPU_A_OVERLOAD_IND:
            FC_CPUA_UpProcess();
            break;

        case ID_FC_CPU_A_UNDERLOAD_IND:
            FC_CPUA_DownProcess();
            break;

        case ID_FC_SET_GPRS_FLOWCTRL_IND:
            FC_GPRS_UpProcess();
            break;

        case ID_FC_STOP_GPRS_FLOWCTRL_IND:
            FC_GPRS_DownProcess();
            break;

        case ID_FC_ADD_RAB_FCID_MAP_IND:
            FC_CDS_AddRab(((FC_ADD_RAB_FCID_MAP_IND_STRU *)pMsg)->enFcId,
                ((FC_ADD_RAB_FCID_MAP_IND_STRU *)pMsg)->ucRabId,
                ((FC_ADD_RAB_FCID_MAP_IND_STRU *)pMsg)->enModemId);
            break;

        case ID_FC_DEL_RAB_FCID_MAP_IND:
            FC_CDS_DelRab(( (FC_DEL_RAB_FCID_MAP_IND_STRU *)pMsg)->ucRabId,
                          ( (FC_DEL_RAB_FCID_MAP_IND_STRU *)pMsg)->enModemId);
            break;

        case ID_FC_MEM_UP_TO_TARGET_PRI_IND:
            FC_POLICY_UpToTargetPri( &(g_astFcPolicy[FC_POLICY_ID_MEM]),
                ((FC_MEM_UP_TO_TARGET_PRI_IND_STRU *)pMsg)->enTargetPri );
            break;

        case ID_FC_MEM_DOWN_TO_TARGET_PRI_IND:
            FC_POLICY_DownToTargetPri( &(g_astFcPolicy[FC_POLICY_ID_MEM]),
                ((FC_MEM_DOWN_TO_TARGET_PRI_IND_STRU *)pMsg)->enTargetPri );
            break;

        /* V9R1�в���Ҫ������Щ��Ϣ */
#if(FEATURE_OFF == FEATURE_ACPU_FC_POINT_REG)
        case ID_FC_ACORE_CRESET_START_IND:
            FC_ACORE_CResetProc(FC_ACORE_CRESET_BEFORE_RESET);
            break;

        case ID_FC_ACORE_CRESET_END_IND:
            FC_ACORE_CResetProc(FC_ACORE_CRESET_AFTER_RESET);
            break;

        case ID_FC_ACORE_CRESET_START_RSP:
            FC_ACORE_CResetRcvStartRsp();
            break;
#endif

        default:
            break;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_MsgProc
 ��������  : ������Ϣ��������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��12��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_ACORE_MsgProc( MsgBlock * pMsg )
{
    switch (pMsg->ulSenderPid)
    {
        case UEPS_PID_FLOWCTRL_A:
        case UEPS_PID_FLOWCTRL_C:
            FC_ACORE_RcvIntraMsg(pMsg);
            break;

        case UEPS_PID_CST:
            FC_RcvCstMsg(pMsg);
            break;

        case UEPS_PID_CDS:
            FC_RcvCdsMsg(pMsg);
            break;

        case VOS_PID_TIMER:
            FC_ACORE_RcvTimerMsg( (REL_TIMER_MSG *)pMsg );
            break;

        default:
            break;
    }

    return VOS_OK;
} /* FC_MsgProc */



/*****************************************************************************
 �� �� ��  : FC_Init
 ��������  : ����ģ���ʼ��
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��7��
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32  FC_ACORE_Init( VOS_VOID )
{
    VOS_UINT32                          ulResult;
    VOS_INT                             iRet;


    ulResult = FC_CommInit();

    if ( VOS_OK != ulResult )
    {
        FC_LOG(PS_PRINT_ERROR, "FC_Init, ERROR, FC_CommInit return error!\n");
        return VOS_ERR;
    }

    ulResult    = FC_CPUA_Init();
    if ( VOS_OK != ulResult )
    {
        FC_LOG(PS_PRINT_ERROR, "FC_ACORE_Init, ERROR, FC_CPUA_Init return error!\n");
        return VOS_ERR;
    }

    FC_CDS_Init();

    FC_MEM_Init();

    FC_ACORE_RegPoint();

    FC_DrvAssemInit();

    /* �����ź���������C�˵�����λʱ��֪ͨ����FcACore�Ļص���������� */
    if ( VOS_OK != VOS_SmBCreate("FcACoreCResetDoneSem", 0, VOS_SEMA4_FIFO, &g_ulFcACoreCResetDoneSem) )
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ACORE_Init, Creat Sem Fail\n");
        return VOS_ERR;
    }

    /* ע��ص�����������C�˸�λ�ӿ��� */
    iRet    = DRV_CCORERESET_REGCBFUNC ("TTF_FcACore", FC_ACORE_CResetCallback, 0, FC_ACORE_CRESET_CALLBACK_PRIOR);
    if ( VOS_OK != iRet )
    {
        FC_LOG(PS_PRINT_ERROR, "FC_ACORE_Init, ERROR, DRV_CCORERESET_REGCBFUNC fail!\n");
        return VOS_ERR;
    }

    return VOS_OK;
}


/*****************************************************************************
 �� �� ��  : FC_FidInit
 ��������  : ����FID��ʼ������
 �������  : enum VOS_INIT_PHASE_DEFINE enPhase
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2011��12��14��
    �޸�����   : �����ɺ���
*****************************************************************************/
VOS_UINT32 FC_ACORE_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32  ulResult = VOS_ERR;


    switch (enPhase)
    {
        case   VOS_IP_LOAD_CONFIG:
#ifndef DMT
            ulResult = VOS_RegisterPIDInfo(UEPS_PID_FLOWCTRL,
                                           (Init_Fun_Type)VOS_NULL_PTR,
                                           (Msg_Fun_Type)FC_ACORE_MsgProc);
#else

            ulResult = VOS_RegisterPIDInfo(UEPS_PID_FLOWCTRL_A,
                                           (Init_Fun_Type)VOS_NULL_PTR,
                                           (Msg_Fun_Type)FC_ACORE_MsgProc);
#endif
            if (VOS_OK != ulResult)
            {
                FC_LOG(PS_PRINT_ERROR, "FC_FidInit, VOS_RegisterPIDInfo Fail\n");
                return VOS_ERR;
            }
#ifndef DMT
            ulResult = VOS_RegisterTaskPrio(UEPS_FID_FLOWCTRL, TTF_FLOW_CTRL_TASK_PRIO);
#else

            ulResult = VOS_RegisterTaskPrio(UEPS_FID_FLOWCTRL_A, TTF_FLOW_CTRL_TASK_PRIO);
#endif
            if (VOS_OK != ulResult)
            {
                FC_LOG(PS_PRINT_ERROR, "FC_FidInit, OS_RegisterTaskPrio Fail\n");
                return VOS_ERR;
            }

            ulResult = FC_ACORE_Init();

            if (VOS_OK != ulResult)
            {
                FC_LOG(PS_PRINT_ERROR, "FC_FidInit, Call FC_Init return error!\n");
                return VOS_ERR;
            }

            break;
        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
            break;
        default:
            break;
    }

    return VOS_OK;
}


/*lint -e565 -e718 -e746 -e40 -e64 -e115 -e86 -e63*//* ֱ�ӵ�����Linux�ں˺���������PC-lint */
/*****************************************************************************
 �� �� ��  : FC_SetPidPri
 ��������  : ��ѯPID�����ȼ�
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��2��16��
    �޸�����   : �����ɺ���

*****************************************************************************/
int  FC_GetPidPri( pid_t pid )
{
    struct task_struct                 *p;


    p = pid_task(find_vpid(pid), PIDTYPE_PID);

    if (NULL == p)
    {
        vos_printf("invalid pid %d\r\n", pid);
        return -1;
    }

    vos_printf ("pid %d, pri %d\r\n", pid, p->rt_priority);


    return (int)(p->rt_priority);
}


/*****************************************************************************
 �� �� ��  : FC_SetPidPri
 ��������  : ����PID���ȼ�
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��2��16��
    �޸�����   : �����ɺ���

*****************************************************************************/
int  FC_SetPidPri( pid_t pid, int pri )
{
    struct sched_param                  stParam;
    struct task_struct                 *p;


    p = pid_task(find_vpid(pid), PIDTYPE_PID);

    if (NULL == p)
    {
        vos_printf("invalid pid %d\r\n", pid);
        return -1;
    }

    stParam.sched_priority  = pri;

    sched_setscheduler(p, -1, &stParam);

    vos_printf ("pid %d, pri %d\r\n", pid, p->rt_priority);

    return (int)(p->rt_priority);
}



/*****************************************************************************
 �� �� ��  : FC_SetPidScheduler
 ��������  : ��������ĵ��Ȳ���
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��2��17��
    �޸�����   : �����ɺ���

*****************************************************************************/
int  FC_SetPidScheduler( pid_t pid, int policy )
{
    struct sched_param                  stParam;
    struct task_struct                 *p;


    p = pid_task(find_vpid(pid), PIDTYPE_PID);

    if (NULL == p)
    {
        vos_printf("invalid pid %d\r\n", pid);
        return -1;
    }

    stParam.sched_priority  = (int)p->rt_priority;

    sched_setscheduler(p, policy, &stParam);

    vos_printf ("pid %d, schedler %d, pri %d\r\n", pid, policy, p->rt_priority);

    return (int)(p->rt_priority);
}
/*lint +e565 +e718 +e746 +e40 +e64 +e115 +e86 +e63*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
