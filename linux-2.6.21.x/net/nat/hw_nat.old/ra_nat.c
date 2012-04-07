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
  ra_nat.c

  Abstract:

  Revision History:
  Who         When            What
  --------    ----------      ----------------------------------------------
  Name        Date            Modification logs
  Steven Liu  2010-04-08      Support RT3883 + RT309x concurrent AP
  Steven Liu  2009-11-26      Support WiFi pseudo interface by using VLAN tag
  Steven Liu  2009-07-21      Support IPV6 Forwarding
  Steven Liu  2009-04-14      Support IPV6 Forwarding
  Steven Liu  2009-04-02      Support RT3883/RT3350
  Steven Liu  2008-03-19      Support RT3052
  Steven Liu  2007-09-25      Support RT2880_MP2
  Steven Liu  2006-10-06      Initial version
 *
 */

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/checksum.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>

#include "ra_nat.h"
#include "foe_fdb.h"
#include "frame_engine.h"
#include "hwnat_ioctl.h"
#include "acl_ioctl.h"
#include "ac_ioctl.h"
#include "acl_policy.h"
#include "mtr_policy.h"
#include "ac_policy.h"
#include "util.h"

/* please change it if you use another VLAN ID to identify LAN/WAN ports */
#define LAN_PORT_VLAN_ID	1
#define WAN_PORT_VLAN_ID	2

extern int (*ra_sw_nat_hook_rx) (struct sk_buff * skb);
extern int (*ra_sw_nat_hook_tx) (struct sk_buff * skb, int gmac_no);

struct FoeEntry    *PpeFoeBase;
dma_addr_t	    PpePhyFoeBase;
struct net_device  *DstPort[MAX_IF_NUM];
uint32_t	    DebugLevel=0;
uint32_t	    ChipVer=0;
uint32_t	    ChipId=0;

uint16_t GLOBAL_PRE_ACL_STR  = DFL_PRE_ACL_STR; 
uint16_t GLOBAL_PRE_ACL_END  = DFL_PRE_ACL_END; 
uint16_t GLOBAL_PRE_MTR_STR  = DFL_PRE_MTR_STR; 
uint16_t GLOBAL_PRE_MTR_END  = DFL_PRE_MTR_END; 
uint16_t GLOBAL_PRE_AC_STR   = DFL_PRE_AC_STR; 
uint16_t GLOBAL_PRE_AC_END   = DFL_PRE_AC_END; 
uint16_t GLOBAL_POST_MTR_STR = DFL_POST_MTR_STR; 
uint16_t GLOBAL_POST_MTR_END = DFL_POST_MTR_END; 
uint16_t GLOBAL_POST_AC_STR  = DFL_POST_AC_STR; 
uint16_t GLOBAL_POST_AC_END  = DFL_POST_AC_END; 

#if 0
void skb_dump(struct sk_buff* sk) {
        unsigned int i;

        printk("\nskb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
                sk->dev?sk->dev->name:"ip stack",sk->len,sk->truesize,
                skb_headroom(sk),skb_tailroom(sk));

        for(i=(unsigned int)sk->head;i<(unsigned int)sk->tail;i++) {
                if((i % 16) == 0)
		    printk("\n");

                if(i==(unsigned int)sk->head) printk("@h");
                if(i==(unsigned int)sk->data) printk("@d");
                if(i==(unsigned int)sk->mac.raw) printk("@m");
                printk("%02X-",*((unsigned char*)i));
        }
        printk("\n");
}
#endif

int RemoveVlanTag(struct sk_buff *skb)
{
    struct ethhdr *eth;
    struct vlan_ethhdr *veth;
    uint16_t VirIfIdx;

    veth = (struct vlan_ethhdr *)(skb->mac.raw);
    VirIfIdx = ntohs(veth->h_vlan_TCI);

    if (skb_cloned(skb) || skb_shared(skb)) {

	struct sk_buff *new_skb;
	new_skb = skb_copy(skb, GFP_ATOMIC);
	kfree_skb(skb);
	if (!new_skb)
	    return 0;
	skb = new_skb;
    }

    /* remove VLAN tag */
    skb->data= skb->mac.raw;
    skb->mac.raw += VLAN_HLEN;
    memmove(skb->mac.raw, skb->data, ETH_ALEN * 2);
    skb_pull(skb, VLAN_HLEN);
    skb->data += ETH_HLEN;  //pointer to layer3 header

    eth = (struct ethhdr *)(skb->mac.raw);
    skb->protocol = eth->h_proto;

    return VirIfIdx;

}
static void FoeAllocTbl(uint32_t NumOfEntry)
{
    uint32_t FoeTblSize;

    FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
    PpeFoeBase = dma_alloc_coherent(NULL, FoeTblSize, &PpePhyFoeBase, GFP_KERNEL);

    RegWrite(PPE_FOE_BASE, PpePhyFoeBase);
    memset(PpeFoeBase, 0, FoeTblSize);
}

static uint8_t *ShowCpuReason(struct sk_buff *skb)
{
    static uint8_t Buf[32];

    switch(FOE_AI(skb))
    {
    case TTL_0: /* 0x80 */
	return("TTL=0\n");
    case FOE_EBL_NOT_IPV4_HLEN5: /* 0x90 */
	return("FOE enable & not IPv4h5nf\n");
    case FOE_EBL_NOT_TCP_UDP_L4_READY: /* 0x91 */
	return("FOE enable & not TCP/UDP/L4_read\n");
    case TCP_SYN_FIN_RST: /* 0x92 */
	return("TCP SYN/FIN/RST\n");
    case UN_HIT: /* 0x93 */
	return("Un-hit\n");
    case HIT_UNBIND: /* 0x94 */
	return("Hit unbind\n");
    case HIT_UNBIND_RATE_REACH: /* 0x95 */
	return("Hit unbind & rate reach\n");
    case HIT_FIN:  /* 0x96 */
	return("Hit fin\n");
    case HIT_BIND_TTL_1: /* 0x97 */
	return("Hit bind & ttl=1 & ttl-1\n");
    case HIT_BIND_KEEPALIVE:  /* 0x98 */
	return("Hit bind & keep alive\n");
    case HIT_BIND_FORCE_TO_CPU: /* 0x99 */
	return("Hit bind & force to CPU\n");
    case ACL_FOE_TBL_ERR: /* 0x9A */
	return("acl link foe table error (!static & !unbind)\n");
    case ACL_TBL_TTL_1: /* 0x9B */
	return("acl link FOE table & TTL=1 & TTL-1\n");
    case ACL_ALERT_CPU: /* 0x9C */
	return("acl alert cpu\n");
    case NO_FORCE_DEST_PORT: /* 0xA0 */
	return("No force destination port\n");
    case ACL_FORCE_PRIORITY0: /* 0xA8 */
	return("ACL FORCE PRIORITY0\n");
    case ACL_FORCE_PRIORITY1: /* 0xA9 */
	return("ACL FORCE PRIORITY1\n");
    case ACL_FORCE_PRIORITY2: /* 0xAA */
	return("ACL FORCE PRIORITY2\n");
    case ACL_FORCE_PRIORITY3: /* 0xAB */
	return("ACL FORCE PRIORITY3\n");
    case ACL_FORCE_PRIORITY4: /* 0xAC */
	return("ACL FORCE PRIORITY4\n");
    case ACL_FORCE_PRIORITY5: /* 0xAD */
	return("ACL FORCE PRIORITY5\n");
    case ACL_FORCE_PRIORITY6: /* 0xAE */
	return("ACL FORCE PRIORITY6\n");
    case ACL_FORCE_PRIORITY7: /* 0xAF */
	return("ACL FORCE PRIORITY7\n");
    case EXCEED_MTU: /* 0xA1 */
	return("Exceed mtu\n");
    }

    sprintf(Buf,"CPU Reason Error - %X\n",FOE_AI(skb));
    return(Buf);
}


