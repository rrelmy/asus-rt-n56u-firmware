<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title></title>
<link href="../NM_style.css" rel="stylesheet" type="text/css" />
<link href="../form_style.css" rel="stylesheet" type="text/css" />

<script type="text/javascript" src="/state.js"></script>
<script>
var d3g = "";
var modemOrder = parent.getSelectedModemOrder();
if(modemOrder == 1)
	d3g = '<% nvram_get_x("", "usb_path1_product"); %>';
else
	d3g = '<% nvram_get_x("", "usb_path2_product"); %>';

function initial(){
		showtext($("disk_model_name"), d3g);
		$("show_remove_button").style.display = "";
}

function goHspdaWizard(){
	parent.location.href = "/Advanced_Modem_others.asp";
}

function remove_d3g(){
	var str = "Do you really want to remove this USB dongle?";
	
	if(confirm(str)){
		parent.showLoading();
		
		document.diskForm.action = "safely_remove_disk.asp";
		setTimeout("document.diskForm.submit();", 8000);
	}
}
</script>
</head>

<body class="statusbody" onload="initial();">
<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#Modelname#>:</th>
    <td><span id="disk_model_name"></span></td>
  </tr>
</table>

<table id="mounted_item1" width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
    <th width="130px"><#GO_HSDPA_SETTING#>:</th>
    <td><input type="button" class="button" onclick="goHspdaWizard();" value="<#btn_go#>" ></td>
  </tr>
</table>

<table width="95%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="table1px">
  <tr>
		<th width="130px"><#Safelyremovedisk_title#>:</th>
    <td>
	<input id="show_remove_button" type="button" class="button" onclick="remove_d3g();" value="<#btn_remove#>" style="display:none;">
    <div id="show_removed_string" style="display:none;"><#Safelyremovedisk#></div>
    </td>
  </tr>
</table>

<div id="mounted_item2" style="padding:5px 0px 5px 25px; ">
<ul style="font-size:11px; font-family:Arial; padding:0px; margin:0px; list-style:outside; line-height:150%;">
	<li><#HSDPAConfig_hsdpa_mode_itemdesc#></li>
</ul>

</div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="disk" value="">
</form>
</body>
</html>
