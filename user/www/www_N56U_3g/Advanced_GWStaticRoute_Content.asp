<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var GWStaticList = [<% get_nvram_list("RouterConfig", "GWStatic"); %>];

function initial(){
	show_banner(1);
	show_menu(5,2,3);
	show_footer();
	
	enable_auto_hint(6, 5);
	disable_udpxy();
	showGWStaticList();
}

function applyRule(){
	if(document.form.udpxy_enable_x.value != 0){
		if(validate_range(document.form.udpxy_enable_x, 1024, 65535)){
			showLoading();
	
			document.form.action_mode.value = " Restart ";
			document.form.current_page.value = "/Advanced_GWStaticRoute_Content.asp";
			document.form.next_page.value = "";
		document.form.submit();	
		}
	}
	else{
		showLoading();
	
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_GWStaticRoute_Content.asp";
		document.form.next_page.value = "";
		document.form.submit();	
	}
}

function done_validating(action){
	refreshpage();
}

function GWStatic_markGroup(o, s, c, b) {	
	document.form.group_id.value = s;	
	
	if(b == " Add "){
		if (document.form.sr_num_x_0.value > c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		else if (!validate_ipaddr(document.form.sr_ipaddr_x_0, "") ||
				 !validate_ipaddr(document.form.sr_netmask_x_0, "") ||
				 !validate_ipaddr(document.form.sr_gateway_x_0, "")){
				 return false;
		}
		else if (document.form.sr_ipaddr_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_ipaddr_x_0.focus();
				 return false;				 
		}
		else if (document.form.sr_netmask_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_netmask_x_0.focus();
				 return false;				 
		}
		else if (document.form.sr_gateway_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_gateway_x_0.focus();
				 return false;				 
		}				
		else if (GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_ipaddr_x_0.value, 16, 0) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_netmask_x_0.value, 16, 1) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_gateway_x_0.value, 16, 2) &&
				 GWStatic_validate_duplicate(GWStaticList, document.form.sr_if_x_0.value, 2, 4)
				) return false;  //Check the IP, Submask, gateway and Interface is duplicate or not.
	}
	
	pageChanged = 0;
	pageChangedCount = 0;
	
	document.form.action_mode.value = b;
	return true;		
}

function GWStatic_validate_duplicate_noalert(o, v, l, off){	
	for (var i=0; i<o.length; i++)
	{
		if (entry_cmp(o[i][off], v, l)==0){ 
			return true;
		}
	}
	return false;
}

function GWStatic_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++){
		if(entry_cmp(o[i][off].toLowerCase(), v.toLowerCase(), l) == 0){
			alert('<#JS_duplicate#>');
			return true;
		}
	}
	return false;
}

function showGWStaticList(){

	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="3" align="center" class="list_table">';
	if(GWStaticList.length == 0)
		code +='<tr><td style="color:#CC0000;"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < GWStaticList.length; i++){
		code +='<tr id="row'+i+'">';
		code +='<td width="">'+ GWStaticList[i][0] +'</td>';		//IP
		code +='<td width="100">'+ GWStaticList[i][1] +'</td>';		//Mask
		code +='<td width="102">'+ GWStaticList[i][2] +'&nbsp;</td>';//Gateway
		code +='<td width="50">'+ GWStaticList[i][3] +'</td>';		//Metric
		code +='<td width="30">'+ GWStaticList[i][4] +'</td>';        //Interface
		code +='<td width=\"27\"><input type=\"checkbox\" name=\"GWStatic_s\" value='+ i +' onClick="changeBgColor(this,'+i+');" id=\"check'+ i +'\"></td>';
		if(i == 0)
			code +="<td width='75' style='background:#C0DAE4;' rowspan=" + GWStaticList.length + "><input style=\"padding:2px 2px 0px 2px\" class=\"button\" type=\"submit\" onclick=\"markGroup(this, 'GWStatic', 32, ' Del ');\" value=\"<#CTL_del#>\"/></td>";
		
	    code +='</tr>';
		}
	}
	code +='<tfoot><tr align="right">';
	code +='<td colspan="8"><input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/></td>';	
	code +='</tr></tfoot>';
  	code +='</table>';
	
	$("GWStaticList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
 		$("row" + num).style.background='#FF9';
	else
 		$("row" + num).style.background='#FFF';
}

function valid_udpxy(){
	if(document.form.udpxy_enable_x.value != 0)
		validate_range(document.form.udpxy_enable_x, 1024, 65535);
}

function valid_muliticast(){
	if(document.form.controlrate_unknown_unicast.value != 0)
		validate_range(document.form.controlrate_unknown_unicast, 0, 1024);
	if(document.form.controlrate_unknown_multicast.value != 0)
		validate_range(document.form.controlrate_unknown_multicast, 0, 1024);
	if(document.form.controlrate_multicast.value != 0)
		validate_range(document.form.controlrate_multicast, 0, 1024);
	if(document.form.controlrate_broadcast.value != 0)
		validate_range(document.form.controlrate_broadcast, 0, 1024);
}

