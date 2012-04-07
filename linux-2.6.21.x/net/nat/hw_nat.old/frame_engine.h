/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    frame_engine.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-11      Initial version
*/

#ifndef _FE_WANTED
#define _FE_WANTED

#include <linux/version.h>

/*
 * DEFINITIONS AND MACROS
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
#include <asm/rt2880/rt_mmap.h>
#define FE_BASE		    RALINK_FRAME_ENGINE_BASE
#define SYS_CTRL_BASE       RALINK_SYSCTL_BASE
#else
#if defined(CONFIG_RALINK_RT2880_SHUTTLE)
#define FE_BASE		   0xA0310000
#define SYS_CTRL_BASE      0xA0300000
#elif defined (CONFIG_RALINK_RT2880_MP)
#define FE_BASE		   0xA0400000
#define SYS_CTRL_BASE      0xA0300000
#else
#error Please Choose Chip Version 
#endif
#endif


#define CHIPID		    SYS_CTRL_BASE + 0x00
#define REVID		    SYS_CTRL_BASE + 0x0C

#define FE_GLO_BASE         FE_BASE
#define PPE_BASE	    FE_BASE + 0x200
#define AC_BASE		    FE_BASE + 0x400
#define METER_BASE	    FE_BASE + 0x600

#define FOE_TS_T	    FE_GLO_BASE+0x1C
#define GDMA1_BASE          FE_GLO_BASE+0x20
#define GDMA1_SCH_CFG       GDMA1_BASE+0x04
#define GDMA2_BASE          FE_GLO_BASE+0x60
#define GDMA2_SCH_CFG       GDMA2_BASE+0x04


#define PPE_GLO_CFG	    PPE_BASE + 0x00
#define PPE_STD_GA_H	    PPE_BASE + 0x04
#define PPE_STD_GA_L	    PPE_BASE + 0x08
#define PPE_ETD_GA_H	    PPE_BASE + 0x0C
#define PPE_EXT_GA_L	    PPE_BASE + 0x10
#define PPE_FLOW_SET	    PPE_BASE + 0x14
#define PPE_PRE_ACL	    PPE_BASE + 0x18
#define PPE_PRE_MTR	    PPE_BASE + 0x1C
#define PPE_PRE_AC	    PPE_BASE + 0x20
#define PPE_POST_MTR	    PPE_BASE + 0x24
#define PPE_POST_AC	    PPE_BASE + 0x28
#define PPE_POL_CFG	    PPE_BASE + 0x2C
#define PPE_FOE_CFG	    PPE_BASE + 0x30
#define PPE_FOE_BASE	    PPE_BASE + 0x34
#define PPE_FOE_USE	    PPE_BASE + 0x38
#define PPE_FOE_BNDR	    PPE_BASE + 0x3C
#define PPE_FOE_LMT1	    PPE_BASE + 0x40
#define PPE_FOE_LMT2	    PPE_BASE + 0x44
#define PPE_FOE_KA	    PPE_BASE + 0x48
#define PPE_FOE_UNB_AGE	    PPE_BASE + 0x4C
#define PPE_FOE_BND_AGE1    PPE_BASE + 0x50
#define PPE_FOE_BND_AGE2    PPE_BASE + 0x54
#define CPU_PORT_CFG	    PPE_BASE + 0x58
#define GE1_PORT_CFG	    PPE_BASE + 0x5C
#define DSCP0_7_MAP_UP	    PPE_BASE + 0x60
#define DSCP8_15_MAP_UP	    PPE_BASE + 0x64
#define DSCP16_23_MAP_UP    PPE_BASE + 0x68
#define DSCP24_31_MAP_UP    PPE_BASE + 0x6C
#define DSCP32_39_MAP_UP    PPE_BASE + 0x70
#define DSCP40_47_MAP_UP    PPE_BASE + 0x74
#define DSCP48_55_MAP_UP    PPE_BASE + 0x78
#define DSCP56_63_MAP_UP    PPE_BASE + 0x7C
#define AUTO_UP_CFG1	    PPE_BASE + 0x80
#define AUTO_UP_CFG2	    PPE_BASE + 0x84
#define UP_RES		    PPE_BASE + 0x88
#define UP_MAP_VPRI	    PPE_BASE + 0x8C
#define UP0_3_MAP_IDSCP	    PPE_BASE + 0x90
#define UP4_7_MAP_IDSCP	    PPE_BASE + 0x94
#define UP0_3_MAP_ODSCP	    PPE_BASE + 0x98
#define UP4_7_MAP_ODSCP	    PPE_BASE + 0x9C
#define UP_MAP_AC	    PPE_BASE + 0xA0

#define GDMA1_FWD_CFG	    FE_BASE + 0x20 
#define GDMA2_FWD_CFG	    FE_BASE + 0x60
#define FE_COS_MAP	    FE_BASE + 0xC8
#define IS_EXT_SW_EN(x)     (x & 1<<30)

#define GDM1_UFRC_P_CPU     (0 << 12) 
#define GDM1_UFRC_P_GDMA1   (1 << 12) 
#define GDM1_UFRC_P_PPE     (6 << 12)

#define GDM1_BFRC_P_CPU     (0 << 8)
#define GDM1_BFRC_P_GDMA1   (1 << 8)
#define GDM1_BFRC_P_PPE     (6 << 8)

#define GDM1_MFRC_P_CPU     (0 << 4)
#define GDM1_MFRC_P_GDMA1   (1 << 4)
#define GDM1_MFRC_P_PPE     (6 << 4)

#define GDM1_OFRC_P_CPU     (0 << 0)
#define GDM1_OFRC_P_GDMA1   (1 << 0)
#define GDM1_OFRC_P_PPE     (6 << 0)


enum FoeSma {
    DROP = 0,		    
    DROP2 = 1,		    
    ONLY_FWD_CPU = 2,	    
    FWD_CPU_BUILD_ENTRY = 3 
};

enum FoeCpuReason {
    TTL_0=0x80,				
    FOE_EBL_NOT_IPV4_HLEN5=0x90,	
    FOE_EBL_NOT_TCP_UDP_L4_READY=0x91,  
    TCP_SYN_FIN_RST=0x92,		
    UN_HIT=0x93,			
    HIT_UNBIND=0x94,			
    HIT_UNBIND_RATE_REACH=0x95,		
    HIT_FIN=0x96,			
    HIT_BIND_TTL_1=0x97,		
    HIT_BIND_KEEPALIVE=0x98,		
    HIT_BIND_FORCE_TO_CPU=0x99,		
    ACL_FOE_TBL_ERR=0x9A,		
    ACL_TBL_TTL_1=0x9B,			
    ACL_ALERT_CPU=0x9C,			
    NO_FORCE_DEST_PORT=0xA0,		
    ACL_FORCE_PRIORITY0=0xA8,		
    ACL_FORCE_PRIORITY1=0xA9,		
    ACL_FORCE_PRIORITY2=0xAA,		
    ACL_FORCE_PRIORITY3=0xAB,		
    ACL_FORCE_PRIORITY4=0xAC,		
    ACL_FORCE_PRIORITY5=0xAD,		
    ACL_FORCE_PRIORITY6=0xAE,		
    ACL_FORCE_PRIORITY7=0xAF,		
    EXCEED_MTU=0xA1			
};	


/* PPE_GLO_CFG, Offset=0x200 */
#define DFL_TTL0_DRP		(1) 
#define DFL_VPRI_EN		(1) 
#define DFL_DPRI_EN		(1) 
#define DFL_REG_VPRI		(1) 
#define DFL_REG_DSCP		(0) 
#define DFL_ACL_PRI_EN		(1) 

