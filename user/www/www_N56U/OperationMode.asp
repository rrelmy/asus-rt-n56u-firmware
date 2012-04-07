<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_1_title#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';


function initial(){
	show_banner(1);
	show_menu(2, 0, 0);
	show_footer();

	load_body();
}

function load_body(){
	
	$('Tab_'+ sw_mode).className = "b1";
	
	if(sw_mode == '1'){
		$("Senario").style.background = "url(/images/sw_mode_rt.gif) no-repeat";
		$('Tab_1').style.background = "url(images/tab_rt1.gif) no-repeat 0 bottom";
		$("radio2").style.visibility = "hidden";
		$("Internet").style.display = "none";
		$("ap-line").style.display = "none";
		$("AP").innerHTML = "<#Internet#>";
		$("mode_btn").style.marginLeft = "223px";
		openHint(22, 1);
	}	
	else if(sw_mode == '2'){
		$("Senario").style.background = "url(/images/sw_mode_re.gif) no-repeat";
		$('Tab_2').style.background = "url(images/tab_re1.gif) no-repeat 0 bottom";
		$("radio2").style.visibility = "visible";
		$("Internet").style.display = "block";
		$("ap-line").style.display = "none";
		$("AP").innerHTML = "<#Device_type_03_AP#>";
		$("mode_btn").style.marginLeft = "243px";
		openHint(22, 2);
		
	}
	else if(sw_mode == '3'){
		$("Senario").style.background = "url(/images/sw_mode_re.gif) no-repeat";
		$('Tab_3').style.background = "url(images/tab_ap1.gif) no-repeat 0 bottom";
		$("radio2").style.visibility = "hidden";
		$("Internet").style.display = "block";
		$("ap-line").style.display = "block";
		$("AP").innerHTML = "<#Device_type_02_RT#>";
		$("mode_btn").style.marginLeft = "263px";
		openHint(22, 3);
	}	
}

var lastClicked = "";
var lastClickedMode = "";

function changeSena(obj, mode){
	
	if(sw_mode == "1")
		cm = "rt";
	else if(sw_mode == "2")
		cm = "re";
	else
		cm = "ap";
	
	if(lastClicked == ""){
		$('Tab_'+ sw_mode).className = "b2";  // Set the className of default tab to "unclicked"
		$('Tab_'+ sw_mode).style.background = "url(images/tab_"+cm+"0.gif) 0 bottom no-repeat";
	}
	
	if(obj.className == "b2"){
		$("Senario").style.background = (mode == "rt")?"url(/images/sw_mode_" + mode + ".gif) no-repeat":"url(/images/sw_mode_re.gif) no-repeat";
	}
	obj.style.background = "url(images/tab_"+ mode +"1.gif) 0 bottom no-repeat";
	
	if(mode == "rt"){
		openHint(22, 1);
		$("radio2").style.visibility = "hidden";
		$("Internet").style.display = "none";
		$("ap-line").style.display = "none";
		$("AP").innerHTML = "<#Internet#>";
		$("mode_btn").style.marginLeft = "223px";
	}
	else if(mode == "re"){
		openHint(22, 2);
		$("radio2").style.visibility = "visible";
		$("Internet").style.display = "block";
		$("ap-line").style.display = "none";
		$("AP").innerHTML = "<#Device_type_03_AP#>";
		$("mode_btn").style.marginLeft = "243px";
	}
	else{
		openHint(22, 3);
		$("radio2").style.visibility = "hidden";
		$("Internet").style.display = "block";
		$("ap-line").style.display = "block";
		$("AP").innerHTML = "<#Device_type_02_RT#>";
		$("mode_btn").style.marginLeft = "263px";
	}
	
	obj.className = "b1";
	
	if(lastClicked.id == obj.id)
		obj.className = "b1";	
	else if(lastClicked)
		lastClicked.className = "b2";

	if(lastClicked.id == obj.id)
		obj.style.background = "url(images/tab_"+ mode +"1.gif) 0 bottom no-repeat";		
	else if(lastClickedMode)
		lastClicked.style.background = "url(images/tab_"+ lastClickedMode +"0.gif) 0 bottom no-repeat";
	
	lastClicked = obj;
	lastClickedMode = mode;
}

function show_SwitchHint(){
		$("switch_hint").style.display = ($("switch_hint").style.display == "block")?"none":"block";
		$("mode_btn").style.display = ($("mode_btn").style.display == "block")?"none":"block";
}