uint32_t FoeDumpPkt(struct sk_buff *skb)
{
#if 0  //dump related info from packet
    struct ethhdr *eth = NULL;
    struct vlan_hdr *vh1 = NULL;
    struct vlan_hdr *vh2 = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;

    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t pppoe_sid = 0;
    uint16_t eth_type=0;
    

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));


    eth_type=ntohs(skb->protocol);

    // Layer 2
    if(eth_type==ETH_P_8021Q) {
	vlan1_gap = VLAN_HLEN;
	vh1 = (struct vlan_hdr *)(skb->data);

	/* VLAN + PPPoE */
	if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
	    pppoe_gap = 8;
	    if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, 0)) {
		return 0;
	    }
	    /* Double VLAN = VLAN + VLAN */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
	    vlan2_gap = VLAN_HLEN;
	    vh2 = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

	    /* VLAN + VLAN + PPPoE */
	    if(ntohs(vh2->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
		pppoe_gap = 8;
		if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, 0)) {
		    return 0;
		}
		/* VLAN + VLAN + IP */
	    }else if(ntohs(vh2->h_vlan_encapsulated_proto)!=ETH_P_IP) {
		return 0;
	    }
	    /* VLAN + IP */
	}else if(ntohs(vh1->h_vlan_encapsulated_proto)!=ETH_P_IP) {
	    return 0;
	}
    }else if(eth_type != ETH_P_IP) {
	return 0;
    }
    
    eth = (struct ethhdr *)(skb->data-14) ; /* DA + SA + ETH_TYPE */

    // Layer 3
    iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);


    // Layer 4
    if(iph->protocol==IPPROTO_TCP) {
	th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else if(iph->protocol==IPPROTO_UDP) {
	uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
    }else { //Not TCP or UDP
	return 0;
    }

    if(vlan1_gap) {
	NAT_PRINT("VLAN1: %d\n",ntohs(vh1->h_vlan_TCI));
    }
    if(vlan2_gap) {
	NAT_PRINT("VLAN2: %d\n",ntohs(vh2->h_vlan_TCI));
    }
    if(pppoe_gap) {
	NAT_PRINT("PPPoE Session ID: %d\n", ntohs(pppoe_sid));
    }
    
    NAT_PRINT("----------------------------------\n");
    NAT_PRINT("SrcMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_source));
    NAT_PRINT("DstMac=%0X:%0X:%0X:%0X:%0X:%0X\n",MAC_ARG(eth->h_dest));
    NAT_PRINT("SrcIp:%s\n",Ip2Str(ntohl(iph->saddr)));
    NAT_PRINT("DstIp:%s\n",Ip2Str(ntohl(iph->daddr)));
    if(th!=NULL) {
	NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(th->source),ntohs(th->dest));
    }else {
	NAT_PRINT("SrcPort:%d Dstport:%d\n",ntohs(uh->source),ntohs(uh->dest));
    }

#else //dump related info from FoE table
    struct FoeEntry *foe_entry = &PpeFoeBase[FOE_ENTRY_NUM(skb)];

    NAT_PRINT("\nRx===<FOE_Entry=%d>=====\n",FOE_ENTRY_NUM(skb)); 
    NAT_PRINT("RcvIF=%s\n", skb->dev->name);
    NAT_PRINT("FOE_Entry=%d\n",FOE_ENTRY_NUM(skb));
    NAT_PRINT("FVLD=%d\n",FOE_FVLD(skb));
    NAT_PRINT("CPU Reason=%s",ShowCpuReason(skb));
    NAT_PRINT("ALG=%d\n",FOE_ALG(skb));
    NAT_PRINT("SP=%d\n",FOE_SP(skb));
    NAT_PRINT("AIS=%d\n",FOE_AIS(skb));

    NAT_PRINT("Information Block 1=%x\n",foe_entry->info_blk1);

    if(foe_entry->bfib1.fmt == IPV4_NAPT) {
	NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
	NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
	NAT_PRINT("SPORT=%d\n",foe_entry->sport);
	NAT_PRINT("DPORT=%d\n",foe_entry->dport);
    }else if(foe_entry->bfib1.fmt == IPV4_NAT) {
	NAT_PRINT("SIP=%s\n",Ip2Str(foe_entry->sip));
	NAT_PRINT("DIP=%s\n",Ip2Str(foe_entry->dip));
    }else if(foe_entry->bfib1.fmt == IPV6_ROUTING) {
	NAT_PRINT("IPv6_DIP0=%08X\n", foe_entry->ipv6_dip0);
	NAT_PRINT("IPv6_DIP1=%08X\n", foe_entry->ipv6_dip1);
	NAT_PRINT("IPv6_DIP2=%08X\n", foe_entry->ipv6_dip2);
	NAT_PRINT("IPv6_DIP3=%08X\n", foe_entry->ipv6_dip3);
    } else {
	NAT_PRINT("Wrong MFT value\n");
    }
#endif
    NAT_PRINT("==================================\n");

    return 1;

}

int32_t PpeRxHandler(struct sk_buff * skb)
{
    struct ethhdr *eth=NULL;
    struct vlan_hdr *vh = NULL;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;
    struct FoeEntry *foe_entry=NULL;


    uint32_t vlan1_gap = 0;
    uint32_t vlan2_gap = 0;
    uint32_t pppoe_gap=0;
    uint16_t eth_type=0;
    uint32_t SrcPortNo=0;
    uint16_t VirIfIdx=0;

    foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];
    eth_type=ntohs(skb->protocol);

    if(DebugLevel==1) {
       FoeDumpPkt(skb);
    }

    if( ((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) ||
			    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))){ 

#if defined  (CONFIG_RA_HW_NAT_WIFI)

	    if(skb->dev == DstPort[DP_RA0]) { VirIfIdx=DP_RA0;}
#if defined (CONFIG_RT2860V2_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RA1]) { VirIfIdx=DP_RA1; }
	    else if(skb->dev == DstPort[DP_RA2]) { VirIfIdx=DP_RA2; }
	    else if(skb->dev == DstPort[DP_RA3]) { VirIfIdx=DP_RA3; }
	    else if(skb->dev == DstPort[DP_RA4]) { VirIfIdx=DP_RA4; }
	    else if(skb->dev == DstPort[DP_RA5]) { VirIfIdx=DP_RA5; }
	    else if(skb->dev == DstPort[DP_RA6]) { VirIfIdx=DP_RA6; }
	    else if(skb->dev == DstPort[DP_RA7]) { VirIfIdx=DP_RA7; }
	    else if(skb->dev == DstPort[DP_RA8]) { VirIfIdx=DP_RA8; }
	    else if(skb->dev == DstPort[DP_RA9]) { VirIfIdx=DP_RA9; }
	    else if(skb->dev == DstPort[DP_RA10]) { VirIfIdx=DP_RA10; }
	    else if(skb->dev == DstPort[DP_RA11]) { VirIfIdx=DP_RA11; }
	    else if(skb->dev == DstPort[DP_RA12]) { VirIfIdx=DP_RA12; }
	    else if(skb->dev == DstPort[DP_RA13]) { VirIfIdx=DP_RA13; }
	    else if(skb->dev == DstPort[DP_RA14]) { VirIfIdx=DP_RA14; }
	    else if(skb->dev == DstPort[DP_RA15]) { VirIfIdx=DP_RA15; }
#endif // CONFIG_RT2860V2_AP_MBSS //
#if defined (CONFIG_RT2860V2_AP_WDS)
	    else if(skb->dev == DstPort[DP_WDS0]) { VirIfIdx=DP_WDS0; }
	    else if(skb->dev == DstPort[DP_WDS1]) { VirIfIdx=DP_WDS1; }
	    else if(skb->dev == DstPort[DP_WDS2]) { VirIfIdx=DP_WDS2; }
	    else if(skb->dev == DstPort[DP_WDS3]) { VirIfIdx=DP_WDS3; }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLI0]) { VirIfIdx=DP_APCLI0; }
#endif // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESH0]) { VirIfIdx=DP_MESH0; }
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	    else if(skb->dev == DstPort[DP_RAI0]) { VirIfIdx=DP_RAI0; }
#if defined (CONFIG_RT3090_AP_MBSS)
	    else if(skb->dev == DstPort[DP_RAI1]) { VirIfIdx=DP_RAI1; }
	    else if(skb->dev == DstPort[DP_RAI2]) { VirIfIdx=DP_RAI2; }
	    else if(skb->dev == DstPort[DP_RAI3]) { VirIfIdx=DP_RAI3; }
	    else if(skb->dev == DstPort[DP_RAI4]) { VirIfIdx=DP_RAI4; }
	    else if(skb->dev == DstPort[DP_RAI5]) { VirIfIdx=DP_RAI5; }
	    else if(skb->dev == DstPort[DP_RAI6]) { VirIfIdx=DP_RAI6; }
	    else if(skb->dev == DstPort[DP_RAI7]) { VirIfIdx=DP_RAI7; }
	    else if(skb->dev == DstPort[DP_RAI8]) { VirIfIdx=DP_RAI8; }
	    else if(skb->dev == DstPort[DP_RAI9]) { VirIfIdx=DP_RAI9; }
	    else if(skb->dev == DstPort[DP_RAI10]) { VirIfIdx=DP_RAI10; }
	    else if(skb->dev == DstPort[DP_RAI11]) { VirIfIdx=DP_RAI11; }
	    else if(skb->dev == DstPort[DP_RAI12]) { VirIfIdx=DP_RAI12; }
	    else if(skb->dev == DstPort[DP_RAI13]) { VirIfIdx=DP_RAI13; }
	    else if(skb->dev == DstPort[DP_RAI14]) { VirIfIdx=DP_RAI14; }
	    else if(skb->dev == DstPort[DP_RAI15]) { VirIfIdx=DP_RAI15; }
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	    else if(skb->dev == DstPort[DP_APCLII0]) { VirIfIdx=DP_APCLII0; }
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	    else if(skb->dev == DstPort[DP_MESHI0]) { VirIfIdx=DP_MESHI0; }
#endif // CONFIG_RT3090_AP_MESH //
	    else if(skb->dev == DstPort[DP_PCI]) { VirIfIdx=DP_PCI; }
	    else { printk("HNAT: The interface %s is unknown\n", skb->dev->name); }

	    skb_push(skb, ETH_HLEN);
	    skb = __vlan_put_tag(skb, VirIfIdx);

	    //redirect to PPE
	    FOE_AI(skb) = UN_HIT;
	    FOE_MAGIC_TAG(skb) = FOE_MAGIC_PPE;
	    skb->dev = DstPort[DP_GMAC];
	    skb->dev->hard_start_xmit(skb, skb->dev);