#define DFL_RED_MODE		(1) 


/* DSCPx_y_MAP_UP=0x60~0x7C */
#define DFL_DSCP0_7_UP		(0) /* User priority of DSCP0~7 */
#define DFL_DSCP24_31_UP	(0x33333333) /* User priority of DSCP24~31 */
#define DFL_DSCP8_15_UP		(0x11111111) /* User priority of DSCP8~15 */
#define DFL_DSCP16_23_UP	(0x22222222) /* User priority of DSCP16~23 */
#define DFL_DSCP32_39_UP	(0x44444444) /* User priority of DSCP32~39 */
#define DFL_DSCP40_47_UP	(0x55555555) /* User priority of DSCP40~47 */
#define DFL_DSCP48_55_UP	(0x66666666) /* User priority of DSCP48~55 */
#define DFL_DSCP56_63_UP	(0x77777777) /* User priority of DSCP56~63 */

/* AUTO_UP_CFG=0x80~0x84 */

#define DFL_ATUP_BND1		(256)
#define DFL_ATUP_BND2		(512)
#define DFL_ATUP_BND3		(1024)
#define DFL_ATUP_R1_UP		(0) 
#define DFL_ATUP_R2_UP		(2) 
#define DFL_ATUP_R3_UP		(4) 
#define DFL_ATUP_R4_UP		(6) 