</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form method="post" name="mediaserverForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="sid_list" value="Storage;">
<input type="hidden" name="current_page" value="/upnp.asp">
<input type="hidden" name="apps_dms" value="">
<input type="hidden" name="flag" value="">
</form>

<form name="form">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>

<table border="0" align="center" cellpadding="0" cellspacing="0" class="content">
	<tr>
		<td width="23">&nbsp;</td>

		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>		

		<td height="430" valign="top">
			<div id="tabMenu" style="display:none;"></div>
			<div style="background-color:#547fb6; width:100%; padding-left:3px;">
				<table border="0" cellspacing="0" cellpadding="0" height="30">
					<tr>
						<td height="30" id="Tab_1" abbr="rt" class="b2" onclick="changeSena(this, 'rt');" style="background:url(images/tab_rt0.gif) 0 bottom no-repeat">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Router</td>
						<td height="30" id="Tab_2" abbr="re" class="b2" onclick="changeSena(this, 're');" style="background:url(images/tab_re0.gif) 0 bottom no-repeat">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Repeater</td>
						<td height="30" id="Tab_3" abbr="ap" class="b2" onclick="changeSena(this, 'ap');" style="background:url(images/tab_ap0.gif) 0 bottom no-repeat">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;AP</td>
					</tr>
				</table>
			</div>
			<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  			<tr>
					<td align="left" valign="top" width="550">
						<div id="Senario">
							<span style="margin:180px 0px 0px 180px;"><#Web_Title#></span>
							<span id="AP" style="margin:160px 0px 0px 365px;"><#Device_type_03_AP#></span>
							<span id="Internet" style="margin:100px 0px 0px 450px;"><#Internet#></span>
							<span style="margin:260px 0px 0px 40px;"><#Wireless_Clients#></span>
							<span style="margin:260px 0px 0px 360px;"><#Wired_Clients#></span>
							<!--object id="radio1" style="position:absolute; margin:150px 0px 0px 100px;" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0" width="85" height="85">
                              <param name="movie" value="/images/radio1.gif" />
							  <param name="wmode" value="transparent">
                              <param name="quality" value="high" />
                              <embed src="/images/radio1.gif" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer" type="application/x-shockwave-flash" width="85" height="85" wmode="transparent"></embed>
					      </object>
						  <object id="radio2" style="position:absolute; margin:90px 0px 0px 240px; background:transparent;" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,29,0" width="85" height="85">
                              <param name="movie" value="/images/radio2.gif" />
							  <param name="wmode" value="transparent">
                              <param name="quality" value="high" />
                              <embed src="/images/radio2.gif" quality="high" pluginspage="http://www.macromedia.com/go/getflashplayer" type="application/x-shockwave-flash" width="85" height="85" wmode="transparent"></embed>
						  </object-->
							<div id="ap-line" style="margin:105px 0px 0px 223px;width:133px; height:41px; position:absolute; background:url(images/sw_mode_ap_line.gif) no-repeat; display:none;"></div>
							<div id="switch_hint"></div>
							<div id="mode_btn"></div>
						</div>
						<!--EMBED id="Senario" src="/images/re.png" width=450 height=280 type=application/x-shockwave-flash allowscriptaccess="never" wmode="transparent" style="margin-left:45px;"></EMBED>
						<EMBED  src="/images/re.png" width=450 height=280 type=application/x-shockwave-flash allowscriptaccess="never" wmode="transparent" style="margin-left:45px;"></EMBED>
						<EMBED  src="/images/ap.png" width=450 height=280 type=application/x-shockwave-flash allowscriptaccess="never" wmode="transparent" style="margin-left:45px;"></EMBED-->
					</td>
					<td id="help_td" style="width:15px;" valign="top">
						<form name="hint_form"></form>
						<div id="helpicon"></div>
						<div id="hintofPM" style="display:none; margin-top:8px;">
							<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
								<thead>
								<tr>
									<td>
										<div id="helpname" class="AiHintTitle"></div>
									</td>
								</tr>
								</thead>
								<tr>
									<td valign="top">
										<div class="hint_body2" id="hint_body"></div>
										<div class="hint_body2"><#OP_switch_button_hint1#><span onclick="show_SwitchHint();" style="text-decoration:underline; color:blue; cursor:pointer;"><#OP_switch_button_hint2#></span></div>
										<iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
									</td>
								</tr>
							</table>
						</div>
					</td>
				</tr>
			</table>
		</td>
		<td width="15" align="center" valign="top"></td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>