#else
	    return 1;
#endif // CONFIG_RA_HW_NAT_WIFI //

	    return 0;

    }
    
    if((FOE_AI(skb)==HIT_BIND_FORCE_TO_CPU)) {
	    skb->dev = DstPort[foe_entry->iblk2.act_dp];
	    skb_push(skb, ETH_HLEN); //pointer to layer2 header
	    skb->dev->hard_start_xmit(skb, skb->dev);
	    return 0;
    }

#if defined(CONFIG_RALINK_RT3883) || defined(CONFIG_RALINK_RT3352)
    if(IS_EXT_SW_EN(RegRead(FE_COS_MAP))){
	SrcPortNo=6;
    }
#endif

    if((FOE_AIS(skb) == 1) && (FOE_SP(skb) == SrcPortNo) && \
	FOE_AI(skb)!=HIT_BIND_KEEPALIVE) {

	VirIfIdx = RemoveVlanTag(skb);

	if(VirIfIdx < MAX_IF_NUM) {
	    skb->dev=DstPort[VirIfIdx];
	}else {
	    printk("HNAT: unknow interface (VirIfIdx=%d)\n", VirIfIdx);
	}
	
	eth=(struct ethhdr *)(skb->mac.raw);

	if(eth->h_dest[0] & 1)
	{
	    if(memcmp(eth->h_dest, skb->dev->broadcast, ETH_ALEN)==0){
		skb->pkt_type=PACKET_BROADCAST;
	    } else {
		skb->pkt_type=PACKET_MULTICAST;
	    }
	}else {

	    if(memcmp(eth->h_dest, skb->dev->dev_addr, ETH_ALEN)==0){
		skb->pkt_type=PACKET_HOST;
	    }else{
		skb->pkt_type=PACKET_OTHERHOST;
	    }
	}

	return 1;
    }



    if( (FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){

	    eth=(struct ethhdr *)(skb->data-ETH_HLEN);	 
					 
	    FoeGetMacInfo(eth->h_dest, foe_entry->smac_hi);	 
	    FoeGetMacInfo(eth->h_source, foe_entry->dmac_hi);

	    if(eth_type==ETH_P_8021Q) {
		    vlan1_gap = VLAN_HLEN;
		    vh = (struct vlan_hdr *) skb->data;

		    if(ntohs(vh->h_vlan_TCI)==LAN_PORT_VLAN_ID){
			    /* It make packet like coming from WAN port */
			    vh->h_vlan_TCI=htons(WAN_PORT_VLAN_ID);

		    } else {
			    /* It make packet like coming from LAN port */
			    vh->h_vlan_TCI=htons(LAN_PORT_VLAN_ID);
		    }

		    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
			    pppoe_gap = 8;
		    }else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
                            vlan2_gap = VLAN_HLEN;
                            vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

                            /* VLAN + VLAN + PPPoE */
                            if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
                                pppoe_gap = 8;
                            }else {
                                /* VLAN + VLAN + IP */
                                eth_type = ntohs(vh->h_vlan_encapsulated_proto);
                            }
		    }else {
                            /* VLAN + IP */
                            eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		    }
	    }

	    if(eth_type == ETH_P_IP) {
		iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

		//Recover to original layer 4 header 
		if (iph->protocol == IPPROTO_TCP) {
		    th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
		    FoeToOrgTcpHdr(foe_entry, iph, th);

		} else if (iph->protocol == IPPROTO_UDP) {
		    uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
		    FoeToOrgUdpHdr(foe_entry, iph, uh);
		}

		//Recover to original layer 3 header 
		FoeToOrgIpHdr(foe_entry,iph);
	    }else if(eth_type == ETH_P_IPV6) {
		/* Nothing to do */
	    }else {
		return 1;
	    }

	    skb->pkt_type=PACKET_HOST;	
	    return 1;

    }

#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
    if( (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) ) 
    {
        AclClassifyKey NewRateReach;
	eth=(struct ethhdr *)(skb->data-ETH_HLEN);
	
	memset(&NewRateReach, 0, sizeof(AclClassifyKey));
	memcpy(NewRateReach.Mac, eth->h_source,ETH_ALEN);

	if(eth_type==ETH_P_8021Q)
	{
	    vlan1_gap = VLAN_HLEN;
	    vh = (struct vlan_hdr *) skb->data;

	    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES) {
		pppoe_gap = 8;
	    }else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
		vlan2_gap = VLAN_HLEN;
		vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

		/* VLAN + VLAN + PPPoE */
		if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
		    pppoe_gap = 8;
		}else {
		    /* VLAN + VLAN + IP */
		    eth_type = ntohs(vh->h_vlan_encapsulated_proto);
		}
	    }else {
		/* VLAN + IP */
		eth_type = ntohs(vh->h_vlan_encapsulated_proto);
	    }
	}

	/*IPv4*/
	if(eth_type == ETH_P_IP)
	{
	    iph = (struct iphdr *) (skb->data + vlan1_gap + vlan2_gap + pppoe_gap);

	    NewRateReach.Sip = ntohl(iph->saddr);
	    NewRateReach.Dip = ntohl(iph->daddr);

	    if (iph->protocol == IPPROTO_TCP) 
	    {
		th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
		NewRateReach.Sp = ntohs(th->source);
		NewRateReach.Dp = ntohs(th->dest);
		NewRateReach.Proto = ACL_PROTO_TCP;
	    }
	    else if (iph->protocol == IPPROTO_UDP) 
	    {
		uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
		NewRateReach.Sp = ntohs(uh->source);
		NewRateReach.Dp = ntohs(uh->dest);
		NewRateReach.Proto = ACL_PROTO_UDP;

	    }

	}

	/*classify user priority*/
	FOE_SP(skb)= AclClassify(&NewRateReach);

	return 1;

    }
#endif

    return 1;
}

/* is_in = 1 --> in  */
/* is_in = 0 --> out */
int32_t GetPppoeSid(struct sk_buff *skb, uint32_t vlan_gap, 
		uint16_t *sid, uint32_t is_in)
{
	struct pppoe_hdr *peh = NULL;
	uint32_t offset = 0;

	if(!is_in) {
		offset = ETH_HLEN;
	}

	peh = (struct pppoe_hdr *) (skb->data + offset + vlan_gap);

	if(DebugLevel==1) { 
		NAT_PRINT("\n==============\n");
		NAT_PRINT(" Ver=%d\n",peh->ver);
		NAT_PRINT(" Type=%d\n",peh->type);
		NAT_PRINT(" Code=%d\n",peh->code);
		NAT_PRINT(" sid=%x\n",ntohs(peh->sid));
		NAT_PRINT(" Len=%d\n",ntohs(peh->length));
		NAT_PRINT(" tag_type=%x\n",ntohs(peh->tag[0].tag_type));
		NAT_PRINT(" tag_len=%d\n",ntohs(peh->tag[0].tag_len));
		NAT_PRINT("=================\n");
	}

	if (peh->ver != 1 || peh->type != 1
			|| (ntohs(peh->tag[0].tag_type) != PPP_IP)){
		return 1;
	}

	*sid = peh->sid;
	return 0;
}

