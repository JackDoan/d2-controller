/**
 * \brief Header file for PIC32CM MC00 
 *
 * Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software and any derivatives
 * exclusively with Microchip products. It is your responsibility to comply with third party license
 * terms applicable to your use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT
 * EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#ifndef _SAM_PERIPHERAL_LEGACY_H_
#define _SAM_PERIPHERAL_LEGACY_H_

/*sercom.h*/
#define SERCOM_I2CM_CTRLA_SDAHOLD_DIS_Val									SERCOM_I2CM_CTRLA_SDAHOLD_DISABLE_Val
#define SERCOM_I2CM_CTRLA_SDAHOLD_DIS										SERCOM_I2CM_CTRLA_SDAHOLD_DISABLE									
#define SERCOM_I2CM_CTRLA_SPEED_SM_Val										SERCOM_I2CM_CTRLA_SPEED_STANDARD_AND_FAST_MODE						
#define SERCOM_I2CM_CTRLA_SPEED_FMP_Val										SERCOM_I2CM_CTRLA_SPEED_FASTPLUS_MODE_Val							
#define SERCOM_I2CM_CTRLA_SPEED_HS_Val										SERCOM_I2CM_CTRLA_SPEED_HIGH_SPEED_MODE_Val							
#define SERCOM_I2CM_CTRLA_SPEED_SM											SERCOM_I2CM_CTRLA_SPEED_STANDARD_AND_FAST_MODE						
#define SERCOM_I2CM_CTRLA_SPEED_FMP											SERCOM_I2CM_CTRLA_SPEED_FASTPLUS_MODE								
#define SERCOM_I2CM_CTRLA_SPEED_HS											SERCOM_I2CM_CTRLA_SPEED_HIGH_SPEED_MODE								
#define SERCOM_I2CM_CTRLA_INACTOUT_DIS_Val									SERCOM_I2CM_CTRLA_INACTOUT_DISABLE_Val								
#define SERCOM_I2CM_CTRLA_INACTOUT_DIS										SERCOM_I2CM_CTRLA_INACTOUT_DISABLE									
#define SERCOM_I2CS_CTRLA_SDAHOLD_DIS_Val									SERCOM_I2CS_CTRLA_SDAHOLD_DISABLE_Val								
#define SERCOM_I2CS_CTRLA_SDAHOLD_DIS										SERCOM_I2CS_CTRLA_SDAHOLD_DISABLE									
#define SERCOM_I2CS_CTRLA_SPEED_SM_Val										SERCOM_I2CS_CTRLA_SPEED_STANDARD_AND_FAST_MODE_Val					
#define SERCOM_I2CS_CTRLA_SPEED_FMP_Val										SERCOM_I2CS_CTRLA_SPEED_FASTPLUS_MODE_Val							
#define SERCOM_I2CS_CTRLA_SPEED_HS_Val										SERCOM_I2CS_CTRLA_SPEED_HIGH_SPEED_MODE_Val							
#define SERCOM_I2CS_CTRLA_SPEED_SM											SERCOM_I2CS_CTRLA_SPEED_STANDARD_AND_FAST_MODE						
#define SERCOM_I2CS_CTRLA_SPEED_FMP											SERCOM_I2CS_CTRLA_SPEED_FASTPLUS_MODE								
#define SERCOM_I2CS_CTRLA_SPEED_HS											SERCOM_I2CS_CTRLA_SPEED_HIGH_SPEED_MODE								
#define   SERCOM_SPIM_CTRLA_DOPO_0x0_Val									SERCOM_SPIM_CTRLA_DOPO_PAD0_Val									
#define   SERCOM_SPIM_CTRLA_DOPO_0x1_Val									SERCOM_SPIM_CTRLA_DOPO_PAD1_Val									
#define   SERCOM_SPIM_CTRLA_DOPO_0x2_Val									SERCOM_SPIM_CTRLA_DOPO_PAD2_Val									
#define   SERCOM_SPIM_CTRLA_DOPO_0x3_Val									SERCOM_SPIM_CTRLA_DOPO_PAD3_Val									
#define SERCOM_SPIM_CTRLA_DOPO_0x0											SERCOM_SPIM_CTRLA_DOPO_PAD0											
#define SERCOM_SPIM_CTRLA_DOPO_0x1											SERCOM_SPIM_CTRLA_DOPO_PAD1											
#define SERCOM_SPIM_CTRLA_DOPO_0x2											SERCOM_SPIM_CTRLA_DOPO_PAD2											
#define SERCOM_SPIM_CTRLA_DOPO_0x3											SERCOM_SPIM_CTRLA_DOPO_PAD3											
#define   SERCOM_SPIS_CTRLA_DOPO_0x0_Val									SERCOM_SPIS_CTRLA_DOPO_PAD0_Val									
#define   SERCOM_SPIS_CTRLA_DOPO_0x1_Val									SERCOM_SPIS_CTRLA_DOPO_PAD1_Val									
#define   SERCOM_SPIS_CTRLA_DOPO_0x2_Val									SERCOM_SPIS_CTRLA_DOPO_PAD2_Val									
#define   SERCOM_SPIS_CTRLA_DOPO_0x3_Val									SERCOM_SPIS_CTRLA_DOPO_PAD3_Val									
#define SERCOM_SPIS_CTRLA_DOPO_0x0											SERCOM_SPIS_CTRLA_DOPO_PAD0											
#define SERCOM_SPIS_CTRLA_DOPO_0x1											SERCOM_SPIS_CTRLA_DOPO_PAD1											
#define SERCOM_SPIS_CTRLA_DOPO_0x2											SERCOM_SPIS_CTRLA_DOPO_PAD2											
#define SERCOM_SPIS_CTRLA_DOPO_0x3											SERCOM_SPIS_CTRLA_DOPO_PAD3											
#define   SERCOM_USART_EXT_CTRLA_FORM_USART_FRAME_LINBRKGEN_Val				SERCOM_USART_EXT_CTRLA_FORM_USART_FRAME_LIN_MASTER_MODE_Val		
#define SERCOM_USART_EXT_CTRLA_FORM_USART_FRAME_LINBRKGEN					SERCOM_USART_EXT_CTRLA_FORM_USART_FRAME_LIN_MASTER_MODE				
#define   SERCOM_USART_INT_CTRLA_FORM_USART_FRAME_LINBRKGEN_Val				SERCOM_USART_INT_CTRLA_FORM_USART_FRAME_LIN_MASTER_MODE_Val		
#define SERCOM_USART_INT_CTRLA_FORM_USART_FRAME_LINBRKGEN					SERCOM_USART_INT_CTRLA_FORM_USART_FRAME_LIN_MASTER_MODE				
#define   SERCOM_SPIM_CTRLB_AMODE_2ADDRS_Val								SERCOM_SPIM_CTRLB_AMODE_2_ADDRESSES_Val							
#define SERCOM_SPIM_CTRLB_AMODE_2ADDRS										SERCOM_SPIM_CTRLB_AMODE_2_ADDRESSES									
#define   SERCOM_SPIS_CTRLB_AMODE_2ADDRS_Val								SERCOM_SPIS_CTRLB_AMODE_2_ADDRESSES_Val							
#define SERCOM_SPIS_CTRLB_AMODE_2ADDRS										SERCOM_SPIS_CTRLB_AMODE_2_ADDRESSES									
#define   SERCOM_USART_EXT_CTRLB_LINCMD_CMD0_Val							SERCOM_USART_EXT_CTRLB_LINCMD_NONE_Val							
#define   SERCOM_USART_EXT_CTRLB_LINCMD_CMD1_Val							SERCOM_USART_EXT_CTRLB_LINCMD_SOFTWARE_CONTROL_TRANSMIT_CMD_Val	
#define   SERCOM_USART_EXT_CTRLB_LINCMD_CMD2_Val							SERCOM_USART_EXT_CTRLB_LINCMD_AUTO_TRANSMIT_CMD_Val				
#define SERCOM_USART_EXT_CTRLB_LINCMD_CMD0									SERCOM_USART_EXT_CTRLB_LINCMD_NONE									
#define SERCOM_USART_EXT_CTRLB_LINCMD_CMD1									SERCOM_USART_EXT_CTRLB_LINCMD_SOFTWARE_CONTROL_TRANSMIT_CMD			
#define SERCOM_USART_EXT_CTRLB_LINCMD_CMD2									SERCOM_USART_EXT_CTRLB_LINCMD_AUTO_TRANSMIT_CMD						
#define   SERCOM_USART_INT_CTRLB_LINCMD_CMD0_Val							SERCOM_USART_INT_CTRLB_LINCMD_NONE_Val							
#define   SERCOM_USART_INT_CTRLB_LINCMD_CMD1_Val							SERCOM_USART_INT_CTRLB_LINCMD_SOFTWARE_CONTROL_TRANSMIT_CMD_Val	
#define   SERCOM_USART_INT_CTRLB_LINCMD_CMD2_Val							SERCOM_USART_INT_CTRLB_LINCMD_AUTO_TRANSMIT_CMD_Val				
#define SERCOM_USART_INT_CTRLB_LINCMD_CMD0									SERCOM_USART_INT_CTRLB_LINCMD_NONE									
#define SERCOM_USART_INT_CTRLB_LINCMD_CMD1									SERCOM_USART_INT_CTRLB_LINCMD_SOFTWARE_CONTROL_TRANSMIT_CMD			
#define SERCOM_USART_INT_CTRLB_LINCMD_CMD2									SERCOM_USART_INT_CTRLB_LINCMD_AUTO_TRANSMIT_CMD						