/* UP_RES=0x88 */
#define PUP_WT_OFFSET		(0) 
#define VUP_WT_OFFSET		(4) 
#define DUP_WT_OFFSET		(8) 
#define AUP_WT_OFFSET		(12) 
#define FUP_WT_OFFSET		(16) 
#define ATP_WT_OFFSET		(20) 

/* DSCP is highest pri */
#define DFL_UP_RES		((0<<ATP_WT_OFFSET) | (0<<VUP_WT_OFFSET) |\
				 (7<<DUP_WT_OFFSET) | (0<<AUP_WT_OFFSET) | \
				 (0<<FUP_WT_OFFSET) | (0<<ATP_WT_OFFSET) )
				
				

/* UP_MAP_VPRI=0x8C */
#define DFL_UP0_VPRI		(0x0) /* user pri 0 map to vlan pri tag */
#define DFL_UP3_VPRI		(0x3) /* user pri 3 map to vlan pri tag */
#define DFL_UP1_VPRI		(0x1) /* user pri 1 map to vlan pri tag */
#define DFL_UP2_VPRI		(0x2) /* user pri 2 map to vlan pri tag */
#define DFL_UP4_VPRI		(0x4) /* user pri 4 map to vlan pri tag */
#define DFL_UP5_VPRI		(0x5) /* user pri 5 map to vlan pri tag */
#define DFL_UP6_VPRI		(0x6) /* user pri 6 map to vlan pri tag */
#define DFL_UP7_VPRI		(0x7) /* user pri 7 map to vlan pri tag */

/* UPx_y_MAP_IDSCP=0x90~0x94 */
#define DFL_UP0_IDSCP		(0x00) /* user pri 0 map to in-profile DSCP */
#define DFL_UP3_IDSCP		(0x18) /* user pri 3 map to in-profile DSCP */
#define DFL_UP1_IDSCP		(0x08) /* user pri 1 map to in-profile DSCP */
#define DFL_UP2_IDSCP		(0x10) /* user pri 2 map to in-profile DSCP */
#define DFL_UP4_IDSCP		(0x20) /* user pri 4 map to in-profile DSCP */
#define DFL_UP5_IDSCP		(0x28) /* user pri 5 map to in-profile DSCP */
#define DFL_UP6_IDSCP		(0x30) /* user pri 6 map to in-profile DSCP */
#define DFL_UP7_IDSCP		(0x38) /* user pri 7 map to in-profile DSCP */

/* UPx_y_MAP_ODSCP=0x98~0x9C */
#define DFL_UP0_ODSCP		(0x00) /* user pri 0 map to out-profile DSCP */
#define DFL_UP3_ODSCP		(0x10) /* user pri 3 map to out-profile DSCP */
#define DFL_UP1_ODSCP		(0x00) /* user pri 1 map to out-profile DSCP */
#define DFL_UP2_ODSCP		(0x08) /* user pri 2 map to out-profile DSCP */
#define DFL_UP4_ODSCP		(0x18) /* user pri 4 map to out-profile DSCP */
#define DFL_UP5_ODSCP		(0x20) /* user pri 5 map to out-profile DSCP */
#define DFL_UP6_ODSCP		(0x28) /* user pri 6 map to out-profile DSCP */
#define DFL_UP7_ODSCP		(0x30) /* user pri 7 map to out-profile DSCP */

/* UP_MAP_AC=0xA0 */
#define DFL_UP0_AC		(2) /* user pri 0 map to access category */
#define DFL_UP1_AC		(2) /* user pri 1 map to access category */
#define DFL_UP2_AC		(2) /* user pri 2 map to access category */
#define DFL_UP3_AC		(2) /* user pri 3 map to access category */
#define DFL_UP4_AC		(2) /* user pri 4 map to access category */
#define DFL_UP5_AC		(2) /* user pri 5 map to access category */
#define DFL_UP6_AC		(2) /* user pri 6 map to access category */
#define DFL_UP7_AC		(2) /* user pri 7 map to access category */

/* 
 * PPE Flow Set 
 */
#define BIT_FBC_POSA		(1<<0)  
#define BIT_FBC_POSM		(1<<1)  
#define BIT_FBC_FOE		(1<<2)  
#define BIT_FBC_PREA		(1<<3)  
#define BIT_FBC_PREM		(1<<4)  
#define BIT_FBC_ACL		(1<<5)  