int32_t PpeTxHandler(struct sk_buff *skb, int gmac_no)
{
	struct vlan_hdr *vh = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *th = NULL;
	struct udphdr *uh = NULL;
	struct ethhdr *eth = NULL;
	uint32_t vlan1_gap = 0;
	uint32_t vlan2_gap = 0;
	uint32_t pppoe_gap = 0;
	uint16_t pppoe_sid = 0;
	struct FoeEntry *foe_entry;
	uint32_t current_time;
	struct FoeEntry entry;
	uint16_t eth_type=0;
	uint32_t offset=0;
#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	uint32_t now=0;
#endif
    
	if(IS_MAGIC_TAG_VALID(skb) && (FOE_AI(skb)==HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb)==0)) 
	{
		eth = (struct ethhdr *) skb->data;
		eth_type=ntohs(eth->h_proto);
		foe_entry=&PpeFoeBase[FOE_ENTRY_NUM(skb)];

#if defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
#define SEMIAUTO_REFRESH_INTERVAL	30
		now = RegRead(FOE_TS_T)&0xFFFF;
		if(time_before((unsigned long)now, 
			    (unsigned long)foe_entry->tmp_buf.time_stamp 
			    + SEMIAUTO_REFRESH_INTERVAL)) {
		    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		    return 1;
		}
#endif
		//if this entry is already in binding state, skip it 
		if(foe_entry->bfib1.state == BIND) {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		}

		/* Get original setting */
		memcpy(&entry, foe_entry, sizeof(entry));


		/* Set Layer2 Info - DMAC, SMAC */
		FoeSetMacInfo(entry.dmac_hi,eth->h_dest);
		FoeSetMacInfo(entry.smac_hi,eth->h_source);

		/* Set VLAN Info - VLAN1/VLAN2 */
		if(eth_type==ETH_P_8021Q) {
			vlan1_gap = VLAN_HLEN;
			vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
			entry.vlan1 = ntohs(vh->h_vlan_TCI);

			/* VLAN + PPPoE */
			if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
				pppoe_gap = 8;
				if (GetPppoeSid(skb, vlan1_gap, &pppoe_sid, 0)) {
					memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
					return 1;
				}
				entry.pppoe_id = ntohs(pppoe_sid);
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			/* Double VLAN = VLAN + VLAN */
			}else if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_8021Q) {
			    vlan2_gap = VLAN_HLEN;
			    vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN);
			    entry.vlan2 = ntohs(vh->h_vlan_TCI);

			    /* VLAN + VLAN + PPPoE */
			    if(ntohs(vh->h_vlan_encapsulated_proto)==ETH_P_PPP_SES){
				pppoe_gap = 8;
				if (GetPppoeSid(skb, vlan2_gap, &pppoe_sid, 0)) {
					memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
					return 1;
				}
				entry.pppoe_id = ntohs(pppoe_sid);
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }else {
				/* VLAN + VLAN + IP */
				eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			    }
			}else {
			    /* VLAN + IP */
			    eth_type = ntohs(vh->h_vlan_encapsulated_proto);
			}
		}


		if(vlan1_gap) {
		    entry.bfib1.v1=MODIFY;
                } else {
		    entry.bfib1.v1 = DELETE ;
                }

                if(vlan2_gap) {
		    entry.bfib1.v2=MODIFY;
                } else {
		    entry.bfib1.v2 = DELETE ;
                }

		//remove vlan tag (used to identify actual incoming interface)
                if(!vlan1_gap && !vlan2_gap) {
                        entry.bfib1.v1 = DELETE ;
                }

		if(pppoe_gap) { 
			entry.bfib1.pppoe = INSERT ;
		} else { 
			entry.bfib1.pppoe = DELETE ;
		}

		/* Set Layer3 Info */
		if( (eth_type == ETH_P_IP) || (eth_type == ETH_P_PPP_SES)) {
		    iph = (struct iphdr *) (skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
		    entry.new_sip = ntohl(iph->saddr);
		    entry.new_dip = ntohl(iph->daddr);

		    /* Set Layer4 Info - NEW_SPORT, NEW_DPORT */
		    if (iph->protocol == IPPROTO_TCP) {
			th = (struct tcphdr *) ((uint8_t *) iph + iph->ihl * 4);
			entry.new_sport = ntohs(th->source);
			entry.new_dport = ntohs(th->dest);
			entry.bfib1.t_u = TCP;
		    } else if (iph->protocol == IPPROTO_UDP) {
			uh = (struct udphdr *) ((uint8_t *) iph + iph->ihl * 4);
			entry.new_sport = ntohs(uh->source);
			entry.new_dport = ntohs(uh->dest);
			entry.bfib1.t_u = UDP;
		    }else {
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
		    }
		} else if (eth_type == ETH_P_IPV6) {
		    /* Nothing to do */
		} else {
		    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		    return 1;
		}

		current_time =RegRead(FOE_TS_T)&0xFFFF;
		entry.bfib1.time_stamp=(uint16_t)current_time;
              
#if defined (CONFIG_RA_HW_NAT_ACL2UP_HELPER)
		/*set user priority*/
		entry.iblk2.up = FOE_SP(skb);
		entry.iblk2.fp = 1;
#endif
		/* Set Information block 2 */
		entry.iblk2.fd=1;
		if( (strncmp(skb->dev->name,"ra",2)==0) ||
		    (strncmp(skb->dev->name,"wds",3)==0) ||
		    (strncmp(skb->dev->name,"mesh",4)==0) ||
		    (strncmp(skb->dev->name,"apcli",5)==0) ||
		    (skb->dev == DstPort[DP_PCI])) {
#if defined  (CONFIG_RA_HW_NAT_WIFI)
			entry.iblk2.dp=0; /* cpu */
#else
			memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
			return 1;
#endif // CONFIG_RA_HW_NAT_WIFI //

		}else {
#if defined (CONFIG_RAETH_GMAC2) // RT3883
			entry.iblk2.dp=gmac_no; //GMAC1 or GMAC2
#elif defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3883) 
			entry.iblk2.dp=1; /* GMAC1 */
#else // RT3052, RT335x
			if((entry.vlan1 & VLAN_VID_MASK)==LAN_PORT_VLAN_ID) {
			    entry.iblk2.dp=1; /* LAN traffic use VirtualPort1 in GMAC1*/
			}else if((entry.vlan1 & VLAN_VID_MASK)==WAN_PORT_VLAN_ID) {
			    entry.iblk2.dp=2; /* WAN traffic use VirtualPort2 in GMAC1*/
			}else {
			    entry.iblk2.dp=1; 
			}
#endif
		}

		if(IS_FORCE_ACL_TO_UP(skb))
		{
			entry.iblk2.up=(GET_ACL_TO_UP(skb)); 
			entry.iblk2.fp=1; 
		}



#define CB_OFF  10
#define RTMP_GET_PACKET_IF(skb)                 skb->cb[CB_OFF+6]
#define MIN_NET_DEVICE_FOR_MBSSID               0x00
#define MIN_NET_DEVICE_FOR_WDS                  0x10
#define MIN_NET_DEVICE_FOR_APCLI                0x20
#define MIN_NET_DEVICE_FOR_MESH                 0x30

		/* Set actual output port info */
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
		if(strncmp(skb->dev->name, "rai", 3)==0) {
#if defined (CONFIG_RT3090_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
		    }else 
#endif // CONFIG_RT3090_AP_MESH //

#if defined (CONFIG_RT3090_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
		    }else 
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_WDS)
	   	    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
		    }else 
#endif // CONFIG_RT3090_AP_WDS //
		    {
			offset = RTMP_GET_PACKET_IF(skb) + DP_RAI0;
		    }
		}else 
#endif // CONFIG_RT3090_AP || CONFIG_RT3090_AP_MODULE // 

		if(strncmp(skb->dev->name, "ra", 2)==0) {
#if defined (CONFIG_RT2860V2_AP_MESH)
		    if(RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
		    }else 
#endif // CONFIG_RT2860V2_AP_MESH //
#if defined (CONFIG_RT2860V2_AP_APCLI)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
		    }else 
#endif  // CONFIG_RT2860V2_AP_APCLI //
#if defined (CONFIG_RT2860V2_AP_WDS)
		    if (RTMP_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS) {
			offset = (RTMP_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
		    }else 
#endif // CONFIG_RT2860V2_AP_WDS //
		    {
			offset = RTMP_GET_PACKET_IF(skb) + DP_RA0;
		    }
		}else if(strncmp(skb->dev->name, "eth2", 4)==0) {
			offset = DP_GMAC; //for debugging purpose
#ifdef CONFIG_RAETH_GMAC2
		}else if(strncmp(skb->dev->name, "eth3", 4)==0) {
			offset = DP_GMAC2; //for debugging purpose
#endif
		}else if(strncmp(skb->dev->name, "eth0", 4)==0) {
			offset = DP_PCI; //for debugging purpose
		}else {
		    printk("HNAT: unknow interface %s\n",skb->dev->name);
		}
		
		entry.iblk2.act_dp = offset; 

		entry.bfib1.ttl = DFL_FOE_TTL_REGEN;