/*port.h*/
// PORT_X_Msk defaluts to 0xFFFFFFFF in PIC32C and 0x00000000 in SAM

/*pac.h*/
#define PAC_INTFLAGAHB_APBB_Pos			PAC_INTFLAGAHB_HPB1_Pos			
#define PAC_INTFLAGAHB_APBB_Msk			PAC_INTFLAGAHB_HPB1_Msk			
#define PAC_INTFLAGAHB_APBB(value)		PAC_INTFLAGAHB_HPB1(value)		
#define PAC_INTFLAGAHB_APBA_Pos			PAC_INTFLAGAHB_HPB0_Pos  		
#define PAC_INTFLAGAHB_APBA_Msk			PAC_INTFLAGAHB_HPB0_Msk			
#define PAC_INTFLAGAHB_APBA(value)		PAC_INTFLAGAHB_HPB0(value) 		
#define PAC_INTFLAGAHB_APBC_Pos			PAC_INTFLAGAHB_HPB2_Pos			
#define PAC_INTFLAGAHB_APBC_Msk			PAC_INTFLAGAHB_HPB2_Msk			
#define PAC_INTFLAGAHB_APBC(value)		PAC_INTFLAGAHB_HPB2(value)		
//PAC_STATUSC_RESETVALUE 0x20000000 in SAM and 0x0 in PIC32C

