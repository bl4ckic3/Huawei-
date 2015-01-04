/******************************************************************************

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : drv_debug_led.h
  �� �� ��   : ����
  ��������   : 2013��2��2��
  ����޸�   :
  ��������   : drv_debug_led.h ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��2��2��
    �޸�����   : �����ļ�

******************************************************************************/

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "drv_global.h"


#ifndef __DRV_DEBUG_LED_H__
#define __DRV_DEBUG_LED_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



/*****************************************************************************
  2 �궨��
*****************************************************************************/
/*V9R1SFT SC����ַ��BBITƽ̨��SC*/
#define SC_BASE    SOC_SCTRL_BASE_ADDR

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
typedef enum
{
    DBLED_DRV1 = 0,        /* DRV���Ե�1 */
    DBLED_DRV2,            /* DRV���Ե�2 */
    DBLED_DSP1 = 2,        /* DSP���Ե�1 */
    DBLED_DSP2,            /* DSP���Ե�2 */
    DBLED_RESERVED1 = 4,   /* �������Ե�1 */
    DBLED_RESERVED2,       /* �������Ե�2 */
    DBLED_MAX
}DBLED_ID_E;

/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  6 STRUCT����
*****************************************************************************/


/*****************************************************************************
  7 UNION����
*****************************************************************************/


/*****************************************************************************
  8 OTHERS����
*****************************************************************************/


/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
/*****************************************************************************
 �� �� ��  : DRV_DBLED_ON
 ��������  : ���ԵƵƵ�����������
 �������  : ledId:���ID,ȡֵ0~5
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��7��25��
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int DRV_DBLED_ON(unsigned int ledId);

/*****************************************************************************
 �� �� ��  : DRV_DBLED_OFF
 ��������  : ���Ե�Ϩ��������
 �������  : ledId:���ID,ȡֵ0~5
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��7��25��
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int DRV_DBLED_OFF(unsigned int ledId);

/*****************************************************************************
 �� �� ��  : DRV_SYSCTRL_REG_SET
 ��������  : DRV�ṩͳһ��sysctrlд�ӿں���
 �������  : regAddr��SC�Ĵ���ƫ�Ƶ�ַ��
             setData: SC�Ĵ���ֵ��
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��7��25��
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int DRV_SYSCTRL_REG_SET(unsigned int regAddr, unsigned int setData);

/*****************************************************************************
 �� �� ��  : DRV_SYSCTRL_REG_GET
 ��������  : DRV�ṩͳһ��sysctrl���ӿں���
 �������  : regAddr�� SC�Ĵ���ƫ�Ƶ�ַ��
             getData������SC�Ĵ���ֵ��ָ�롣
 �������  : ��
 �� �� ֵ  : int
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��7��25��
    �޸�����   : �����ɺ���

*****************************************************************************/
extern int DRV_SYSCTRL_REG_GET(unsigned int regAddr, unsigned int * getData);





#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of drv_debug_led.h */