#if defined (CONFIG_RA_HW_NAT_AUTO_BIND)
		entry.bfib1.state = BIND;
#elif defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
		memcpy(&entry.tmp_buf, &entry.bfib1 , sizeof(entry.bfib1));
#endif
		memcpy(foe_entry, &entry, sizeof(entry));
//		FoeDumpEntry(FOE_ENTRY_NUM(skb));	

	}else if( (FOE_AI(skb)==HIT_BIND_KEEPALIVE) && (DFL_FOE_KA_ORG==0)){
		memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
		return 0;
	}

	return 1;
}

void  PpeSetFoeEbl(uint32_t FoeEbl)
{
	uint32_t PpeFlowSet=0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if(FoeEbl==1) {
		PpeFlowSet = (BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet|= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);

#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet |= (BIT_IPV6_FOE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_FUC_FOE | BIT_FMC_FOE | BIT_FBC_FOE);
		PpeFlowSet &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
#if defined(CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_FOE_EN);
#endif
	}

	RegWrite( PPE_FLOW_SET, PpeFlowSet);
}


static void PpeSetFoeHashMode(uint32_t HashMode)
{

	FoeAllocTbl(FOE_4TB_SIZ);

	switch(FOE_4TB_SIZ){
	case 1024:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_1K, 0, 3);
		break;
	case 2048:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_2K, 0, 3);
		break;
	case 4096:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_4K, 0, 3);
		break;
	case 8192:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_8K, 0, 3);
		break;
	case 16384:
		RegModifyBits(PPE_FOE_CFG, FoeTblSize_16K, 0, 3);
		break;
	}

	if(ChipId==RT2880 && ChipVer < RT2880_MP2) {
		if(FOE_4TB_SIZ!=1024){
			HashMode=~HashMode;
		}
	}

	RegModifyBits(PPE_FOE_CFG, HashMode , 3, 1);

#if defined (CONFIG_RA_HW_NAT_AUTO_BIND) || defined (CONFIG_RA_HW_NAT_SEMIAUTO_BIND)
	RegModifyBits(PPE_FOE_CFG, FWD_CPU_BUILD_ENTRY, 4, 2);
#elif defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	RegModifyBits(PPE_FOE_CFG, ONLY_FWD_CPU, 4, 2);
#else
	#error "Please Choice Action for FoE search miss"
#endif
}

static void PpeSetAgeOut(void)
{
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UNB_AGE, 8, 1);

	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_MNP, 16, 16);

	RegModifyBits(PPE_FOE_UNB_AGE, DFL_FOE_UNB_DLTA, 0, 8);

#if defined (CONFIG_RA_HW_NAT_AUTO_BIND)
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_TCP_AGE, 9, 1);

	RegModifyBits(PPE_FOE_CFG, DFL_FOE_UDP_AGE, 10, 1);

	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	RegModifyBits(PPE_FOE_BND_AGE1, DFL_FOE_UDP_DLTA, 0, 16);

	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);

	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_TCP_DLTA, 0, 16);
#endif

#if defined (CONFIG_RA_HW_NAT_MANUAL_BIND)
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

	RegModifyBits(PPE_FOE_BND_AGE2, DFL_FOE_FIN_DLTA, 16, 16);
#endif
}

static void PpeSetFoeKa(void)
{
	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_ORG, 12, 1);

	RegModifyBits(PPE_FOE_CFG, DFL_FOE_KA_EN, 13, 1);

	RegModifyBits(PPE_FOE_CFG, DFL_FOE_ACL_AGE, 14, 1);
	
	RegModifyBits(PPE_FOE_KA, DFL_FOE_KA_T, 0, 16);

	RegModifyBits(PPE_FOE_KA, DFL_FOE_TCP_KA, 16, 8);

	RegModifyBits(PPE_FOE_KA, DFL_FOE_UDP_KA, 24, 8);

}

static void PpeSetFoeBindRate(uint32_t FoeBindRate)
{

	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_QURT_LMT, 0, 14);

	RegModifyBits(PPE_FOE_LMT1, DFL_FOE_HALF_LMT, 16, 14);

	RegModifyBits(PPE_FOE_LMT2, DFL_FOE_FULL_LMT, 0, 14);

	RegWrite(PPE_FOE_BNDR, FoeBindRate);
}


static void PpeSetFoeGloCfgEbl(uint32_t Ebl)
{
	if(Ebl==1) {
		RegModifyBits(PPE_GLO_CFG, 1, 0, 1);

		RegModifyBits(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);

		RegModifyBits(PPE_GLO_CFG, DFL_VPRI_EN, 8, 1);

		RegModifyBits(PPE_GLO_CFG, DFL_DPRI_EN, 9, 1);

		RegModifyBits(PPE_GLO_CFG, DFL_REG_VPRI, 10, 1);

		RegModifyBits(PPE_GLO_CFG, DFL_REG_DSCP, 11, 1);
		
		RegModifyBits(PPE_GLO_CFG, DFL_RED_MODE, 12, 2);

		RegModifyBits(PPE_GLO_CFG, DFL_ACL_PRI_EN, 14, 1);
#ifdef CONFIG_RALINK_RT3052
		RegModifyBits(RALINK_ETH_SW_BASE+0xC8, 0x0, 8, 2);
		
		RegModifyBits(RALINK_ETH_SW_BASE+0x10, 0x0, 0, 16);
#endif

	} else {
		RegModifyBits(PPE_GLO_CFG, 0, 0, 1);
	}

}

#ifndef CONFIG_RALINK_RT3052_MP
static void  PpeSetUserPriority(void)
{
    /* Set weight of decision in resolution */
    RegWrite(UP_RES, DFL_UP_RES);
   
    /* Set DSCP to User priority mapping table */ 
    RegWrite(DSCP0_7_MAP_UP, DFL_DSCP0_7_UP);
    RegWrite(DSCP24_31_MAP_UP, DFL_DSCP24_31_UP);
    RegWrite(DSCP8_15_MAP_UP, DFL_DSCP8_15_UP);
    RegWrite(DSCP16_23_MAP_UP, DFL_DSCP16_23_UP);
    RegWrite(DSCP32_39_MAP_UP, DFL_DSCP32_39_UP);
    RegWrite(DSCP40_47_MAP_UP, DFL_DSCP40_47_UP);
    RegWrite(DSCP48_55_MAP_UP, DFL_DSCP48_55_UP);
    RegWrite(DSCP56_63_MAP_UP, DFL_DSCP56_63_UP);
   
#if 0 
    /* Set boundary and range of auto user priority */ 
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_BND1, 16, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND2, 0, 14);
    RegModifyBits(AUTO_UP_CFG2, DFL_ATUP_BND3, 16, 14);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R1_UP, 0, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R2_UP, 4, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R3_UP, 8, 3);
    RegModifyBits(AUTO_UP_CFG1, DFL_ATUP_R4_UP, 12, 3);
#endif

    /* Set mapping table of user priority to vlan priority */
    RegModifyBits(UP_MAP_VPRI, DFL_UP0_VPRI, 0, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP1_VPRI, 4, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP2_VPRI, 8, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP3_VPRI, 12, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP4_VPRI, 16, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP5_VPRI, 20, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP6_VPRI, 24, 3);
    RegModifyBits(UP_MAP_VPRI, DFL_UP7_VPRI, 28, 3);
   
    /* Set mapping table of user priority to in-profile DSCP */
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP0_IDSCP, 0, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP1_IDSCP, 8, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP2_IDSCP, 16, 6);
    RegModifyBits(UP0_3_MAP_IDSCP, DFL_UP3_IDSCP, 24, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP4_IDSCP, 0, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP5_IDSCP, 8, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP6_IDSCP, 16, 6);
    RegModifyBits(UP4_7_MAP_IDSCP, DFL_UP7_IDSCP, 24, 6);
     
    /* Set mapping table of user priority to out-profile DSCP */
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP0_ODSCP, 0, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP1_ODSCP, 8, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP2_ODSCP, 16, 6);
    RegModifyBits(UP0_3_MAP_ODSCP, DFL_UP3_ODSCP, 24, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP4_ODSCP, 0, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP5_ODSCP, 8, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP6_ODSCP, 16, 6);
    RegModifyBits(UP4_7_MAP_ODSCP, DFL_UP7_ODSCP, 24, 6);

    /* Set mapping table of user priority to access category */
    RegModifyBits(UP_MAP_AC, DFL_UP0_AC, 0, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP1_AC, 2, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP2_AC, 4, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP3_AC, 6, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP4_AC, 8, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP5_AC, 10, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP6_AC, 12, 2);
    RegModifyBits(UP_MAP_AC, DFL_UP7_AC, 14, 2);
}
#endif