/*oscctrl.h*/
#define OSCCTRL_STATUS_DPLLLTO_Pos				OSCCTRL_STATUS_DPLLTO_Pos				
#define OSCCTRL_STATUS_DPLLLTO_Msk				OSCCTRL_STATUS_DPLLTO_Msk				
#define OSCCTRL_STATUS_DPLLLTO(value)			OSCCTRL_STATUS_DPLLTO(value)			
#define   OSCCTRL_DPLLCTRLB_FILTER_Default_Val	OSCCTRL_DPLLCTRLB_FILTER_DEFAULT_Val	
#define OSCCTRL_DPLLCTRLB_FILTER_Default		OSCCTRL_DPLLCTRLB_FILTER_DEFAULT		
#define   OSCCTRL_DPLLCTRLB_LTIME_Default_Val	OSCCTRL_DPLLCTRLB_LTIME_DEFAULT_Val	
#define OSCCTRL_DPLLCTRLB_LTIME_Default			OSCCTRL_DPLLCTRLB_LTIME_DEFAULT			

/*osc32kctrl.h*/
//#define   OSC32KCTRL_XOSC32K_STARTUP_CYCLE262144_Val	NOT DEFINED IN PIC32C

/*nvmctrl.h*/
#define   NVMCTRL_CTRLA_CMD_DFER_Val		NVMCTRL_CTRLA_CMD_RWWEEER_Val		
#define   NVMCTRL_CTRLA_CMD_DFWP_Val		NVMCTRL_CTRLA_CMD_RWWEEWP_Val		
#define NVMCTRL_CTRLA_CMD_DFER				NVMCTRL_CTRLA_CMD_RWWEEER			
#define NVMCTRL_CTRLA_CMD_DFWP				NVMCTRL_CTRLA_CMD_RWWEEWP			
#define NVMCTRL_PARAM_DFP_Pos				NVMCTRL_PARAM_RWWEEP_Pos			
#define NVMCTRL_PARAM_DFP_Msk				NVMCTRL_PARAM_RWWEEP_Msk			
#define NVMCTRL_PARAM_DFP(value)			NVMCTRL_PARAM_RWWEEP(value)         
//#define NVMCTRL_PBLDATA0_RESETVALUE is 0x00 in SAM and 0xFFFFFFFF in PIC32C
//#define NVMCTRL_PBLDATA1_RESETVALUE is 0x00 in SAM and 0xFFFFFFFF in PIC32C
//#define NVMCTRL_PBLDATA1_Msk is 0x00 in SAM and 0xFFFFFFFF in PIC32C

