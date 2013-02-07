# If source directory exists, then build from source, otherwise use pre-builts
REBUILD_MODULE=$(shell if [ -d "$(src)/common" ]; then echo 1; else echo 0; fi)
ifeq ($(REBUILD_MODULE),0)
# Assume prebuilt object module(s) in prebuilt directory
obj-$(CONFIG_RT3090_AP) += RT3090_ap.o
RT3090_ap-objs += prebuilt/RT3090_ap.o
else

EXTRA_CFLAGS = -Idrivers/net/wireless/RT3090_ap/include

obj-$(CONFIG_RT3090_AP) += RT3090_ap.o

RT3090_ap-objs += common/crypt_md5.o
RT3090_ap-objs += common/crypt_sha2.o
RT3090_ap-objs += common/crypt_hmac.o
RT3090_ap-objs += common/crypt_aes.o
RT3090_ap-objs += common/crypt_arc4.o
RT3090_ap-objs += common/mlme.o
RT3090_ap-objs += common/cmm_wep.o
RT3090_ap-objs += common/action.o
RT3090_ap-objs += common/cmm_data.o
RT3090_ap-objs += common/rtmp_init.o
RT3090_ap-objs += common/cmm_tkip.o
RT3090_ap-objs += common/cmm_aes.o
RT3090_ap-objs += common/cmm_sync.o
RT3090_ap-objs += common/eeprom.o
RT3090_ap-objs += common/cmm_sanity.o
RT3090_ap-objs += common/cmm_info.o
RT3090_ap-objs += common/cmm_cfg.o
RT3090_ap-objs += common/cmm_wpa.o
RT3090_ap-objs += common/dfs.o
RT3090_ap-objs += common/spectrum.o
RT3090_ap-objs += common/rtmp_timer.o
RT3090_ap-objs += common/rt_channel.o
RT3090_ap-objs += common/cmm_profile.o
RT3090_ap-objs += common/cmm_asic.o
RT3090_ap-objs += common/cmm_cmd.o

#RT3090_ap-objs += common/ee_flash.o

RT3090_ap-objs += ap/ap.o
RT3090_ap-objs += ap/ap_assoc.o
RT3090_ap-objs += ap/ap_auth.o
RT3090_ap-objs += ap/ap_connect.o
RT3090_ap-objs += ap/ap_mlme.o
RT3090_ap-objs += ap/ap_sanity.o
RT3090_ap-objs += ap/ap_sync.o
RT3090_ap-objs += ap/ap_wpa.o
RT3090_ap-objs += ap/ap_data.o
RT3090_ap-objs += ap/ap_uapsd.o
RT3090_ap-objs += ap/ap_autoChSel.o
RT3090_ap-objs += ap/ap_qload.o
RT3090_ap-objs += ap/ap_cfg.o

RT3090_ap-objs += common/rtmp_init_inf.o
RT3090_ap-objs += os/linux/rt_profile.o
RT3090_ap-objs += os/linux/ap_ioctl.o
RT3090_ap-objs += os/linux/rt_linux.o
RT3090_ap-objs += os/linux/rt_main_dev.o

RT3090_ap-objs += common/ba_action.o

RT3090_ap-objs += common/cmm_mac_pci.o
RT3090_ap-objs += common/cmm_data_pci.o
RT3090_ap-objs += common/rtmp_mcu.o
RT3090_ap-objs += common/ee_prom.o
RT3090_ap-objs += common/ee_efuse.o
RT3090_ap-objs += common/rt_rf.o
RT3090_ap-objs += chips/rt30xx.o
RT3090_ap-objs += chips/rt3090.o
RT3090_ap-objs += chips/rt33xx.o
RT3090_ap-objs += chips/rt3390.o
#RT3090_ap-objs += chips/rt5390.o
RT3090_ap-objs += os/linux/rt_pci_rbus.o
RT3090_ap-objs += os/linux/rt_rbus_pci_util.o
RT3090_ap-objs += os/linux/pci_main_dev.o

RT3090_ap-objs += common/rt_ate.o

ifeq ($(CONFIG_RT3090_AP_WMM_ACM),y)
RT3090_ap-objs += common/acm_edca.o
RT3090_ap-objs += common/acm_comm.o
RT3090_ap-objs += common/acm_iocl.o
endif