static void FoeFreeTbl(uint32_t NumOfEntry)
{
	uint32_t FoeTblSize;

	FoeTblSize = NumOfEntry * sizeof(struct FoeEntry);
	dma_free_coherent(NULL, FoeTblSize, PpeFoeBase, PpePhyFoeBase);
	RegWrite( PPE_FOE_BASE, 0);
}

static int32_t PpeEngStart(void)
{
	/* Set PPE Flow Set */
	PpeSetFoeEbl(1);

	/* Set PPE FOE Hash Mode */
	PpeSetFoeHashMode(DFL_FOE_HASH_MODE);

	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	/* Set Auto Age-Out Function */
	PpeSetAgeOut();

	/* Set PPE FOE KEEPALIVE TIMER */
	PpeSetFoeKa(); 

	/* Set PPE FOE Bind Rate */
	PpeSetFoeBindRate(DFL_FOE_BNDR); 

	/* Set PPE Global Configuration */
	PpeSetFoeGloCfgEbl(1);

#ifndef CONFIG_RALINK_RT3052_MP
	/* Set User Priority related register */
	PpeSetUserPriority();
#endif
	return 0;
}

static int32_t PpeEngStop(void)
{
	/* Set PPE FOE ENABLE */
	PpeSetFoeGloCfgEbl(0);

	/* Set PPE Flow Set */
	PpeSetFoeEbl(0);
	
	/* Set default index in policy table */
	PpeSetPreAclEbl(0);
	PpeSetPreMtrEbl(0);
	PpeSetPostMtrEbl(0);
	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	/* Free FOE table */ 
	FoeFreeTbl(FOE_4TB_SIZ);

	return 0;
}

static void PpeSetDstPort(uint32_t Ebl)
{
    if(Ebl) {
	DstPort[DP_RA0]=dev_get_by_name("ra0"); 
#if defined (CONFIG_RT2860V2_AP_MBSS)
	DstPort[DP_RA1]=dev_get_by_name("ra1"); 
	DstPort[DP_RA2]=dev_get_by_name("ra2"); 
	DstPort[DP_RA3]=dev_get_by_name("ra3"); 
	DstPort[DP_RA4]=dev_get_by_name("ra4"); 
	DstPort[DP_RA5]=dev_get_by_name("ra5"); 
	DstPort[DP_RA6]=dev_get_by_name("ra6"); 
	DstPort[DP_RA7]=dev_get_by_name("ra7"); 
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352)
	DstPort[DP_RA8]=dev_get_by_name("ra8"); 
	DstPort[DP_RA9]=dev_get_by_name("ra9"); 
	DstPort[DP_RA10]=dev_get_by_name("ra10"); 
	DstPort[DP_RA11]=dev_get_by_name("ra11"); 
	DstPort[DP_RA12]=dev_get_by_name("ra12"); 
	DstPort[DP_RA13]=dev_get_by_name("ra13"); 
	DstPort[DP_RA14]=dev_get_by_name("ra14"); 
	DstPort[DP_RA15]=dev_get_by_name("ra15"); 
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	DstPort[DP_WDS0]=dev_get_by_name("wds0");
	DstPort[DP_WDS1]=dev_get_by_name("wds1");
	DstPort[DP_WDS2]=dev_get_by_name("wds2");
	DstPort[DP_WDS3]=dev_get_by_name("wds3");
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	DstPort[DP_APCLI0]=dev_get_by_name("apcli0");
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	DstPort[DP_MESH0]=dev_get_by_name("mesh0");
#endif
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	DstPort[DP_RAI0]=dev_get_by_name("rai0"); 
#if defined (CONFIG_RT3090_AP_MBSS)
	DstPort[DP_RAI1]=dev_get_by_name("rai1"); 
	DstPort[DP_RAI2]=dev_get_by_name("rai2"); 
	DstPort[DP_RAI3]=dev_get_by_name("rai3"); 
	DstPort[DP_RAI4]=dev_get_by_name("rai4"); 
	DstPort[DP_RAI5]=dev_get_by_name("rai5"); 
	DstPort[DP_RAI6]=dev_get_by_name("rai6"); 
	DstPort[DP_RAI7]=dev_get_by_name("rai7"); 
	DstPort[DP_RAI8]=dev_get_by_name("rai8"); 
	DstPort[DP_RAI9]=dev_get_by_name("rai9"); 
	DstPort[DP_RAI10]=dev_get_by_name("rai10"); 
	DstPort[DP_RAI11]=dev_get_by_name("rai11"); 
	DstPort[DP_RAI12]=dev_get_by_name("rai12"); 
	DstPort[DP_RAI13]=dev_get_by_name("rai13"); 
	DstPort[DP_RAI14]=dev_get_by_name("rai14"); 
	DstPort[DP_RAI15]=dev_get_by_name("rai15"); 
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	DstPort[DP_APCLII0]=dev_get_by_name("apclii0");
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	DstPort[DP_MESHI0]=dev_get_by_name("meshi0");
#endif // CONFIG_RT3090_AP_MESH //
	DstPort[DP_GMAC]=dev_get_by_name("eth2");
#ifdef CONFIG_RAETH_GMAC2
	DstPort[DP_GMAC2]=dev_get_by_name("eth3");
#endif
	DstPort[DP_PCI]=dev_get_by_name("eth0"); // PCI interface name
    }else {
	if(DstPort[DP_RA0]!=NULL) { dev_put(DstPort[DP_RA0]); }
#if defined (CONFIG_RT2860V2_AP_MBSS)
	if(DstPort[DP_RA1]!=NULL) { dev_put(DstPort[DP_RA1]); }
	if(DstPort[DP_RA2]!=NULL) { dev_put(DstPort[DP_RA2]); }
	if(DstPort[DP_RA3]!=NULL) { dev_put(DstPort[DP_RA3]); }
	if(DstPort[DP_RA4]!=NULL) { dev_put(DstPort[DP_RA4]); }
	if(DstPort[DP_RA5]!=NULL) { dev_put(DstPort[DP_RA5]); }
	if(DstPort[DP_RA6]!=NULL) { dev_put(DstPort[DP_RA6]); }
	if(DstPort[DP_RA7]!=NULL) { dev_put(DstPort[DP_RA7]); }
#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352)
	if(DstPort[DP_RA8]!=NULL) { dev_put(DstPort[DP_RA8]); }
	if(DstPort[DP_RA9]!=NULL) { dev_put(DstPort[DP_RA9]); }
	if(DstPort[DP_RA10]!=NULL) { dev_put(DstPort[DP_RA10]); }
	if(DstPort[DP_RA11]!=NULL) { dev_put(DstPort[DP_RA11]); }
	if(DstPort[DP_RA12]!=NULL) { dev_put(DstPort[DP_RA12]); }
	if(DstPort[DP_RA13]!=NULL) { dev_put(DstPort[DP_RA13]); }
	if(DstPort[DP_RA14]!=NULL) { dev_put(DstPort[DP_RA14]); }
	if(DstPort[DP_RA15]!=NULL) { dev_put(DstPort[DP_RA15]); }
#endif
#endif
#if defined (CONFIG_RT2860V2_AP_WDS)
	if(DstPort[DP_WDS0]!=NULL) { dev_put(DstPort[DP_WDS0]); }
	if(DstPort[DP_WDS1]!=NULL) { dev_put(DstPort[DP_WDS1]); }
	if(DstPort[DP_WDS2]!=NULL) { dev_put(DstPort[DP_WDS2]); }
	if(DstPort[DP_WDS3]!=NULL) { dev_put(DstPort[DP_WDS3]); }
#endif
#if defined (CONFIG_RT2860V2_AP_APCLI)
	if(DstPort[DP_APCLI0]!=NULL) { dev_put(DstPort[DP_APCLI0]); }
#endif
#if defined (CONFIG_RT2860V2_AP_MESH)
	if(DstPort[DP_MESH0]!=NULL) { dev_put(DstPort[DP_MESH0]); }
#endif
#if defined (CONFIG_RT3090_AP) || defined (CONFIG_RT3090_AP_MODULE)
	if(DstPort[DP_RAI0]!=NULL) { dev_put(DstPort[DP_RAI0]); }
#if defined (CONFIG_RT3090_AP_MBSS)
	if(DstPort[DP_RAI1]!=NULL) { dev_put(DstPort[DP_RAI1]); }
	if(DstPort[DP_RAI2]!=NULL) { dev_put(DstPort[DP_RAI2]); }
	if(DstPort[DP_RAI3]!=NULL) { dev_put(DstPort[DP_RAI3]); }
	if(DstPort[DP_RAI4]!=NULL) { dev_put(DstPort[DP_RAI4]); }
	if(DstPort[DP_RAI5]!=NULL) { dev_put(DstPort[DP_RAI5]); }
	if(DstPort[DP_RAI6]!=NULL) { dev_put(DstPort[DP_RAI6]); }
	if(DstPort[DP_RAI7]!=NULL) { dev_put(DstPort[DP_RAI7]); }
	if(DstPort[DP_RAI8]!=NULL) { dev_put(DstPort[DP_RAI8]); }
	if(DstPort[DP_RAI9]!=NULL) { dev_put(DstPort[DP_RAI9]); }
	if(DstPort[DP_RAI10]!=NULL) { dev_put(DstPort[DP_RAI10]); }
	if(DstPort[DP_RAI11]!=NULL) { dev_put(DstPort[DP_RAI11]); }
	if(DstPort[DP_RAI12]!=NULL) { dev_put(DstPort[DP_RAI12]); }
	if(DstPort[DP_RAI13]!=NULL) { dev_put(DstPort[DP_RAI13]); }
	if(DstPort[DP_RAI14]!=NULL) { dev_put(DstPort[DP_RAI14]); }
	if(DstPort[DP_RAI15]!=NULL) { dev_put(DstPort[DP_RAI15]); }
#endif // CONFIG_RT3090_AP_MBSS //
#endif
#if defined (CONFIG_RT3090_AP_APCLI)
	if(DstPort[DP_APCLII0]!=NULL) { dev_put(DstPort[DP_APCLII0]); }
#endif // CONFIG_RT3090_AP_APCLI //
#if defined (CONFIG_RT3090_AP_MESH)
	if(DstPort[DP_MESHI0]!=NULL) { dev_put(DstPort[DP_MESHI0]); }
#endif // CONFIG_RT3090_AP_MESH //
	if(DstPort[DP_GMAC]!=NULL) { dev_put(DstPort[DP_GMAC]); }
#ifdef CONFIG_RAETH_GMAC2
	if(DstPort[DP_GMAC2]!=NULL) { dev_put(DstPort[DP_GMAC2]); }
#endif
	if(DstPort[DP_PCI]!=NULL) { dev_put(DstPort[DP_PCI]); }
    }

}