/*mclk.h*/
#define MCLK_AHBMASK_APBA_Pos		MCLK_AHBMASK_HPB0_Pos		
#define MCLK_AHBMASK_APBA_Msk		MCLK_AHBMASK_HPB0_Msk		
#define MCLK_AHBMASK_APBA(value)	MCLK_AHBMASK_HPB0(value)	
#define MCLK_AHBMASK_APBB_Pos		MCLK_AHBMASK_HPB1_Pos		
#define MCLK_AHBMASK_APBB_Msk		MCLK_AHBMASK_HPB1_Msk		
#define MCLK_AHBMASK_APBB(value)	MCLK_AHBMASK_HPB1(value)	
#define MCLK_AHBMASK_APBC_Pos		MCLK_AHBMASK_HPB2_Pos		
#define MCLK_AHBMASK_APBC_Msk		MCLK_AHBMASK_HPB2_Msk		
#define MCLK_AHBMASK_APBC(value)	MCLK_AHBMASK_HPB2(value)	

/*gclk.h*/
#define   GCLK_GENCTRL_SRC_FDPLL96M_Val	GCLK_GENCTRL_SRC_DPLL96M_Val	
#define GCLK_GENCTRL_SRC_FDPLL96M		GCLK_GENCTRL_SRC_DPLL96M		

