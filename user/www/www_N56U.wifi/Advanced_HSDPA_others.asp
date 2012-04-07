<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Wireless Router <#Web_Title#> - <#menu5_4_4#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/dongle_list.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>';

wan_proto_t = '<% nvram_get_x("Layer3Forwarding", "wan_proto_t"); %>';
wan_proto_t = wan_proto_t.toLowerCase();

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>]; // [[MAC, associated, authorized], ...]
var isp = '<% nvram_get_x("General", "hsdpa_isp"); %>';
var apn = '<% nvram_get_x("General", "hsdpa_apn"); %>';
var dialnum = '<% nvram_get_x("General", "hsdpa_dialnum"); %>';

function initial(){
        show_banner(1);
        show_menu(5, 4, 4);
        show_footer();
        show_ISP_list();
				show_APN_list();
				checkCookie();
        enable_auto_hint(21, 7);
        document.form.Dev3G.disabled = (document.form.wan_proto.value == "3g")?false:true;
        //document.form.Dev3G.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_country.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_isp.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_apn.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.wan_3g_pin.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_dialnum.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_user.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_pass.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";        
}

function switch_hsdap_mode(mode){
        document.form.Dev3G.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_country.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_isp.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_apn.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.wan_3g_pin.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_dialnum.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_user.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";
        document.form.hsdpa_pass.disabled = (document.form.wan_proto.value == "3g")?false:true;
        $("hsdpa_hint").style.display = (document.form.wan_proto.value == "3g")?"":"none";             
        
        if(document.form.wan_proto.value != "3g"){
                if(wan_proto_t == "automatic ip"){
                        document.form.wan_proto.options[0].value = "dhcp";
                }
                else if(wan_proto_t){
                        document.form.wan_proto.options[0].value = wan_proto_t;
                }
        }
}

function getCookie(c_name){
	if (document.cookie.length>0){
		c_start=document.cookie.indexOf(c_name + "=");
		if (c_start!=-1){
			c_start=c_start + c_name.length+1;
			c_end=document.cookie.indexOf(";",c_start);
			if (c_end==-1) c_end=document.cookie.length;
				return unescape(document.cookie.substring(c_start,c_end));
		}
	}
	return "";
}

function setCookie(c_name,value){
	document.cookie = c_name + "=" + value;
}

function checkCookie(){
	var cookie = getCookie("switch3g");
	if (cookie == "0"){
		showLoading(50);
		setCookie("switch3g", "1");
	}
}

function applyRule(){
        showLoading(); 
        if(document.form.wan_proto.value == "3g" && document.form.wan_proto.value != "<% nvram_get_x("Layer3Forwarding", "wan_proto"); %>")
        	setCookie("switch3g", "0");
	//alert(document.form.wan_proto.value);
        document.form.action_mode.value = " Apply ";
        document.form.current_page.value = "/Advanced_HSDPA_others.asp";
        document.form.next_page.value = "";

        document.form.submit();
}