static uint32_t SetGdmaFwd(uint32_t Ebl) 
{
	uint32_t data=0;

	data=RegRead(GDMA1_FWD_CFG);

	if(Ebl) {	
	    data |= GDM1_UFRC_P_PPE;
	    data |= GDM1_BFRC_P_PPE;
	    data |= GDM1_MFRC_P_PPE;
	    data |= GDM1_OFRC_P_PPE;

	}else {
	    data &= ~GDM1_UFRC_P_PPE;
	    data &= ~GDM1_BFRC_P_PPE;
	    data &= ~GDM1_MFRC_P_PPE;
	    data &= ~GDM1_OFRC_P_PPE;
	
	}

	RegWrite(GDMA1_FWD_CFG, data);

#ifdef CONFIG_RAETH_GMAC2
	data=RegRead(GDMA2_FWD_CFG);

	if(Ebl) {	
	    data |= GDM1_UFRC_P_PPE;
	    data |= GDM1_BFRC_P_PPE;
	    data |= GDM1_MFRC_P_PPE;
	    data |= GDM1_OFRC_P_PPE;

	}else {
	    data &= ~GDM1_UFRC_P_PPE;
	    data &= ~GDM1_BFRC_P_PPE;
	    data &= ~GDM1_MFRC_P_PPE;
	    data &= ~GDM1_OFRC_P_PPE;
	
	}
	RegWrite(GDMA2_FWD_CFG, data);
#endif

	return 0;  
}

static int32_t GetChipInfo(void)
{
    uint8_t Id[10];

    memset(Id, 0, sizeof(Id));
    strncpy(Id, (char *)CHIPID, 8);
    ChipVer = RegRead(REVID);

    NAT_PRINT("CHIPID=%s\n", Id);

    if(strcmp(Id,"RT2880  ")==0) {
	ChipId=RT2880;
    }else if(strcmp(Id,"RT3052  ")==0) {
	ChipId=RT3052;
    }else if(strcmp(Id,"RT3883  ")==0) {
	ChipId=RT3883;
    }else if(strcmp(Id,"RT3350  ")==0) {
	ChipId=RT3350;
    }else if(strcmp(Id,"RT3352  ")==0) {
	ChipId=RT3352;
    }else {
	return 0;
    }

    return 1;
}


/*
 * PPE Enabled: GMAC<->PPE<->CPU
 * PPE Disabled: GMAC<->CPU
 */
static int32_t PpeInitMod(void)
{

    /* Get Chip Information */
    if(!GetChipInfo()) {
	NAT_PRINT("===========================\n");
	NAT_PRINT("This Chip is not supported.\n");
	NAT_PRINT("===========================\n");
	return 1;
    }

    NAT_PRINT("Ralink HW NAT Module Enabled\n");

    //Get net_device structure of Dest Port 
    PpeSetDstPort(1);

    /* Register ioctl handler */
    PpeRegIoctlHandler();
    AclRegIoctlHandler();
    AcRegIoctlHandler();
    MtrRegIoctlHandler();

    /* Initialize PPE related register */
    PpeEngStart();

    /* Register RX/TX hook point */
    ra_sw_nat_hook_tx = PpeTxHandler;
    ra_sw_nat_hook_rx = PpeRxHandler;
    
    /* Set GMAC fowrards packet to PPE */
    SetGdmaFwd(1);

    return 0;
}

static void PpeCleanupMod(void)
{
    NAT_PRINT("Ralink HW NAT Module Disabled\n");

    /* Set GMAC fowrards packet to CPU */
    SetGdmaFwd(0);

    /* Unregister RX/TX hook point */
    ra_sw_nat_hook_rx = NULL;
    ra_sw_nat_hook_tx = NULL;

    /* Restore PPE related register */
    PpeEngStop();

    /* Unregister ioctl handler */
    PpeUnRegIoctlHandler();
    AclUnRegIoctlHandler();
    AcUnRegIoctlHandler();
    MtrUnRegIoctlHandler();

    //Release net_device structure of Dest Port 
    PpeSetDstPort(0);

}

/*HNAT QOS*/
int PpeSetDscpRemarkEbl(uint32_t enable)
{
    /* Re-generate DSCP */
    RegModifyBits(PPE_GLO_CFG, enable, 11, 1);
    return HWNAT_SUCCESS;
}

int PpeSetVpriRemarkEbl(uint32_t enable)
{
    /* Re-generate VLAN Priority */
    RegModifyBits(PPE_GLO_CFG, enable, 10, 1);
    return HWNAT_SUCCESS;
}