/*dmac.h*/
#define DMAC_PRICTRL_RESETVALUE							DMAC_PRICTRL0_RESETVALUE						
#define DMAC_PRICTRL_LVLPRI0_Pos						DMAC_PRICTRL0_LVLPRI0_Pos             			
#define DMAC_PRICTRL_LVLPRI0_Msk						DMAC_PRICTRL0_LVLPRI0_Msk             			
#define DMAC_PRICTRL_LVLPRI0(value)						DMAC_PRICTRL0_LVLPRI0(value)          			
#define DMAC_PRICTRL_RRLVLEN0_Pos						DMAC_PRICTRL0_RRLVLEN0_Pos            			
#define DMAC_PRICTRL_RRLVLEN0_Msk						DMAC_PRICTRL0_RRLVLEN0_Msk            			
#define DMAC_PRICTRL_RRLVLEN0(value)					DMAC_PRICTRL0_RRLVLEN0(value)         			
#define DMAC_PRICTRL_RRLVLEN0_STATIC_LVL_Val			DMAC_PRICTRL0_RRLVLEN0_STATIC_LVL_Val 			
#define DMAC_PRICTRL_RRLVLEN0_ROUND_ROBIN_LVL_Val		DMAC_PRICTRL0_RRLVLEN0_ROUND_ROBIN_LVL_Val 		
#define DMAC_PRICTRL_RRLVLEN0_STATIC_LVL				DMAC_PRICTRL0_RRLVLEN0_STATIC_LVL     			
#define DMAC_PRICTRL_RRLVLEN0_ROUND_ROBIN_LVL			DMAC_PRICTRL0_RRLVLEN0_ROUND_ROBIN_LVL 			
#define DMAC_PRICTRL_LVLPRI1_Pos						DMAC_PRICTRL0_LVLPRI1_Pos             			
#define DMAC_PRICTRL_LVLPRI1_Msk						DMAC_PRICTRL0_LVLPRI1_Msk             			
#define DMAC_PRICTRL_LVLPRI1(value)						DMAC_PRICTRL0_LVLPRI1(value)          			
#define DMAC_PRICTRL_RRLVLEN1_Pos						DMAC_PRICTRL0_RRLVLEN1_Pos            			
#define DMAC_PRICTRL_RRLVLEN1_Msk						DMAC_PRICTRL0_RRLVLEN1_Msk            			
#define DMAC_PRICTRL_RRLVLEN1(value)					DMAC_PRICTRL0_RRLVLEN1(value)         			
#define DMAC_PRICTRL_LVLPRI2_Pos						DMAC_PRICTRL0_LVLPRI2_Pos             			
#define DMAC_PRICTRL_LVLPRI2_Msk						DMAC_PRICTRL0_LVLPRI2_Msk             			
#define DMAC_PRICTRL_LVLPRI2(value)						DMAC_PRICTRL0_LVLPRI2(value)          			
#define DMAC_PRICTRL_RRLVLEN2_Pos						DMAC_PRICTRL0_RRLVLEN2_Pos            			
#define DMAC_PRICTRL_RRLVLEN2_Msk						DMAC_PRICTRL0_RRLVLEN2_Msk            			
#define DMAC_PRICTRL_RRLVLEN2(value)					DMAC_PRICTRL0_RRLVLEN2(value)         			
#define DMAC_PRICTRL_LVLPRI3_Pos						DMAC_PRICTRL0_LVLPRI3_Pos             			
#define DMAC_PRICTRL_LVLPRI3_Msk						DMAC_PRICTRL0_LVLPRI3_Msk             			
#define DMAC_PRICTRL_LVLPRI3(value)						DMAC_PRICTRL0_LVLPRI3(value)          			
#define DMAC_PRICTRL_RRLVLEN3_Pos 						DMAC_PRICTRL0_RRLVLEN3_Pos            			
#define DMAC_PRICTRL_RRLVLEN3_Msk						DMAC_PRICTRL0_RRLVLEN3_Msk            			
#define DMAC_PRICTRL_RRLVLEN3(value)					DMAC_PRICTRL0_RRLVLEN3(value)         			
#define DMAC_PRICTRL_Msk								DMAC_PRICTRL0_Msk                     			
#define DMAC_PRICTRL_REG_OFST							DMAC_PRICTRL0_REG_OFST							
#define DMAC_PRICTRL									DMAC_PRICTRL0									

/*dac.h*/
#define   DAC_CTRLB_REFSEL_INTREF_Val			DAC_CTRLB_REFSEL_INT1V_Val          
#define   DAC_CTRLB_REFSEL_VDDANA_Val			DAC_CTRLB_REFSEL_AVCC_Val           
#define   DAC_CTRLB_REFSEL_VREFA_Val			DAC_CTRLB_REFSEL_VREFP_Val          
#define DAC_CTRLB_REFSEL_INTREF					DAC_CTRLB_REFSEL_INT1V                
#define DAC_CTRLB_REFSEL_VDDANA					DAC_CTRLB_REFSEL_AVCC                 
#define DAC_CTRLB_REFSEL_VREFA					DAC_CTRLB_REFSEL_VREFP                