function done_validating(action){
        refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(21, 7);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">

<input type="hidden" name="current_page" value="Advanced_HSDPA_others.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="General;Layer3Forwarding;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="computer_nameb" value="">
<input type="hidden" name="computer_name2" value="<% nvram_get_x("Storage", "computer_nameb"); %>">
<input type="hidden" name="computer_name3" value="<% nvram_get_x("Storage", "computer_name"); %>">
<!--input type="hidden" name="st_samba_workgroupb" value="">
<input type="hidden" name="samba_workgroup2" value="<% nvram_get_x("Storage", "st_samba_workgroupb"); %>">
<input type="hidden" name="samba_workgroup3" value="<% nvram_get_x("Storage", "st_samba_workgroup"); %>"-->

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
        <td width="23">&nbsp;</td>
        
        <!--=====Beginning of Main Menu=====-->
        <td valign="top" width="202">
          <div id="mainMenu"></div>
          <div id="subMenu"></div>
        </td>
        
    <td valign="top">
        <div id="tabMenu" class="submenuBlock"></div>
                <br />
                <!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
                <td align="left" valign="top" >
                
<table width="90%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
        <thead>
        <tr>
                <td><#menu5_4_4#></td>
        </tr>
        </thead>
        <tbody>
          <tr>
            <td bgcolor="#FFFFFF"><#HSDPAConfig_hsdpa_mode_itemdesc#></td>
          </tr>
        </tbody>
        <tr>
                <td bgcolor="#FFFFFF">
                        <table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
                                <tr>
                                        <th width="40%">
                                                <a class="hintstyle" href="javascript:openHint(21,1);"><#HSDPAConfig_hsdpa_enable_itemname#></a>
                                        </th>
                                        <td>
                                                <select name="wan_proto" class="input" onchange="switch_hsdap_mode(this.value);">
                                                        <option value="dhcp" <% nvram_match_x("Storage", "wan_proto", "dhcp", "selected"); %>><#WLANConfig11b_WirelessCtrl_buttonname#></option>
                                                        <option value="3g" <% nvram_match_x("Storage", "wan_proto", "3g", "selected"); %>><#WLANConfig11b_WirelessCtrl_button1name#></option>
                                                </select>
                                                <br/><span id="hsdpa_hint" style="display:none;"><#HSDPAConfig_hsdpa_enable_hint1#></span>
                                        </td>
                                </tr>
                                
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,13);"><#HSDPAConfig_USBAdapter_itemname#></a></th>
                                <td>
                        <select name="Dev3G" class="input" disabled="disabled">
			<option value="AUTO" id="AUTO" <% nvram_match_x("General","Dev3G", "AUTO","selected"); %>>AUTO</option>
                        <option value="ASUS-T500" <% nvram_match_x("General","Dev3G", "ASUS-T500","selected"); %>>ASUS T500/T600</option>
                        <option value="BandLuxe-C120" <% nvram_match_x("General","Dev3G", "BandLuxe-C120","selected"); %>>BandLuxe C100,C120</option>
                        <option value="BandLuxe-C170" <% nvram_match_x("General","Dev3G", "BandLuxe-C170","selected"); %>>BandLuxe C170,C270</option>
                        <!--option value="Option-GlobeSurfer-Icon" <% nvram_match_x("General","Dev3G", "Option-GlobeSurfer-Icon","selected"); %>>Option GlobeSurfer Icon</option>
                        <option value="Option-GlobeSurfer-Icon-7.2" <% nvram_match_x("General","Dev3G", "Option-GlobeSurfer-Icon-7.2","selected"); %>>Option GlobeSurfer Icon 7.2</option>
                        <option value="Option-GlobeTrotter-GT-MAX-3.6" <% nvram_match_x("General","Dev3G", "Option-GlobeTrotter-GT-MAX-3.6","selected"); %>>Option GlobeTrotter GT MAX 3.6</option>
                        <option value="Option-GlobeTrotter-GT-MAX-7.2" <% nvram_match_x("General","Dev3G", "Option-GlobeTrotter-GT-MAX-7.2","selected"); %>>Option GlobeTrotter GT MAX 7.2</option>
                        <option value="Option-GlobeTrotter-EXPRESS-7.2" <% nvram_match_x("General","Dev3G", "Option-GlobeTrotter-EXPRESS-7.2","selected"); %>>Option GlobeTrotter EXPRESS 7.2</option>
                        <option value="Option-iCON-210" <% nvram_match_x("General","Dev3G", "Option-iCON-210","selected"); %>>Option iCON 210 / PROLiNK PHS100 / Hyundai Mobile MB-810</option>
                        <option value="Option-GlobeTrotter-HSUPA-Modem" <% nvram_match_x("General","Dev3G", "Option-GlobeTrotter-HSUPA-Modem","selected"); %>>Option GlobeTrotter HSUPA Modem</option>
                        <option value="Option-iCON-401" <% nvram_match_x("General","Dev3G", "Option-iCON-401","selected"); %>>Option iCON 401</option>
                        <option value="Vodafone-K3760" <% nvram_match_x("General","Dev3G", "Vodafone-K3760","selected"); %>>Vodafone K3760</option>
                        <option value="ATT-USBConnect-Quicksilver" <% nvram_match_x("General","Dev3G", "ATT-USBConnect-Quicksilver","selected"); %>>AT&amp;T USBConnect Quicksilver</option-->
			<option value="Huawei-E1550" <% nvram_match_x("General","Dev3G", "Huawei-E1550","selected"); %>>HUAWEI E1550</option>
                        <option value="HUAWEI-E160G" <% nvram_match_x("General","Dev3G", "HUAWEI-E160G","selected"); %>>HUAWEI E160G</option>
                        <option value="Huawei-E161" <% nvram_match_x("General","Dev3G", "Huawei-E161","selected"); %>>HUAWEI E161</option>
                        <option value="HUAWEI-E169" <% nvram_match_x("General","Dev3G", "HUAWEI-E169","selected"); %>>HUAWEI E169</option>
                        <option value="Huawei-E176" <% nvram_match_x("General","Dev3G", "Huawei-E176","selected"); %>>HUAWEI E176</option>
                        <option value="Huawei-E180" <% nvram_match_x("General","Dev3G", "Huawei-E180","selected"); %>>HUAWEI E180</option>
                        <option value="HUAWEI-E220" <% nvram_match_x("General","Dev3G", "HUAWEI-E220","selected"); %>>HUAWEI E220,E156G,E230,E270,E870</option>
                        <option value="Huawei-K3520" <% nvram_match_x("General","Dev3G", "Huawei-K3520","selected"); %>>Huawei-K3520</option>
                        <option value="MU-Q101" <% nvram_match_x("General","Dev3G", "MU-Q101","selected"); %>>NU MU-Q101</option>
                        <option value="OPTION-ICON225" <% nvram_match_x("General","Dev3G", "OPTION-ICON225","selected"); %>>OPTION ICON 225</option>
                        <!--option value="Huawei-E630" <% nvram_match_x("General","Dev3G", "Huawei-E630","selected"); %>>Huawei E630</option>
                        <option value="Huawei-E270" <% nvram_match_x("General","Dev3G", "Huawei-E270","selected"); %>>Huawei E270+,E1762,E1820</option>
                        <option value="Huawei-E1612" <% nvram_match_x("General","Dev3G", "Huawei-E1612","selected"); %>>HUAWEI E1612</option>
                        <option value="Huawei-E1690" <% nvram_match_x("General","Dev3G", "Huawei-E1690","selected"); %>>Huawei E1690,E1692</option>
                        <option value="Huawei-K3765" <% nvram_match_x("General","Dev3G", "Huawei-K3765","selected"); %>>Huawei K3765</option-->
                        <!--option value="Huawei-K4505" <% nvram_match_x("General","Dev3G", "Huawei-K4505","selected"); %>>Huawei K4505</option>
                        <option value="ZTE-MF620" <% nvram_match_x("General","Dev3G", "ZTE-MF620","selected"); %>>ZTE MF620</option>
                        <option value="ZTE-MF622" <% nvram_match_x("General","Dev3G", "ZTE-MF622","selected"); %>>ZTE MF622</option>
                        <option value="ZTE-MF626" <% nvram_match_x("General","Dev3G", "ZTE-MF626","selected"); %>>ZTE MF626,MF628+,MF633,MF636,MF637</option>
                        <option value="ZTE-MF628" <% nvram_match_x("General","Dev3G", "ZTE-MF628","selected"); %>>ZTE MF628</option-->
                        <!--option value="ZTE-AC8710" <% nvram_match_x("General","Dev3G", "ZTE-AC8710","selected"); %>>ZTE AC8710,AC2726</option>
                        <option value="ZTE-AC2710" <% nvram_match_x("General","Dev3G", "ZTE-AC2710","selected"); %>>ZTE AC2710</option>
                        <option value="ZTE-6535-Z" <% nvram_match_x("General","Dev3G", "ZTE-6535-Z","selected"); %>>ZTE 6535-Z</option>
                        <option value="ZTE-K3520-Z" <% nvram_match_x("General","Dev3G", "ZTE-K3520-Z","selected"); %>>ZTE K3520-Z</option>
                        <option value="ZTE-MF110" <% nvram_match_x("General","Dev3G", "ZTE-MF110","selected"); %>>ZTE MF110</option>
                        <option value="ZTE-K3565" <% nvram_match_x("General","Dev3G", "ZTE-K3565","selected"); %>>ZTE K3565</option>
                        <option value="ONDA-MT503HS" <% nvram_match_x("General","Dev3G", "ONDA-MT503HS","selected"); %>>ONDA MT503HS</option>
                        <option value="ONDA-MT505UP" <% nvram_match_x("General","Dev3G", "ONDA-MT505UP","selected"); %>>ONDA MT505UP</option>
                        <option value="Novatel-Wireless-Ovation-MC950D-HSUPA" <% nvram_match_x("General","Dev3G", "Novatel-Wireless-Ovation-MC950D-HSUPA","selected"); %>>Novatel Wireless Ovation MC950D,XU950D,930D</option>
                        <option value="Novatel-U727" <% nvram_match_x("General","Dev3G", "Novatel-U727","selected"); %>>Novatel U727 USB modem</option>
                        <option value="Novatel-MC990D" <% nvram_match_x("General","Dev3G", "Novatel-MC990D","selected"); %>>Novatel MC990D</option>
                        <option value="Novatel-U760" <% nvram_match_x("General","Dev3G", "Novatel-U760","selected"); %>>Novatel U760</option>
                        <option value="Alcatel-X020" <% nvram_match_x("General","Dev3G", "Alcatel-X020","selected"); %>>Alcatel One Touch X02, X030</option>
                        <option value="Alcatel-X200" <% nvram_match_x("General","Dev3G", "Alcatel-X200","selected"); %>>Alcatel X200,X060S</option>
                        <option value="AnyDATA-ADU-500A" <% nvram_match_x("General","Dev3G", "AnyDATA-ADU-500A","selected"); %>>AnyDATA ADU-500A,ADU-510A,ADU-510L,ADU-520A</option>
                        <option value="Solomon-S3Gm-660" <% nvram_match_x("General","Dev3G", "Solomon-S3Gm-660","selected"); %>>Solomon S3Gm-660</option>
                        <option value="C-motechD-50" <% nvram_match_x("General","Dev3G", "C-motechD-50","selected"); %>>C-motech D-50</option>
                        <option value="C-motech-CGU-628" <% nvram_match_x("General","Dev3G", "C-motech-CGU-628","selected"); %>>C-motech CGU-628</option>
                        <option value="Toshiba-G450" <% nvram_match_x("General","Dev3G", "Toshiba-G450","selected"); %>>Toshiba G450</option>
                        <option value="UTStarcom-UM175" <% nvram_match_x("General","Dev3G", "UTStarcom-UM175","selected"); %>>UTStarcom UM175</option>
                        <option value="Hummer-DTM5731" <% nvram_match_x("General","Dev3G", "Hummer-DTM5731","selected"); %>>Hummer DTM5731</option>
                        <option value="A-Link-3GU" <% nvram_match_x("General","Dev3G", "A-Link-3GU","selected"); %>>A-Link 3GU</option>
                        <option value="Sierra-Wireless-Compass-597" <% nvram_match_x("General","Dev3G", "Sierra-Wireless-Compass-597","selected"); %>>Sierra Wireless Compass 597</option>
                        <option value="Sierra-881U" <% nvram_match_x("General","Dev3G", "Sierra-881U","selected"); %>>Sierra Wireless AirCard 881U</option>
                        <option value="Sony-Ericsson-W910i" <% nvram_match_x("General","Dev3G", "Sony-Ericsson-W910i","selected"); %>>Sony-Ericsson-W910i</option>
                        <option value="Sony-Ericsson-MD400" <% nvram_match_x("General","Dev3G", "Sony-Ericsson-MD400","selected"); %>>Sony Ericsson MD400</option>
                        <option value="LG-LDU-1900D" <% nvram_match_x("General","Dev3G", "LG-LDU-1900D","selected"); %>>LG LDU-1900D EV-DO</option>
                        <option value="Samsung-SGH-Z810" <% nvram_match_x("General","Dev3G", "Samsung-SGH-Z810","selected"); %>>Samsung SGH-Z810 USB</option>
                        <option value="MobiData-MBD-200HU" <% nvram_match_x("General","Dev3G", "MobiData-MBD-200HU","selected"); %>>MobiData MBD-200HU</option>
                        <option value="ST-Mobile" <% nvram_match_x("General","Dev3G", "ST-Mobile","selected"); %>>ST Mobile Connect HSUPA USB Modem</option>
                        <option value="MyWave-SW006" <% nvram_match_x("General","Dev3G", "MyWave-SW006","selected"); %>>MyWave SW006</option>
                        <option value="Cricket-A600" <% nvram_match_x("General","Dev3G", "Cricket-A600","selected"); %>>Cricket A600</option>
                        <option value="EpiValley-SEC-7089" <% nvram_match_x("General","Dev3G", "EpiValley-SEC-7089","selected"); %>>EpiValley SEC-7089</option>
                        <option value="Samsung-U209" <% nvram_match_x("General","Dev3G", "Samsung-U209","selected"); %>>Samsung U209</option>
                        <option value="D-Link-DWM-162-U5" <% nvram_match_x("General","Dev3G", "D-Link-DWM-162-U5","selected"); %>>D-Link DWM-162-U5</option>
                        <option value="Novatel-MC760" <% nvram_match_x("General","Dev3G", "Novatel-MC760","selected"); %>>Novatel MC760</option>
                        <option value="Philips-TalkTalk" <% nvram_match_x("General","Dev3G", "Philips-TalkTalk","selected"); %>>Philips TalkTalk</option>
                        <option value="HuaXing-E600" <% nvram_match_x("General","Dev3G", "HuaXing-E600","selected"); %>>HuaXing E600</option>
                        <option value="C-motech-CHU-629S" <% nvram_match_x("General","Dev3G", "C-motech-CHU-629S","selected"); %>>C-motech CHU-629S</option>
                        <option value="Sagem-9520" <% nvram_match_x("General","Dev3G", "Sagem-9520","selected"); %>>Sagem F-ST 9520-35-GLR</option>
                        <option value="Nokia-CS-15" <% nvram_match_x("General","Dev3G", "Nokia-CS-15","selected"); %>>Nokia CS-15</option>
                        <option value="Vodafone-MD950" <% nvram_match_x("General","Dev3G", "Vodafone-MD950","selected"); %>>Vodafone MD950</option>
                        <option value="Siptune-LM-75" <% nvram_match_x("General","Dev3G", "Siptune-LM-75","selected"); %>>Siptune LM-75</option-->
                        </select>
                                </td>
                                </tr>
                                	
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,9);"><#HSDPAConfig_Country_itemname#></a></th>
                                <td>
                                	<select name="hsdpa_country" id="hsdpa_country" class="input" onchange="show_ISP_list()">
                                	<option value="AU" <% nvram_match_x("General","hsdpa_country", "AU","selected"); %>>AU</option>
                                	<option value="CA" <% nvram_match_x("General","hsdpa_country", "CA","selected"); %>>CA</option>
                                	<option value="CN" <% nvram_match_x("General","hsdpa_country", "CN","selected"); %>>CN</option>
                                	<option value="US" <% nvram_match_x("General","hsdpa_country", "US","selected"); %>>US</option>
                                	<option value="IT" <% nvram_match_x("General","hsdpa_country", "IT","selected"); %>>IT</option>
                                	<option value="PO" <% nvram_match_x("General","hsdpa_country", "PO","selected"); %>>PO</option>
                                	<option value="UK" <% nvram_match_x("General","hsdpa_country", "UK","selected"); %>>UK</option>
                                	<option value="IN" <% nvram_match_x("General","hsdpa_country", "IN","selected"); %>>IN</option>
                                	<option value="MA" <% nvram_match_x("General","hsdpa_country", "MA","selected"); %>>MA</option>
                                	<option value="SG" <% nvram_match_x("General","hsdpa_country", "SG","selected"); %>>SG</option>
                                	<option value="PH" <% nvram_match_x("General","hsdpa_country", "PH","selected"); %>>PH</option>
                                	<option value="SA" <% nvram_match_x("General","hsdpa_country", "SA","selected"); %>>SA</option>
                                	<option value="HK" <% nvram_match_x("General","hsdpa_country", "HK","selected"); %>>HK</option>
                                	<option value="TW" <% nvram_match_x("General","hsdpa_country", "TW","selected"); %>>TW</option>
                                	<option value="EG" <% nvram_match_x("General","hsdpa_country", "EG","selected"); %>>EG</option>
                                	<option value="DR" <% nvram_match_x("General","hsdpa_country", "DR","selected"); %>>DR</option>
                                	<option value="ES" <% nvram_match_x("General","hsdpa_country", "ES","selected"); %>>ES</option>
                                	<option value="BR" <% nvram_match_x("General","hsdpa_country", "BR","selected"); %>>BR</option>
                                	<option value="NE" <% nvram_match_x("General","hsdpa_country", "NE","selected"); %>>NE</option>
                                	<option value="NO" <% nvram_match_x("General","hsdpa_country", "NO","selected"); %>>NO</option>
                                	<option value="RU" <% nvram_match_x("General","hsdpa_country", "RU","selected"); %>>RU</option>
					<option value="TH" <% nvram_match_x("General","hsdpa_country", "TH","selected"); %>>TH</option>
                                	</select>
                                </td>
                                </tr>
                                
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,8);"><#HSDPAConfig_ISP_itemname#></a></th>
                                <td>
                                	<select name="hsdpa_isp" id="hsdpa_isp" class="input" onchange="show_APN_list()"></select>
                                </td>
                                </tr>

                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,3);"><#HSDPAConfig_private_apn_itemname#></a></th>
                                <td>
                                	<input id="hsdpa_apn" name="hsdpa_apn" class="input" type="text" value=""/>
                                </td>
                                </tr>

                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,2);"><#HSDPAConfig_PIN_itemname#></a></th>
                                <td>
                                	<input id="wan_3g_pin" name="wan_3g_pin" class="input" type="password" maxLength="8" value="<% nvram_get_x("", "wan_3g_pin"); %>"/>
                                </td>
                                </tr>
                                
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,10);"><#HSDPAConfig_DialNum_itemname#></a></th>
                                <td>
                                	<input id="hsdpa_dialnum" name="hsdpa_dialnum" class="input" type="text" value=""/>
                                </td>
                                </tr>
                                
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,11);"><#AiDisk_Account#></a></th>
                                <td>
                                	<input id="hsdpa_user" name="hsdpa_user" class="input" type="text" value="<% nvram_get_x("", "hsdpa_user"); %>"/>
                                </td>
                                </tr>
                                
                                <tr>
                                <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(21,12);"><#AiDisk_Password#></a></th>
                                <td>
                                	<input id="hsdpa_pass" name="hsdpa_pass" class="input" type="password" value="<% nvram_get_x("", "hsdpa_pass"); %>"/>
                                </td>
                                </tr>
                                <tr align="right">
                                        <td colspan="2">
                                                <input type="button" class="button" value="<#CTL_apply#>" onclick="applyRule();">
                                        </td>
                                </tr>
                        </table>
                </td>
        </tr>
</table>                

                </td>
</form>

                                        <!--==============Beginning of hint content=============-->
                                        <td id="help_td" style="width:15px;" valign="top">
                                                <form name="hint_form"></form>
                                                <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
                                                <div id="hintofPM" style="display:none;">
                                                        <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
                                                                <thead>
                                                                <tr>
                                                                        <td>
                                                                                <div id="helpname" class="AiHintTitle"></div>
                                                                                <a href="javascript:closeHint();">
                                                                                        <img src="images/button-close.gif" class="closebutton">
                                                                                </a>
                                                                        </td>
                                                                </tr>
                                                                </thead>
                                                                
                                                                <tr>
                                                                        <td valign="top">
                                                                                <div class="hint_body2" id="hint_body"></div>
                                                                                <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                                                                        </td>
                                                                </tr>
                                                        </table>
                                                </div>
                                        </td>
                                        <!--==============Ending of hint content=============-->
                                        
                                </tr>
                        </table>                                
                </td>
                
    <td width="10" align="center" valign="top">&nbsp;</td>
        </tr>
</table>

<div id="footer"></div>
</body>
</html>