function disable_udpxy(){
	if(document.form.mr_enable_x[0].checked == 1){
//		document.form.udpxy_enable_x.disabled = 0;
		return change_common_radio(document.form.mr_enable_x, 'RouterConfig', 'mr_enable_x', '1');
	}
	else{	
//		document.form.udpxy_enable_x.disabled = 1;*/
		return change_common_radio(document.form.mr_enable_x, 'RouterConfig', 'mr_enable_x', '0');
	}	
}// The input fieldof UDP proxy does not relate to Mutlicast Routing. 
</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(6, 5);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_GWStaticRoute_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="RouterConfig;LANHostConfig;WLANConfig11b;">
<input type="hidden" name="group_id" value="GWStatic">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="sr_num_x_0" value="<% nvram_get_x("RouterConfig", "sr_num_x"); %>" readonly="1">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />
<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" >
	  <table width="550" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle" table>
		<thead>
		<tr>
		  <td><#menu5_2#> - <#menu5_2_3#></td>
		</tr>
		</thead>
		
		<tr>
		  <td bgcolor="#FFFFFF"><#RouterConfig_GWStaticEnable_sectiondesc#></td>
		</tr>
		
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF"></td>
		</tr>
		
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			  <thead>
			  <tr>
				<td colspan="2" id="GWStatic"><#RouterConfig_GWStatic_groupitemdesc#></td>
			  </tr>
			  </thead>
				
				<!-- 2008.03 James. patch for Oleg's patch. { -->
				<tr>
					<th width="50%"><#RouterConfig_GWDHCPEnable_itemname#></th>
					<td>
						<input type="radio" value="1" name="dr_enable_x" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '1')" <% nvram_match_x("RouterConfig", "dr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="dr_enable_x" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '0')" <% nvram_match_x("RouterConfig", "dr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				
				<tr>
					<th width="50%"><#RouterConfig_GWMulticastEnable_itemname#></th>
					<td>
						<input type="radio" value="1" name="mr_enable_x" class="input" onClick="disable_udpxy();" <% nvram_match_x("RouterConfig", "mr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="mr_enable_x" class="input" onClick="disable_udpxy();" <% nvram_match_x("RouterConfig", "mr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>
				
				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 7);"><#RouterConfig_GWMulticast_unknownUni_itemname#></a></th>
        	<td>
          	<input id="controlrate_unknown_unicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_unicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_unicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 7);" onblur="valid_muliticast();"/>
          </td>
				</tr>

				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 8);"><#RouterConfig_GWMulticast_unknownMul_itemname#></a></th>
        	<td>
          	<input id="controlrate_unknown_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_unknown_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_unknown_multicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 8);" onblur="valid_muliticast();"/>
          </td>
				</tr>

				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 9);"><#RouterConfig_GWMulticast_Multicast_itemname#></a></th>
        	<td>
          	<input id="controlrate_multicast" type="text" maxlength="4" class="input" size="15" name="controlrate_multicast" value="<% nvram_get_x("LANHostConfig", "controlrate_multicast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 9);" onblur="valid_muliticast();"/>
          </td>
				</tr>

				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 10);"><#RouterConfig_GWMulticast_Broadcast_itemname#></a></th>
        	<td>
          	<input id="controlrate_broadcast" type="text" maxlength="4" class="input" size="15" name="controlrate_broadcast" value="<% nvram_get_x("LANHostConfig", "controlrate_broadcast"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 10);" onblur="valid_muliticast();"/>
          </td>
				</tr>
				<!-- 2008.03 James. patch for Oleg's patch. } -->
				
				<!-- 2008.03 James. patch for Oleg's patch. { -->
				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);">2.4GHz <#WLANConfig11b_MultiRateAll_itemname#></a></th>
					<td>
						<select name="rt_mrate" class="input" onClick="openHint(3, 7);" onChange="return change_common(this, 'WLANConfig11b', 'rt_mrate')">
							<option value="0" <% nvram_match_x("WLANConfig11b", "rt_mrate", "0", "selected"); %>>Disable</option>
							<!--option value="1" <% nvram_match_x("WLANConfig11b", "rt_mrate", "1", "selected"); %>>1</option>
							<option value="2" <% nvram_match_x("WLANConfig11b", "rt_mrate", "2", "selected"); %>>2</option>
							<option value="3" <% nvram_match_x("WLANConfig11b", "rt_mrate", "3", "selected"); %>>5.5</option>
							<option value="4" <% nvram_match_x("WLANConfig11b", "rt_mrate", "4", "selected"); %>>6</option>
							<option value="5" <% nvram_match_x("WLANConfig11b", "rt_mrate", "5", "selected"); %>>9</option>
							<option value="6" <% nvram_match_x("WLANConfig11b", "rt_mrate", "6", "selected"); %>>11</option>
							<option value="7" <% nvram_match_x("WLANConfig11b", "rt_mrate", "7", "selected"); %>>12</option>
							<option value="8" <% nvram_match_x("WLANConfig11b", "rt_mrate", "8", "selected"); %>>18</option>
							<option value="9" <% nvram_match_x("WLANConfig11b", "rt_mrate", "9", "selected"); %>>24</option>
							<option value="10" <% nvram_match_x("WLANConfig11b", "rt_mrate", "10", "selected"); %>>36</option>
							<option value="11" <% nvram_match_x("WLANConfig11b", "rt_mrate", "11", "selected"); %>>48</option>
							<option value="12" <% nvram_match_x("WLANConfig11b", "rt_mrate", "12", "selected"); %>>54</option-->
							<option value="13" <% nvram_match_x("WLANConfig11b", "rt_mrate", "13", "selected"); %>>Auto</option>
						</select>
					</td>
				</tr>
				<!-- 2008.03 James. patch for Oleg's patch. } -->

				<!-- 2008.03 James. patch for Oleg's patch. { -->
				<tr>
					<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(3, 7);">5GHz <#WLANConfig11b_MultiRateAll_itemname#></a></th>
					<td>
						<select name="wl_mrate" class="input" onClick="openHint(3, 7);" onChange="return change_common(this, 'WLANConfig11b', 'wl_mrate')">
							<option value="0" <% nvram_match_x("WLANConfig11b", "wl_mrate", "0", "selected"); %>>Disable</option>
							<option value="13" <% nvram_match_x("WLANConfig11b", "wl_mrate", "13", "selected"); %>>Auto</option>
						</select>
					</td>
				</tr>
				<!-- 2008.03 James. patch for Oleg's patch. } -->

				<tr>
        	<th width="50%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6, 6);"><#RouterConfig_IPTV_itemname#>:</a></th>
        	<td>
          	<input id="udpxy_enable_x" type="text" maxlength="5" class="input" size="15" name="udpxy_enable_x" value="<% nvram_get_x("LANHostConfig", "udpxy_enable_x"); %>" onkeypress="return is_number(this);" onClick="openHint(6, 6);" onblur="valid_udpxy();"/>
          </td>
        </tr>
				
			  <tr>
			    <th width="50%"><#RouterConfig_GWStaticEnable_itemname#></th>
			    <td>
				  <input type="radio" value="1" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '1')" <% nvram_match_x("RouterConfig", "sr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
				  <input type="radio" value="0" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '0')" <% nvram_match_x("RouterConfig", "sr_enable_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			  </tr>
			</table>
			
			<table width="100%" border="1" align="center" cellpadding="3" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
   		  	<thead>
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,1);"><#RouterConfig_GWStaticIP_itemname#></a></th>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,2);"><#RouterConfig_GWStaticMask_itemname#></a></th>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,3);"><#RouterConfig_GWStaticGW_itemname#></a></th>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,4);"><#RouterConfig_GWStaticMT_itemname#></a></th>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(6,5);"><#RouterConfig_GWStaticIF_itemname#></a></th>
				<th>&nbsp;</th>
			  </tr>
			  </thead>
			  <tr>
				<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_ipaddr_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
				<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_netmask_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
				<td align="center"><input type="text" maxlength="15" class="input" size="12" name="sr_gateway_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
				<td align="center"><input type="text" maxlength="3" class="input" size="3" name="sr_matric_x_0"  onkeypress="return is_number(this)"></td>
				<td width="64" align="center">
					<select name="sr_if_x_0" class="input">
						<option value="LAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "LAN","selected", 0); %>>LAN</option>
						<!-- 2008.03 James. patch for Oleg's patch. { -->
						<option value="MAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "MAN","selected", 0); %>>MAN</option>
						<!-- 2008.03 James. patch for Oleg's patch. } -->
						<option value="WAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "WAN","selected", 0); %>>WAN</option>
					</select>
				</td>
				
				<td width="75" align="center">
					<!-- 2008.03 James. patch for Oleg's patch. { -->
					<input class="button" type="submit" onClick="return GWStatic_markGroup(this, 'GWStatic', 32, ' Add ');" name="GWStatic" value="<#CTL_add#>" size="12">
					<!-- 2008.03 James. patch for Oleg's patch. } -->
				</td>
			  </tr>
			</table>
			<div id="GWStaticList_Block"></div>
		  </td>
		</tr>
		</tbody>
	  </table>
	</td>
</form>

	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;"  valign="top">
	  <div id="helpicon" onClick="openHint(0, 0);" title="<#Help_button_default_hint#>">
		<img src="images/help.gif">
	  </div>
	  
	  <div id="hintofPM" style="display:none;">
<form name="hint_form"></form>
		<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
		  <thead>
		  <tr>
			<td>
			  <div id="helpname" class="AiHintTitle"></div>
			  <a href="javascript:closeHint();"><img src="images/button-close.gif" class="closebutton" /></a>
			</td>
		  </tr>
		  </thead>
		  
		  <tbody>
		  <tr>
			<td valign="top">
			  <div id="hint_body" class="hint_body2"></div>
			  <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
			</td>
		  </tr>
		  </tbody>
		</table>
	  </div>
	</td>
	<!--==============Ending of hint content=============-->
  </tr>
</table>				
<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