/*ccl.h*/
#define   CCL_LUTCTRL_INVEI_NORMAL_Val			CCL_LUTCTRL_INVEI_DISABLE_Val       
#define   CCL_LUTCTRL_INVEI_INVERTED_Val		CCL_LUTCTRL_INVEI_ENABLE_Val        
#define CCL_LUTCTRL_INVEI_NORMAL				CCL_LUTCTRL_INVEI_DISABLE             
#define CCL_LUTCTRL_INVEI_INVERTED				CCL_LUTCTRL_INVEI_ENABLE              

/*adc.h*/
#define   ADC_REFCTRL_REFSEL_VREFA_Val				ADC_REFCTRL_REFSEL_AREFA_Val				
#define ADC_REFCTRL_REFSEL_VREFA					ADC_REFCTRL_REFSEL_AREFA					
#define   ADC_INPUTCTRL_MUXPOS_INTREF_Val			ADC_INPUTCTRL_MUXPOS_BANDGAP_Val			
#define   ADC_INPUTCTRL_MUXPOS_SCALEDVDDCORE_Val	ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC_Val	
#define   ADC_INPUTCTRL_MUXPOS_SCALEDVDDANA_Val		ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC_Val		
#define ADC_INPUTCTRL_MUXPOS_INTREF					ADC_INPUTCTRL_MUXPOS_BANDGAP				
#define ADC_INPUTCTRL_MUXPOS_SCALEDVDDCORE			ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC			
#define ADC_INPUTCTRL_MUXPOS_SCALEDVDDANA			ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC			

/*ac.h*/
#define   AC_COMPCTRL_MUXNEG_AIN0_Val		AC_COMPCTRL_MUXNEG_PIN0_Val		
#define   AC_COMPCTRL_MUXNEG_AIN1_Val		AC_COMPCTRL_MUXNEG_PIN1_Val		
#define   AC_COMPCTRL_MUXNEG_AIN2_Val		AC_COMPCTRL_MUXNEG_PIN2_Val		
#define   AC_COMPCTRL_MUXNEG_AIN3_Val		AC_COMPCTRL_MUXNEG_PIN3_Val		
#define   AC_COMPCTRL_MUXNEG_INTREF_Val		AC_COMPCTRL_MUXNEG_BANDGAP_Val 	
#define AC_COMPCTRL_MUXNEG_AIN0				AC_COMPCTRL_MUXNEG_PIN0				
#define AC_COMPCTRL_MUXNEG_AIN1				AC_COMPCTRL_MUXNEG_PIN1				
#define AC_COMPCTRL_MUXNEG_AIN2				AC_COMPCTRL_MUXNEG_PIN2				
#define AC_COMPCTRL_MUXNEG_AIN3				AC_COMPCTRL_MUXNEG_PIN3				
#define AC_COMPCTRL_MUXNEG_INTREF			AC_COMPCTRL_MUXNEG_BANDGAP			
#define   AC_COMPCTRL_MUXPOS_AIN0_Val		AC_COMPCTRL_MUXPOS_PIN0_Val		
#define   AC_COMPCTRL_MUXPOS_AIN1_Val		AC_COMPCTRL_MUXPOS_PIN1_Val		
#define   AC_COMPCTRL_MUXPOS_AIN2_Val		AC_COMPCTRL_MUXPOS_PIN2_Val		
#define   AC_COMPCTRL_MUXPOS_AIN3_Val		AC_COMPCTRL_MUXPOS_PIN3_Val		
#define AC_COMPCTRL_MUXPOS_AIN0				AC_COMPCTRL_MUXPOS_PIN0				
#define AC_COMPCTRL_MUXPOS_AIN1				AC_COMPCTRL_MUXPOS_PIN1				
#define AC_COMPCTRL_MUXPOS_AIN2				AC_COMPCTRL_MUXPOS_PIN2				
#define AC_COMPCTRL_MUXPOS_AIN3				AC_COMPCTRL_MUXPOS_PIN3				

#endif /* _SAM_PERIPHERAL_LEGACY_H_ */