ifeq ($(CONFIG_RT3090_AP_WSC),y)
RT3090_ap-objs += common/wsc.o 
RT3090_ap-objs += common/wsc_tlv.o
RT3090_ap-objs += common/wsc_ufd.o
RT3090_ap-objs += common/crypt_biginteger.o
RT3090_ap-objs += common/crypt_dh.o
endif

#ifeq ($(CONFIG_RT3090_AP_NINTENDO),y)
#RT3090_ap-objs += ap/ap_nintendo.o
#endif

ifeq ($(CONFIG_RT3090_AP_WDS),y)
RT3090_ap-objs += ap/ap_wds.o
RT3090_ap-objs += ap/ap_wds_inf.o
RT3090_ap-objs += common/client_wds.o
endif

ifeq ($(CONFIG_RT3090_AP_MBSS),y)
RT3090_ap-objs += ap/ap_mbss.o
endif

ifeq ($(CONFIG_RT3090_AP_VIDEO_TURBINE),y)
RT3090_ap-objs += ../rt2860v2/common/cmm_video.o
endif

#ifeq ($(CONFIG_RT3090_AP_APCLI),y)
#RT3090_ap-objs += ap/ap_apcli_inf.o
#RT3090_ap-objs += ap/ap_apcli.o
#RT3090_ap-objs += ap/apcli_assoc.o
#RT3090_ap-objs += ap/apcli_auth.o
#RT3090_ap-objs += ap/apcli_ctrl.o
#RT3090_ap-objs += ap/apcli_sync.o
#RT3090_ap-objs += common/cmm_mat.o
#RT3090_ap-objs += common/cmm_mat_iparp.o
#RT3090_ap-objs += common/cmm_mat_pppoe.o
#RT3090_ap-objs += common/cmm_mat_ipv6.o
#endif

#ifeq ($(CONFIG_RT3090_AP_WAPI),y)
#RT3090_ap-objs += common/wapi.o
#RT3090_ap-objs += common/wapi_sms4.o
#RT3090_ap-objs += common/wapi_crypt.o
#
##--------SDK release----------------
##before compiling code
## RT3090_ap-objs += common/wapi.o
## RT3090_ap-objs += common/wapi_sms4.o
## RT3090_ap-objs += common/wapi_crypt.o
##
##after compiling code
## RT3090_ap-objs += common/wapi.obj
## RT3090_ap-objs += common/wapi_sms4.obj
## RT3090_ap-objs += common/wapi_crypt.obj
#endif

#ifeq ($(CONFIG_RT3090_80211R_FT),y)
#RT3090_ap-objs += common/ft.o
#RT3090_ap-objs += common/ft_tlv.o
#RT3090_ap-objs += common/ft_ioctl.o
#RT3090_ap-objs += common/ft_rc.o
#RT3090_ap-objs += ap/ap_ftkd.o
#endif

ifeq ($(CONFIG_RT3090_AP_IGMP_SNOOP),y)
RT3090_ap-objs += common/igmp_snoop.o
endif

ifeq ($(CONFIG_RT3090_AP_NETIF_BLOCK),y)
RT3090_ap-objs += common/netif_block.o
endif

ifeq ($(CONFIG_RT3090_AP_DLS),y)
RT3090_ap-objs += ap/ap_dls.o
endif

#ifeq ($(CONFIG_RT3090_AP_IDS),y)
#RT3090_ap-objs += ap/ap_ids.o
#endif

#ifeq ($(CONFIG_RT2860V2_AP_VIDEO_TURBINE),y)
#RT3090_ap-objs += common/cmm_video.o
#endif

#ifeq ($(CONFIG_RT3090_AP_MESH),y)
#RT3090_ap-objs += common/mesh_bmpkt.o
#RT3090_ap-objs += common/mesh_ctrl.o
#RT3090_ap-objs += common/mesh_link_mng.o
#RT3090_ap-objs += common/mesh_sanity.o
#RT3090_ap-objs += common/mesh_tlv.o
#RT3090_ap-objs += common/mesh.o
#RT3090_ap-objs += common/mesh_inf.o
#RT3090_ap-objs += common/mesh_forwarding.o
#RT3090_ap-objs += common/mesh_path_mng.o
#endif