int PpeSetWeightFOE(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, FUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightACL(uint32_t weight)
{ 
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, AUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}    

int PpeSetWeightDSCP(uint32_t weight)
{
    RegModifyBits(UP_RES, weight, DUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetWeightVPRI(uint32_t weight)
{
    /* Set weight of decision in resolution */
    RegModifyBits(UP_RES, weight, VUP_WT_OFFSET, 3);
    return HWNAT_SUCCESS;
}

int PpeSetDSCP_UP(uint32_t DSCP_SET, unsigned char UP)
{ 
    int DSCP_UP;

    DSCP_UP = ((UP<<0) | (UP<<4) | (UP<<8) | (UP<<12)\
	    | (UP<<16) | (UP<<20) | (UP<<24) | (UP<<28));
    /* Set DSCP to User priority mapping table */ 
    switch(DSCP_SET)
    {	
    case 0:	
	RegWrite(DSCP0_7_MAP_UP, DSCP_UP);
	break;
    case 1:
	RegWrite(DSCP8_15_MAP_UP, DSCP_UP);
	break;
    case 2:
	RegWrite(DSCP16_23_MAP_UP, DSCP_UP);
	break;
    case 3:
	RegWrite(DSCP24_31_MAP_UP, DSCP_UP);
	break;
    case 4:
	RegWrite(DSCP32_39_MAP_UP, DSCP_UP);
	break;
    case 5:
	RegWrite(DSCP40_47_MAP_UP, DSCP_UP);
	break;
    case 6:
	RegWrite(DSCP48_55_MAP_UP, DSCP_UP);
	break;
    case 7:
	RegWrite(DSCP56_63_MAP_UP, DSCP_UP);
	break;
    default:

	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_IDSCP(uint32_t UP, uint32_t IDSCP)
{    
    /* Set mapping table of user priority to in-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_IDSCP, IDSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_IDSCP, IDSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}
int PpeSetUP_ODSCP(uint32_t UP, uint32_t ODSCP)
{    
    /* Set mapping table of user priority to out-profile DSCP */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 1:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 2:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 3:
	RegModifyBits(UP0_3_MAP_ODSCP, ODSCP, 24, 6);
	break;
    case 4:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 0, 6);
	break;
    case 5:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 8, 6);
	break;
    case 6:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 16, 6);
	break;
    case 7:
	RegModifyBits(UP4_7_MAP_ODSCP, ODSCP, 24, 6);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_VPRI(uint32_t UP, uint32_t VPRI)
{    
    /* Set mapping table of user priority to vlan priority */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_VPRI, VPRI, 0, 3);
	break;
    case 1:
	RegModifyBits(UP_MAP_VPRI, VPRI, 4, 3);
	break;
    case 2:
	RegModifyBits(UP_MAP_VPRI, VPRI, 8, 3);
	break;
    case 3:
	RegModifyBits(UP_MAP_VPRI, VPRI, 12, 3);
	break;
    case 4:
	RegModifyBits(UP_MAP_VPRI, VPRI, 16, 3);
	break;
    case 5:
	RegModifyBits(UP_MAP_VPRI, VPRI, 20, 3);
	break;
    case 6:
	RegModifyBits(UP_MAP_VPRI, VPRI, 24, 3);
	break;
    case 7:
	RegModifyBits(UP_MAP_VPRI, VPRI, 28, 3);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetUP_AC(uint32_t UP, uint32_t AC)
{
    /* Set mapping table of user priority to access category */
    switch(UP)
    {
    case 0:
	RegModifyBits(UP_MAP_AC, AC, 0, 2);
	break;
    case 1:
	RegModifyBits(UP_MAP_AC, AC, 2, 2);
	break;
    case 2:
	RegModifyBits(UP_MAP_AC, AC, 4, 2);
	break;
    case 3:
	RegModifyBits(UP_MAP_AC, AC, 6, 2);
	break;
    case 4:
	RegModifyBits(UP_MAP_AC, AC, 8, 2);
	break;
    case 5:
	RegModifyBits(UP_MAP_AC, AC, 10, 2);
	break;
    case 6:
	RegModifyBits(UP_MAP_AC, AC, 12, 2);
	break;
    case 7:
	RegModifyBits(UP_MAP_AC, AC, 14, 2);
	break;
    default:
	break;
    }
    return HWNAT_SUCCESS;
}

int PpeSetSchMode(uint32_t policy)
{
    /* Set GDMA1&2 Schduling Mode */
    RegModifyBits(GDMA1_SCH_CFG, policy, 24, 2);
    RegModifyBits(GDMA2_SCH_CFG, policy, 24, 2);

    return HWNAT_SUCCESS;
}

int PpeSetSchWeight(uint8_t W0, uint8_t W1, uint8_t W2, uint8_t W3)
{
    /* Set GDMA1 Schduling Weight */
    RegModifyBits(GDMA1_SCH_CFG, W0, 0, 3);
    RegModifyBits(GDMA1_SCH_CFG, W1, 4, 3);
    RegModifyBits(GDMA1_SCH_CFG, W2, 8, 3);
    RegModifyBits(GDMA1_SCH_CFG, W3, 12, 3);

    /* Set GDMA2 Schduling Weight */
    RegModifyBits(GDMA2_SCH_CFG, W0, 0, 3);
    RegModifyBits(GDMA2_SCH_CFG, W1, 4, 3);
    RegModifyBits(GDMA2_SCH_CFG, W2, 8, 3);
    RegModifyBits(GDMA2_SCH_CFG, W3, 12, 3);


    return HWNAT_SUCCESS;
}

#if 1

int PpeSetBindThreshold(uint32_t threshold)
{
   /* Set reach bind rate for unbind state */
    RegWrite(PPE_FOE_BNDR, threshold);

    return HWNAT_SUCCESS;
}

int PpeSetMaxEntryLimit(uint32_t full, uint32_t half, uint32_t qurt)
{
	/* Allowed max entries to be build during a time stamp unit */

	/* smaller than 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, qurt, 0, 14);

	/* between 1/2 and 1/4 of total entries */
	RegModifyBits(PPE_FOE_LMT1, half, 16, 14);

	/* between full and 1/2 of total entries */
	RegModifyBits(PPE_FOE_LMT2, full, 0, 14);

    return HWNAT_SUCCESS;
}

int PpeSetRuleSize(uint16_t pre_acl, uint16_t pre_meter, uint16_t pre_ac, uint16_t post_meter, uint16_t post_ac)
{

/* Pre Access Control List Rule Start Index */
	GLOBAL_PRE_ACL_STR = 0;

    /* Pre Access Control List Rule End Index */
	GLOBAL_PRE_ACL_END = GLOBAL_PRE_ACL_STR;

    /* Pre Meter Rule Start Index */
	GLOBAL_PRE_MTR_STR = GLOBAL_PRE_ACL_STR + pre_acl;
     /* Pre Meter Rule End Index */
	GLOBAL_PRE_MTR_END = GLOBAL_PRE_MTR_STR;

    /* Pre Accounting Rule Start Index */
	GLOBAL_PRE_AC_STR = GLOBAL_PRE_MTR_STR + pre_meter;

    /* Pre Accounting Rule End Index */
	GLOBAL_PRE_AC_END = GLOBAL_PRE_AC_STR;

    /* Post Meter Rule Start Index */
	GLOBAL_POST_MTR_STR = GLOBAL_PRE_AC_STR + pre_ac;

    /* Post Meter Rule End Index */
	GLOBAL_POST_MTR_END = GLOBAL_POST_MTR_STR;

    /* Post Accounting Rule Start Index */
	GLOBAL_POST_AC_STR = GLOBAL_POST_MTR_STR + post_meter;

    /* Post Accounting Rule End Index */
	GLOBAL_POST_AC_END = GLOBAL_POST_AC_STR;




    /* Set Pre ACL Table */
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_STR, 0, 9);
    RegModifyBits(PPE_PRE_ACL, GLOBAL_PRE_ACL_END, 16, 9);
    /* Set Pre AC Table */
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_STR, 0, 9);
    RegModifyBits(PPE_PRE_AC, GLOBAL_PRE_AC_END, 16, 9);

    /* Set Post AC Table */
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_STR, 0, 9);
    RegModifyBits(PPE_POST_AC, GLOBAL_POST_AC_END, 16, 9);
    /* Set Pre MTR Table */
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_STR, 0, 9);
    RegModifyBits(PPE_PRE_MTR, GLOBAL_PRE_MTR_END, 16, 9);

    /* Set Post MTR Table */
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_STR, 0, 9);
    RegModifyBits(PPE_POST_MTR, GLOBAL_POST_MTR_END, 16, 9);



    return HWNAT_SUCCESS;
}

int PpeSetKaInterval(uint8_t tcp_ka, uint8_t udp_ka)
{
	/* Keep alive time for bind FOE TCP entry */
	RegModifyBits(PPE_FOE_KA, tcp_ka, 16, 8);

	/* Keep alive timer for bind FOE UDP entry */
	RegModifyBits(PPE_FOE_KA, udp_ka, 24, 8);


    return HWNAT_SUCCESS;
}

int PpeSetUnbindLifeTime(uint8_t lifetime)
{
	/* set Delta time for aging out an unbind FOE entry */
	RegModifyBits(PPE_FOE_UNB_AGE, lifetime, 0, 8);

    return HWNAT_SUCCESS;
}

int PpeSetBindLifetime(uint16_t tcp_life, uint16_t udp_life, uint16_t fin_life)
{

	/* set Delta time for aging out an bind UDP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE1, udp_life, 0, 16);

	/* set Delta time for aging out an bind TCP FIN FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, fin_life, 16, 16);

	/* set Delta time for aging out an bind TCP FOE entry */
	RegModifyBits(PPE_FOE_BND_AGE2, tcp_life, 0, 16);




    return HWNAT_SUCCESS;
}


#endif
module_init(PpeInitMod);
module_exit(PpeCleanupMod);

MODULE_AUTHOR("Steven Liu/Kurtis Ke");
MODULE_LICENSE("Proprietary");
MODULE_DESCRIPTION("Ralink Hardware NAT v0.7\n");

