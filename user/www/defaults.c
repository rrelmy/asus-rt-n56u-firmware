/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * Router default NVRAM values
 *
 * Copyright 2003, ASUSTeK Inc.
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of ASUSTeK Inc..                            
 *
 * $Id: defaults.c,v 1.1.1.1 2007/02/15 12:15:07 jiahao Exp $
 */

#include <epivers.h>
#include <string.h>
#include <bcmnvram.h>

#define XSTR(s) STR(s)
#define STR(s) #s

struct nvram_tuple router_defaults[] = {
	{"restore_defaults", "0", 0},
{"sid_1","General", 0},

{"1_x_Username","admin",0},

{"1_x_Password","admin",0},

{"1_x_SystemUpTime","",0},

{"1_x_ProductID","",0},

{"1_x_FirmwareVersion","",0},

{"1_x_HardwareVersion","",0},

{"1_x_BootloaderVersion","",0},

{"sid_2","Layer3Forwarding", 0},

{"2_DefaultConnectionService","",0},

{"2_x_ConnectionType","dhcp",0},

{"2_x_ConnectionMode","2",0},

{"2_x_ConnectionSpeed","auto",0},

{"sid_3","WANCommonInterface", 0},

{"sid_4","IPConnection", 0},

{"4_ConnectionType","IP_Routed",0},

{"4_ConnectionStatus","",0},

{"4_Uptime","",0},

{"4_NATEnabled","1",0},

{"4_RSIPAvailable","1",0},

{"4_ExternalIPAddress","",0},

{"4_x_ExternalSubnetMask","",0},

{"4_x_ExternalGateway","",0},

{"4_x_DNSServerEnable","1",0},

{"4_x_DNSServer1","",0},

{"4_x_DNSServer2","",0},

{"4_IPTablesInfo","",0},

{"4_PortMappingEnabled","1",0},

{"4_NATEnabled","1",0},

{"4_ExposedIP","",0},

{"4_BattleNet","1",0},

{"4_ExposedIPCount","0",0},

{"4_VServerEnable","1",0},

{"4_VServerCount","0",0},

{"4_autofwEnable","0",0},

{"4_autofwCount","0",0},

{"4_MappedIP_0","",0},

{"4_MappedInPort_0","",0},

{"4_MappedDescript_0","",0},

{"4_MappedAction_0","Unset",0},

{"4_MappedIP_1","",0},

{"4_MappedInPort_1","",0},

{"4_MappedDescript_1","",0},

{"4_MappedAction_1","Unset",0},

{"4_MappedIP_2","",0},

{"4_MappedInPort_2","",0},

{"4_MappedDescript_2","",0},

{"4_MappedAction_2","Unset",0},

{"4_MappedIP_3","",0},

{"4_MappedInPort_3","",0},

{"4_MappedDescript_3","",0},

{"4_MappedAction_3","Unset",0},

{"4_MappedIP_4","",0},

{"4_MappedInPort_4","",0},

{"4_MappedDescript_4","",0},

{"4_MappedAction_4","Unset",0},

{"4_MappedIP_5","",0},

{"4_MappedInPort_5","",0},

{"4_MappedDescript_5","",0},

{"4_MappedAction_5","Unset",0},

{"4_PortMappingProtocol","",0},                          

{"4_RemoteHost","",0},                          

{"4_ExternalPort","",0},                          

{"4_InternalClient","",0},                          

{"4_InternalPort","",0},                          

{"4_PortMappingDescription","",0},                          

{"4_PortMappingLeaseDuration","",0},                          

{"4_ExposedLanIP","",0},                          

{"4_ExposedWanIP","",0},                          

{"4_ExposedDescript","",0},                          

{"4_VServerPort","",0},                          

{"4_VServerIP","",0},                          

{"4_VServerLPort","",0},                          

{"4_VServerProto","",0},                          

{"4_VServerDescript","",0},                          

{"4_autofwOutPort","",0},                          

{"4_autofwOutProto","",0},                          

{"4_autofwInPort","",0},                          

{"4_autofwInProto","",0},                          

{"4_autofwDesc","",0},                          

{"sid_5","PPPConnection", 0},

{"5_ConnectionType","IP_Routed",0},

{"5_UserName","",0},

{"5_Password","",0},

{"5_ConnectionStatus","",0},

{"5_AutoDisconnectTime","0",0},

{"5_IdleDisconnectTime","1800",0},

{"5_x_IdleTxOnly","0",0},

{"5_x_PPPoEMTU","1492",0},

{"5_x_PPPoEMRU","1492",0},

{"5_WarnDisconnectDelay","0",0},

{"5_x_ServiceName","",0},

{"5_x_AccessConcentrator","",0},

{"5_x_PPPoERelay","0",0},

{"5_x_HostNameForISP","",0},

{"5_x_MacAddressForISP","",0},

{"5_x_HeartBeat","",0},

{"5_ExternalIPAddress","",0},

{"5_x_ExternalSubnetMask","",0},

{"5_x_ExternalGateway","",0},

{"5_x_WANType","",0},

{"5_x_WANIPAddress","",0},

{"5_x_WANSubnetMask","",0},

{"5_x_WANGateway","",0},

{"5_x_WANDNSServer","",0},

{"5_x_WANLink","",0},

{"5_x_WANAction","",0},

{"5_x_DDNSStatus","",0},

{"5_x_MultiPPPoEDef","0",0},

{"5_x_MultiPPPoEEnable1","0",0},

{"5_x_UserName1","",0},

{"5_x_Password1","",0},

{"5_x_IdleTime1","1800",0},

{"5_x_IdleTxOnly1","0",0},

{"5_x_PPPoEMTU1","1492",0},

{"5_x_PPPoEMRU1","1492",0},

{"5_x_ServiceName1","",0},

{"5_x_AccessConcentrator1","",0},

{"5_x_MultiPPPoEEnable2","0",0},

{"5_x_UserName2","",0},

{"5_x_Password2","",0},

{"5_x_IdleTime2","1800",0},

{"5_x_IdleTxOnly2","0",0},

{"5_x_PPPoEMTU2","1492",0},

{"5_x_PPPoEMRU2","1492",0},

{"5_x_ServiceName2","",0},

{"5_x_AccessConcentrator2","",0},

{"5_PPPoERouteCount","0",0},

{"5_x_PPPoESess","",0},                          

{"5_x_PPPoEDir","",0},                          

{"5_x_PPPoEIP","",0},                          

{"5_x_PPPoEMask","0",0},                          

{"sid_6","EthernetLink", 0},

{"sid_7","FirewallConfig", 0},

{"7_FirewallEnable","1",0},

{"7_WanLanLog","none",0},

{"7_x_NatLogEnable","0",0},

{"7_x_WanWebEnable","0",0},

{"7_x_WanWebPort","8080",0},

{"7_x_WanLPREnable","0",0},

{"7_x_WanPingEnable","0",0},

{"7_WanLanFirewallEnable","0",0},

{"7_WanLanActiveDate","1111111",0},

{"7_WanLanActiveTime","00002359",0},

{"7_WanLanDefaultAct","ACCEPT",0},

{"7_WanLanICMP","",0},

{"7_LanWanActiveSVC","000000",0},

{"7_LanWanFirewallEnable","0",0},

{"7_LanWanActiveDate","1111111",0},

{"7_LanWanActiveTime","00002359",0},

{"7_LanWanDefaultAct","ACCEPT",0},

{"7_LanWanICMP","",0},

{"7_FirewallLog","",0},

{"7_SystemLog","",0},

{"7_SystemCmd","",0},

{"7_UrlFilterEnable","0",0},

{"7_URLActiveDate","1111111",0},

{"7_URLActiveTime","00002359",0},

{"7_UrlFilterDefaultAct","DROP",0},

{"7_UrlListCount","0",0},

{"7_WanLanRuleCount","0",0},

{"7_LanWanRuleCount","0",0},

{"7_WanLocalFirewallEnable","0",0},

{"7_WanLocalActiveDate","1111111",0},

{"7_WanLocalActiveTime","00002359",0},

{"7_WanLocalDefaultAct","ACCEPT",0},

{"7_WanLocalICMP","",0},

{"7_WanLocalLog","None",0},

{"7_WanLocalRuleCount","0",0},

{"7_DmzEnable","0",0},

{"7_DmzDevices","Both",0},

{"7_DmzIP","192.168.2.1",0},

{"7_DmzMask","255.255.255.0",0},

{"7_DmzWanFirewallEnable","0",0},

{"7_DmzWanDefaultAct","ACCEPT",0},

{"7_DmzWanICMP","",0},

{"7_WanDmzDefaultAct","ACCEPT",0},

{"7_WanDmzICMP","",0},

{"7_WanDmzLog","None",0},

{"7_DmzWanRuleCount","0",0},

{"7_WanDmzRuleCount","0",0},

{"7_DmzLanFirewallEnable","0",0},

{"7_DmzLanDefaultAct","ACCEPT",0},

{"7_DmzLanICMP","",0},

{"7_LanDmzDefaultAct","ACCEPT",0},

{"7_LanDmzICMP","",0},

{"7_LanDmzLog","None",0},

{"7_DmzLanRuleCount","0",0},

{"7_LanDmzRuleCount","0",0},

{"7_WanLanSrcIP","",0},                          

{"7_WanLanSrcPort","",0},                          

{"7_WanLanDstIP","",0},                          

{"7_WanLanDstPort","",0},                          

{"7_WanLanProFlag","",0},                          

{"7_LanWanSrcIP","",0},                          

{"7_LanWanDstPort","",0},                          

{"7_LanWanProFlag","",0},                          

{"7_LanWanDescript","",0},                          

{"7_URLEntry","",0},                          

{"7_WanLocalSrcIP","",0},                          

{"7_WanLocalDstPort","",0},                          

{"7_WanLocalProFlag","",0},                          

{"7_DmzWanSrcIP","",0},                          

{"7_DmzWanSrcPort","",0},                          

{"7_DmzWanDstIP","",0},                          

{"7_DmzWanDstPort","",0},                          

{"7_DmzWanProFlag","",0},                          

{"7_WanDmzSrcIP","",0},                          

{"7_WanDmzSrcPort","",0},                          

{"7_WanDmzDstIP","",0},                          

{"7_WanDmzDstPort","",0},                          

{"7_WanDmzProFlag","",0},                          

{"7_DmzLanSrcIP","",0},                          

{"7_DmzLanSrcPort","",0},                          

{"7_DmzLanDstIP","",0},                          

{"7_DmzLanDstPort","",0},                          

{"7_DmzLanProFlag","",0},                          

{"7_LanDmzSrcIP","",0},                          

{"7_LanDmzSrcPort","",0},                          

{"7_LanDmzDstIP","",0},                          

{"7_LanDmzDstPort","",0},                          

{"7_LanDmzProFlag","",0},                          

{"sid_8","RouterConfig", 0},

{"8_GWStaticEnable","0",0},

{"8_GWStaticRIP","0",0},

{"8_GWStaticCount","0",0},

{"8_RipRedistStatic","0",0},

{"8_RipSRouteMetric","1",0},

{"8_RipSRouteAct","1",0},

{"8_RouteInfo","",0},

{"8_StaticRouteCount","0",0},

{"8_RipSRouteCount","0",0},

{"8_RipSRouteCount","0",0},

{"8_RipEnableWAN","0",0},

{"8_RipVersionWAN","Both",0},

{"8_RipSpWANEnable","1",0},

{"8_RipAuthMethodWAN","Disable",0},

{"8_RipAuthKeyWAN","",0},

{"8_RipEnableLAN","0",0},

{"8_RipVersionLAN","Both",0},

{"8_RipSpLANEnable","1",0},

{"8_RipAuthMethodLAN","Disable",0},

{"8_RipAuthKeyLAN","",0},

{"8_RipUpdate","30",0},

{"8_RipTimeout","180",0},

{"8_RipGarbage","120",0},

{"8_RipDRouteType0Act","0",0},

{"8_RipDRouteType1Act","0",0},

{"8_RipDRouteType2Act","0",0},

{"8_RipDRouteType3Act","0",0},

{"8_RipOffsetMetric","1",0},

{"8_RipOffsetIn","0",0},

{"8_RipOffsetOut","0",0},

{"8_RipDRouteCount","0",0},

{"8_RipOffsetCount","0",0},

{"8_GWStaticIP","",0},                          

{"8_GWStaticMask","",0},                          

{"8_GWStaticGW","",0},                          

{"8_GWStaticMT","",0},                          

{"8_GWStaticIF","",0},                          

{"8_StaticIP","",0},                          

{"8_StaticMask","0",0},                          

{"8_StaticGW","",0},                          

{"8_RipSRouteIP","",0},                          

{"8_RipSRouteMask","0",0},                          

{"8_RipDRouteType","",0},                          

{"8_RipDRouteIP","",0},                          

{"8_RipDRouteMask","0",0},                          

{"8_RipOffsetDir","",0},                          

{"8_RipOffsetIP","",0},                          

{"8_RipOffsetMask","0",0},                          

{"sid_9","LANHostConfig", 0},

{"9_x_LANDHCPClient","1",0},

{"9_IPRouters","192.168.1.1",0},

{"9_SubnetMask","255.255.255.0",0},

{"9_x_LANIPAddress","",0},

{"9_x_LANSubnetMask","",0},

{"9_x_LANGateway","",0},

{"9_x_HostName","",0},

{"9_x_Gateway","",0},

{"9_DHCPServerConfigurable","1",0},

{"9_DHCPRelay","1",0},

{"9_DHCPRelayIP","",0},

{"9_DomainName","",0},

{"9_MinAddress","192.168.1.2",0},

{"9_MaxAddress","192.168.1.254",0},

{"9_LeaseTime","86400",0},

{"9_x_LGateway","",0},

{"9_x_LDNSServer1","",0},

{"9_x_LDNSServer2","",0},

{"9_x_WINSServer","",0},

{"9_ManualDHCPEnable","0",0},

{"9_ManualDHCPCount","0",0},

{"9_ReservedAddressListCount","0",0},

{"9_DHCPLog","",0},

{"9_DmzDHCPServerConfigurable","1",0},

{"9_DmzDomainName","",0},

{"9_DmzMinAddress","192.168.2.2",0},

{"9_DmzMaxAddress","192.168.2.254",0},

{"9_DmzLeaseTime","86400",0},

{"9_x_DmzLDNSServer1","",0},

{"9_x_DmzLDNSServer2","",0},

{"9_x_DmzWINSServer","",0},

{"9_DmzManualDHCPEnable","1",0},

{"9_DmzManualDHCPCount","0",0},

{"9_DmzDHCPLog","",0},

{"9_x_UPnPEnable","1",0},

{"9_x_ServerLogEnable","",0},

{"9_x_TimeZone","GMT0",0},

{"9_x_TimerInterval","20",0},

{"9_x_NTPServer1","time.nist.gov",0},

{"9_x_NTPServer2","",0},

{"9_x_RedirectEnable","1",0},

{"9_x_RedirectTo","",0},

{"9_x_DDNSEnable","0",0},

{"9_x_DDNSServer","",0},

{"9_x_DDNSUserName","",0},

{"9_x_DDNSPassword","",0},

{"9_x_DDNSHostNames","",0},

{"9_x_DDNSWildcard","0",0},

{"9_x_DDNSStatus","",0},

{"9_ManualMac","",0},                          

{"9_ManualIP","",0},                          

{"9_DmzManualMac","",0},                          

{"9_DmzManualIP","",0},                          

{"9_ReservedAddresses","",0},                          

{"sid_10","WLANConfig11a", 0},

{"10_x_CountryCode","",0},

{"10_SSID","default",0},

{"10_Channel","52",0},

{"10_x_OutdoorChannel","0",0},

{"10_DataRate","0",0},

{"10_x_TurboMode","0",0},

{"10_AuthenticationMethod","Open System",0},

{"10_WEPType","None",0},

{"10_x_Phrase","",0},

{"10_WEPKey1","",0},

{"10_WEPKey2","",0},

{"10_WEPKey3","",0},

{"10_WEPKey4","",0},

{"10_WEPDefaultKey","Key1",0},

{"10_x_AESEnable","0",0},

{"10_x_BlockBCSSID","0",0},

{"10_WirelessLog","",0},

{"sid_11","DeviceSecurity11a", 0},

{"11_ACLMethod","disabled",0},

{"11_ApplyDevices","Both",0},

{"11_ACLListCount","0",0},

{"11_AESMethod","",0},

{"11_MacAddr","",0},                          

{"11_AESMacAddr","",0},                          

{"11_AESType","",0},                          

{"11_AESKey","",0},                          

{"sid_12","WLANAuthentication11a", 0},

{"12_ExAuthDBIPAddr","",0},

{"12_ExAuthDBPortNumber","1812",0},

{"12_ExAuthDBPassword","",0},

{"12_UseLocalAuthDatabase","0",0},

{"12_AuthDBUsername","",0},                          

{"12_AuthDBSecret","",0},                          

{"12_CertDBType","",0},                          

{"12_CertDBFile","",0},                          

{"sid_13","WLANConfig11b", 0},

{"13_Country","",0},

{"13_SSID","default",0},

{"13_x_APMode","0",0},

{"13_Channel","0",0},

{"13_x_BRApply","0",0},

{"13_x_BRestrict","0",0},

{"13_RBRListCount","0",0},

{"13_x_Mode11g","1",0},

{"13_x_GProtection","0",0},

{"13_AuthenticationMethod","open",0},

{"13_WPAType","0",0},

{"13_x_PSKKey","",0},

{"13_WEPType","0",0},

{"13_x_Phrase","",0},

{"13_WEPKey1","",0},

{"13_WEPKey2","",0},

{"13_WEPKey3","",0},

{"13_WEPKey4","",0},

{"13_WEPDefaultKey","1",0},

{"13_x_Rekey","0",0},

{"13_x_AfterBurner","off",0},

{"13_x_BlockBCSSID","0",0},

{"13_x_IsolateAP","0",0},

{"13_DataRateAll","0",0},

{"13_DataRate","default",0},

{"13_x_Frag","2346",0},

{"13_x_RTS","2347",0},

{"13_x_DTIM","1",0},

{"13_x_Beacon","100",0},

{"13_x_FrameBurst","0",0},

{"13_x_ExtendMode","ap",0},

{"13_x_RadioEnable","1",0},

{"13_x_RadioEnableDate","1111111",0},

{"13_x_RadioEnableTime","00002359",0},

{"13_x_RadioPower","19",0},

{"13_WirelessLog","",0},

{"13_WirelessCtrl","",0},

{"13_x_WMM","0",0},

{"13_x_NOACK","0",0},

{"13_x_WMMQ1","",0},

{"13_x_WMMQ2","",0},

{"13_x_WMMQ3","",0},

{"13_x_WMMQ4","",0},

{"13_x_STAWMMQ1","",0},

{"13_x_STAWMMQ2","",0},

{"13_x_STAWMMQ3","",0},

{"13_x_STAWMMQ4","",0},

{"13_x_Preauth","1",0},

{"13_x_Reauth","36000",0},

{"13_x_BRhwaddr","",0},                          

{"sid_13","WLANConfig11b", 0},

{"13_GEnable","1",0},

{"13_GSSID","guest",0},

{"13_GAuthenticationMethod","open",0},

{"13_GWPAType","0",0},

{"13_Gx_PSKKey","",0},

{"13_GWEPType","0",0},

{"13_Gx_Phrase","",0},

{"13_GWEPKey1","",0},

{"13_GWEPKey2","",0},

{"13_GWEPKey3","",0},

{"13_GWEPKey4","",0},

{"13_GWEPDefaultKey","1",0},

{"13_GIPRouters","192.168.1.1",0},

{"13_GSubnetMask","255.255.255.0",0},

{"13_GDHCPServerConfigurable","1",0},

{"13_GMinAddress","192.168.1.2",0},

{"13_GMaxAddress","192.168.1.254",0},

{"13_GLeaseTime","86400",0},

{"sid_14","DeviceSecurity11b", 0},

{"14_ACLMethod","disabled",0},

{"14_ApplyDevices","Both",0},

{"14_ACLListCount","0",0},

{"14_AESMethod","",0},

{"14_MacAddr","",0},                          

{"14_AESMacAddr","",0},                          

{"14_AESType","",0},                          

{"14_AESKey","",0},                          

{"sid_15","WLANAuthentication11b", 0},

{"15_ExAuthDBIPAddr","",0},

{"15_ExAuthDBPortNumber","1812",0},

{"15_ExAuthDBPassword","",0},

{"15_UseLocalAuthDatabase","0",0},

{"15_AuthDBUsername","",0},                          

{"15_AuthDBSecret","",0},                          

{"15_CertDBType","",0},                          

{"15_CertDBFile","",0},                          

{"sid_16","PrinterStatus", 0},

{"16_x_PrinterModel","",0},

{"16_x_PrinterStatus","",0},

{"16_x_PrinterUser","",0},

{"16_x_PrinterAction","",0},

{"16_x_WanCamEnable","1",0},

{"16_x_WEnable","1",0},

{"16_x_WModel","0",0},

{"16_x_WImageSize","1",0},

{"16_x_WSense","1",0},

{"16_x_WRecordTime","0",0},

{"16_x_WFreshTime","1",0},

{"16_x_WCaption","Web Camera Live Demo!!!",0},

{"16_x_WEBPort","7776",0},

{"16_x_WEBPass","0",0},

{"16_x_WPort","7777",0},

{"16_x_WSecurity","0",0},

{"16_x_WSecurityDate","1111111",0},

{"16_x_WSecurityTime","00002359",0},

{"16_x_WSendto","",0},

{"16_x_WServer","",0},

{"16_x_WSubject","Motion detection alert!!!",0},

{"16_x_WAttach","1",0},

{"16_x_WSignal","0",0},

{"16_x_WRemote","LAN Only",0},

{"16_x_WRemote1","",0},

{"16_x_WRemote2","",0},

{"16_x_WRemote3","",0},

{"16_x_WRemote4","",0},

{"16_x_WRemote5","",0},

{"16_x_WRemote6","",0},

{"16_x_FEject","",0},

{"16_x_FIsFtpServer","1",0},

{"16_x_FIsAnonymous","1",0},

{"16_x_FIsSuperuser","0",0},

{"16_x_FPort","21",0},

{"16_x_FMaxUsers","12",0},

{"16_x_FLoginTimeout","120",0},

{"16_x_FStayTimeout","240",0},

{"16_x_FInitScript","",0},

{"16_x_FLanguage","0",0},

{"16_x_FUserCount","0",0},

{"16_x_FBanIPCount","0",0},

{"16_x_QEnable","0",0},

{"16_x_QRuleCount","0",0},

{"16_x_UQRuleCount","0",0},

{"16_x_FUserName","",0},                          

{"16_x_FUserPass","",0},                          

{"16_x_FUserLogin","",0},                          

{"16_x_FUserRight","",0},                          

{"16_x_FBanIP","",0},                          

{"16_x_QIPAddress","",0},                          

{"16_x_QPort","",0},                          

{"16_x_QMaxBW","",0},                          

{"16_x_QMinBW","",0},                          

{"16_x_UQPort","",0},                          

{"16_x_UQMaxBW","",0},                          

{"16_x_UQMinBW","",0},                          

	{ 0, 0, 0 }
};