#ifeq ($(CONFIG_RT3090_RT3XXX_AP_ANTENNA_DIVERSITY),y)
#RT3090_ap-objs += os/linux/ap_diversity.o
#endif

#ifeq ($(CONFIG_RT3090_80211K_RR),y)
#RT3090_ap-objs += common/rrm_tlv.o
#RT3090_ap-objs += common/rrm_sanity.o
#RT3090_ap-objs += common/rrm.o
#endif

endif # MODULE/common exists

###################
#  CFLAGS
#  ##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
				-Wall -Wstrict-prototypes -Wno-trigraphs
EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DAP_SCAN_SUPPORT -DUAPSD_AP_SUPPORT -DIAPP_SUPPORT -DDBG -DDOT11_N_SUPPORT -DDOT1X_SUPPORT -DSTATS_COUNT_SUPPORT #-DSYSTEM_LOG_SUPPORT
EXTRA_CFLAGS += -DRTMP_MAC_PCI -DRT30xx -DRT33xx -DRT3090 -DRT3390 -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT -DRTMP_EFUSE_SUPPORT -DRALINK_ATE -DRALINK_QA -DRTMP_INTERNAL_TX_ALC -DCONFIG_RT2880_ATE_CMD_NEW 

# ASUS EXT
EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC 
#EXTRA_CFLAGS += -DGREENAP_SUPPORT
ifneq ($(CONFIG_RA_HW_NAT_WIFI),y)
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
endif
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
EXTRA_CFLAGS += -DRSSI_FEEDBACK

ifeq ($(CONFIG_RT3090_AP_FLASH),y)
RT3090_ap-objs += common/ee_flash.o
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_DLS),y)
EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
endif

#ifeq ($(CONFIG_RT3090_AP_IDS),y)
#EXTRA_CFLAGS += -DIDS_SUPPORT
#endif

#ifeq ($(CONFIG_RT3090_RT3XXX_AP_ANTENNA_DIVERSITY),y)
#EXTRA_CFLAGS   += -DRT3XXX_ANTENNA_DIVERSITY_SUPPORT
#endif

ifeq ($(CONFIG_RT3090_80211N_DRAFT3),y)
EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_RT3090_AP_WMM_ACM),y)
EXTRA_CFLAGS += -DWMM_ACM_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WSC),y)
EXTRA_CFLAGS += -DWSC_AP_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WSC_V2),y)
EXTRA_CFLAGS += -DWSC_V2_SUPPORT
endif

#ifeq ($(CONFIG_RT3090_AP_NINTENDO),y)
#EXTRA_CFLAGS += -DNINTENDO_AP
#endif

ifeq ($(CONFIG_RT3090_AP_LLTD),y)
EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_WDS),y)
EXTRA_CFLAGS += -DWDS_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_MBSS),y)
EXTRA_CFLAGS += -DMBSS_SUPPORT
endif

#ifeq ($(CONFIG_RT3090_AP_APCLI),y)
#EXTRA_CFLAGS += -DAPCLI_SUPPORT
#EXTRA_CFLAGS += -DMAT_SUPPORT
##EXTRA_CFLAGS += -DMLME_EX
#endif

ifeq ($(CONFIG_RT3090_AP_IGMP_SNOOP),y)
EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
endif

ifeq ($(CONFIG_RT3090_AP_NETIF_BLOCK),y)
EXTRA_CFLAGS += -DBLOCK_NET_IF
endif

#ifeq ($(CONFIG_RT3090_AP_WAPI),y)
#EXTRA_CFLAGS   += -DWAPI_SUPPORT -DSOFT_ENCRYPT
#endif

#ifeq ($(CONFIG_RT3090_AP_VIDEO_TURBINE),y)
#EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT
#endif

#ifeq ($(CONFIG_RT3090_AP_MESH),y)
#EXTRA_CFLAGS   += -DMESH_SUPPORT -DINTEL_CMPC
##EXTRA_CFLAGS += -DMLME_EX
#endif

clean:
	@rm -f `find ./ -name "*.o.cmd"`
	@rm -f `find ./ -name "*.ko"`
	@rm -f `find ./ -name "*.o"`