#define BIT_FMC_POSA		(1<<8) 
#define BIT_FMC_POSM		(1<<9) 
#define BIT_FMC_FOE		(1<<10) 
#define BIT_FMC_PREA		(1<<11) 
#define BIT_FMC_PREM		(1<<12) 
#define BIT_FMC_ACL		(1<<13) 

#define BIT_FUC_POSA		(1<<16) 
#define BIT_FUC_POSM	    	(1<<17) 
#define BIT_FUC_FOE		(1<<18) 
#define BIT_FUC_PREA		(1<<19) 
#define BIT_FUC_PREM		(1<<20) 
#define BIT_FUC_ACL		(1<<21) 
#define BIT_IPV4_NAPT_EN        (1<<27) 
#define BIT_IPV4_NAT_EN         (1<<26) 
#define BIT_IPV6_FOE_EN		(1<<24) 
#define BIT_IPV6_PE_EN		(1<<25) 

/* 
 * Pre/Post - ACL & MTR Table 
 */ 

#define DFL_PRE_ACL_STR		0	

#define DFL_PRE_ACL_END		DFL_PRE_ACL_STR

#define DFL_PRE_MTR_STR		DFL_PRE_ACL_STR + CONFIG_RA_HW_NAT_PRE_ACL_SIZE   
#define DFL_PRE_MTR_END		DFL_PRE_MTR_STR

#define DFL_PRE_AC_STR		DFL_PRE_MTR_STR + CONFIG_RA_HW_NAT_PRE_MTR_SIZE

#define DFL_PRE_AC_END		DFL_PRE_AC_STR

#define DFL_POST_MTR_STR	DFL_PRE_AC_STR + CONFIG_RA_HW_NAT_PRE_AC_SIZE

#define DFL_POST_MTR_END	DFL_POST_MTR_STR

#define DFL_POST_AC_STR		DFL_POST_MTR_STR + CONFIG_RA_HW_NAT_POST_MTR_SIZE 

#define DFL_POST_AC_END		DFL_POST_AC_STR

#define DFL_POL_AC_PRD		0xFFFF

#define DFL_FOE_BNDR		CONFIG_RA_HW_NAT_BINDING_THRESHOLD 

#define DFL_FOE_QURT_LMT	CONFIG_RA_HW_NAT_QURT_LMT 

#define DFL_FOE_HALF_LMT	CONFIG_RA_HW_NAT_HALF_LMT 

#define DFL_FOE_FULL_LMT	CONFIG_RA_HW_NAT_FULL_LMT 

#define DFL_FOE_KA_T		1   

#define DFL_FOE_TCP_KA		CONFIG_RA_HW_NAT_TCP_KA

#define DFL_FOE_UDP_KA		CONFIG_RA_HW_NAT_UDP_KA   

/* 
 * PPE_FOE_CFG 
 */
#ifdef CONFIG_RA_HW_NAT_HASH0
#define DFL_FOE_HASH_MODE	0
#elif CONFIG_RA_HW_NAT_HASH1
#define DFL_FOE_HASH_MODE	1
#endif

#define DFL_FOE_UNB_AGE		1   
#define DFL_FOE_TCP_AGE		1  
#define DFL_FOE_UDP_AGE		1  
#define DFL_FOE_FIN_AGE		1  
#define DFL_FOE_KA_ORG		0  
#define DFL_FOE_KA_EN		1  
#define DFL_FOE_ACL_AGE		0  

#define DFL_FOE_UNB_MNP		CONFIG_RA_HW_NAT_UNB_MNP 
#define DFL_FOE_ACL_DLTA        CONFIG_RA_HW_NAT_ACL_DLTA
#define DFL_FOE_UNB_DLTA	CONFIG_RA_HW_NAT_UNB_DLTA

#define DFL_FOE_UDP_DLTA	CONFIG_RA_HW_NAT_UDP_DLTA

#define DFL_FOE_FIN_DLTA 	CONFIG_RA_HW_NAT_FIN_DLTA 
#define DFL_FOE_TCP_DLTA	CONFIG_RA_HW_NAT_TCP_DLTA 


#define DFL_FOE_TTL_REGEN	1   


#define IS_FORCE_ACL_TO_UP(x)   (FOE_AI(x)&0xF8)==ACL_FORCE_PRIORITY0
#define GET_ACL_TO_UP(x)        FOE_AI(x)&0x07


#endif
