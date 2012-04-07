<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
"http://www.w3.org/TR/html4/loose.dtd">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>device-map/clients.asp</title>
<style type="text/css"></style>
<link href="/form_style.css" rel="stylesheet" type="text/css" />
<link href="/NM_style.css" rel="stylesheet" type="text/css" />


<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/alttxt.js"></script>
<script>
// for client_function.js
<% login_state_hook(); %>

var list_of_BlockedClient = [<% get_nvram_list("FirewallConfig", "MFList"); %>];

var list_of_ClientPriority = [<% get_nvram_list("PrinterStatus", "x_USRRuleList"); %>];
var list_of_changedPriority = new Array();

var leases = [<% dhcp_leases(); %>];	// [[hostname, MAC, ip, lefttime], ...]
var arps = [<% get_arp_table(); %>];		// [[ip, x, x, MAC, x, type], ...]
var arls = [<% get_arl_table(); %>];		// [[MAC, port, x, x], ...]
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var ipmonitor = [<% get_static_client(); %>];	// [[IP, MAC, DeviceName, Type, http, printer, iTune], ...]
var networkmap_fullscan = '<% nvram_match_x("", "networkmap_fullscan", "0", "done"); %>'; //2008.07.24 Add.  0 stands for complete, (null) stands for scanning.

var clients = getclients(1); //_noMonitor
var unblocked_clients = new Array();
var blocked_clients = new Array();
var rule_array;
var page_modified = 0;

function initial(){
	flash_button();
	parent.show_client_status(clients.length);
	isFullscanDone();
	
	// organize the clients
	set_client_is_blocked();
	show_clients();
	parent.hideLoading();		
}

function isFullscanDone(){	
	if(networkmap_fullscan == "done"){
		$("LoadingBar").style.display = "none";
		$("refresh_list").disabled = false;
	}
	else{
		$("LoadingBar").style.display = "block";
		if(clients.length < 20)
			setTimeout("location.href='clients.asp';",5000);
		else
			setTimeout("location.href='clients.asp';",clients.length*500);
		$("refresh_list").disabled = true;
	}
}

function get_client_priority(client_ip){
	for(var i = 0; i < list_of_ClientPriority.length; ++i){
		if(list_of_ClientPriority[i][0] == "client_priority"&&
				list_of_ClientPriority[i][1] == client_ip)
			return list_of_ClientPriority[i][4];
	}
	
	return 4;
}

function get_priority_list_order(client_ip){
	for(var i = 0; i < list_of_ClientPriority.length; ++i){
		if(list_of_ClientPriority[i][0] == "client_priority"&&
				list_of_ClientPriority[i][1] == client_ip)
			return i;
	}
	
	return -1;
}

function get_changed_priority(obj){
	var start = "client_priority_".length;
	var end = obj.id.length;
	var client_order = parseInt(obj.id.substring(start, end));
	var changed_priority = obj.value;
	var len = 0;	
	
	/*if(changed_priority == 4)
		document.form.hwnat_suggest.value = 0;
	else
		document.form.hwnat_suggest.value = 1;*/

	for(var i = 0; i < unblocked_clients.length; ++i){
		if(unblocked_clients[i] == client_order){ 
			if(clients[i][10] != changed_priority){
				if(checkDuplicateName(clients[i][1], list_of_changedPriority)){
					for(var j = 0; j < list_of_changedPriority.length; ++j){
						if(list_of_changedPriority[j][0] == clients[i][1]){
							list_of_changedPriority[j][1] = changed_priority;
							
							return;
						}
					}
				}
				else{
					len = list_of_changedPriority.length;
					
					list_of_changedPriority[len] = new Array(3);
					list_of_changedPriority[len][0] = clients[i][1];	// client's ip
					list_of_changedPriority[len][1] = changed_priority;	// new priority
					
					return;
				}
			}
			else{
				if(checkDuplicateName(clients[i][1], list_of_changedPriority)){
					for(var j = 0; j < list_of_changedPriority.length; ++j){
						if(list_of_changedPriority[j][0] == clients[i][1]){
							list_of_changedPriority.splice(j, 1);
							return;
						}
					}
				}				
				return;
			}
		}
	}
}

var unblocked_clients, blocked_clients = new Array;
function set_client_is_blocked(){
	if(list_type == '1'){  // when MAC filter is in "Accept mode".
		for(var i = 0; i < clients.length; ++i){
			if(checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][9] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][9] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "block";
	}
	else if(list_type == '0'){                  // when MAC filter is disabled.  2010.12 jerry5 modified.
		for(var i = 0; i < clients.length; ++i){
			clients[i][9] = "u";
			unblocked_clients[i] = i;
		}
		$("alert_block").style.display = "none";
	}
	else{	// when MAC filter is in "Reject mode".  2010.12 jerry5 modified.
		for(var i = 0; i < clients.length; ++i){
			if(!checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][9] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][9] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "none";
	}
}

function simplyName(orig_name, index){
	if(orig_name != null){
		if(orig_name.length > 17)
			shown_client_name = orig_name.substring(0, 15) + "...";
		else
			shown_client_name = orig_name;
	}
	else
		shown_client_name = clients[index][2];
		
	return;
}

var DEVICE_TYPE = ["", "<#Device_type_01_PC#>", "<#Device_type_02_RT#>", "<#Device_type_03_AP#>", "<#Device_type_04_NAS#>", "<#Device_type_05_IC#>", "<#Device_type_06_OD#>"];
var shown_client_name = "";

function show_clients(){

	var addClient, clientType, clientName, clientIP, clientPriority, clientBlock;		
	
	for(var j=0, i=0, k=0; j < clients.length; j++){
		if(clients[j][9] == "u"){
			
		  addClient = $('Clients_table').insertRow(k+2);
		  clientType = addClient.insertCell(0);
	  	clientName = addClient.insertCell(1);
		  clientIP = addClient.insertCell(2);
		  clientPriority = addClient.insertCell(3);
			simplyName(clients[j][0], j);
		  	  	  	  
		  clients[j][5] = (clients[j][5] == undefined)?6:clients[j][5];
		  	 	  
		  var isPrt = "";
			switch(clients[j][7]){
				case "1":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(LPR)";
					break;
				case "2":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(RAW)";
					break;
				case "3":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(SMB)";
					break;
				default:
					;
		  };
		  
		  var isITu = (clients[j][8] == "1")?"<br/><strong><#Device_service_iTune#> </strong>YES":"";
		  var isWL = (clients[j][3] == "10")?"<br/><strong><#Device_service_Wireless#> </strong>YES":"";

		  clientType.style.textAlign = "center";	  
		  clientType.innerHTML = "<img title='"+ DEVICE_TYPE[clients[j][5]]+"' src='/images/wl_device/" + clients[j][5] +".gif'>";
		  clientName.innerHTML = shown_client_name;
		  clients[j][0] = (clients[j][0] == null)?"":clients[j][0];
		  clientName.abbr = clients[j][0] + 
		  									"<br/><strong>MAC: </strong>"+ clients[j][2] + isPrt + isITu + isWL;
	
		  clientName.onmouseover = function(){		  		
		  		writetxt(this.abbr);
		  };
		  clientName.onmouseout = function(){ writetxt(0); };
		  
	  	clientIP.innerHTML = (clients[j][6] == "1")?"<a href=http://"+ clients[j][1] +" target='blank'>" + clients[j][1] + "</a>":""+ clients[j][1];
		  
		  clients[j][10] = get_client_priority(clients[j][1]);
		  
	   	var selected_one = ["","",""];
			if(clients[j][10] == 1)
				selected_one[0] = "selected";
			else if(clients[j][10] == 4)
				selected_one[1] = "selected";
			else if(clients[j][10] == 6)
				selected_one[2] = "selected";
			else
				selected_one[1] = "selected";   
	
			if(clients[j][1].length > 0){
		  	clientPriority.innerHTML = "<select id='client_priority_"+j+"' class='input' onchange='get_changed_priority(this);'><option value='1' "+selected_one[0]+"><#Priority_Level_1#></option><option value='4' "+selected_one[1]+"><#Priority_Level_2#></option><option value='6'  "+selected_one[2]+"><#Priority_Level_3#></option></select>";
			}
			else{
				clientPriority.innerHTML = "<select disabled=disabled class='input'><option value=1><#btn_Disabled#></option></select>";
			}
		  
		  if(list_type != "1"){
			  clientBlock = addClient.insertCell(4);
			  clientBlock.style.textAlign = "center";
			  clientBlock.innerHTML = '<img src="/images/icon-01.gif" id=unblock_client'+j+' onClick="blockClient('+j+')" style="cursor:pointer;">\n'
		  }
		  k++; //for insertRow();
		}
		else if(clients[j][9] == "b"){  //show blocked device..
			simplyName(clients[j][0], j);
				
			add_xClient = $('xClients_table').insertRow(i+2); //here use i for create row
			xClientType = add_xClient.insertCell(0);
			xClientName = add_xClient.insertCell(1);
			xClientIP = add_xClient.insertCell(2);
			xClientMAC = add_xClient.insertCell(3);

		  var isPrt = (clients[j][7] == "1")?"<br/><strong><#Device_service_Printer#> </strong>YES":"";
		  var isITu = (clients[j][8] == "1")?"<br/><strong><#Device_service_iTune#> </strong>YES":"";
		  var isWL = (clients[j][3] == "10")?"<br/><strong><#Device_service_Wireless#> </strong>YES":"";
		  		  	  
			clients[j][5] = (clients[j][5] == undefined)?6:clients[j][5];
		  clients[j][0] = (clients[j][0] == null)?"":clients[j][0];
		  	
			xClientType.style.textAlign = "center";
		  xClientType.innerHTML = "<img title='" +DEVICE_TYPE[clients[j][5]]+"' src='/images/wl_device/" + clients[j][5] +".gif'>";
		  xClientName.innerHTML = shown_client_name;
		  xClientName.abbr = clients[j][0] + isPrt + isITu + isWL;
		  									
			xClientIP.innerHTML = clients[j][1];
			xClientMAC.innerHTML = clients[j][2];	  									
	
		  xClientName.onmouseover = function(){	  		
		  		writetxt(this.abbr);
		  };
		  xClientName.onmouseout = function(){ writetxt(0); };
			
			if(list_type != "1"){
				xClientunBlock = add_xClient.insertCell(4);
				xClientunBlock.style.textAlign = "center";
				xClientunBlock.innerHTML = '<img src="/images/icon-02.gif" onClick="unBlockClient('+j+')" style="cursor:pointer;">\n'
			}
			i++; //for insertRow();
		}
	}	
}


function blockClient(unBlockedClient_order){
	var str = "";
	
	if(list_type == "1"){
		alert("<#macfilter_alert_str1#>");
		return;
	}	
	this.selectedClientOrder = unBlockedClient_order;
	
	str += '<#block_Comfirm1#>" ';
	str += (clients[unBlockedClient_order][0] == null)?clients[unBlockedClient_order][2]:clients[unBlockedClient_order][0];
	str += '" ?\n';
	str += '<#block_Comfirm2#>';
	
	if(confirm(str)){
		set_filter_rule("add");
		do_block_client();
	}
}

function unBlockClient(blockedClient_order){
	var str = "";
	
	if(list_type == "1"){
		alert("<#macfilter_alert_str1#>");
		return;
	}
	
	this.selectedClientOrder = blockedClient_order;
	
	str += '<#unblock_Comfirm1#>" ';	
	str += (clients[blockedClient_order][0] == null)?clients[blockedClient_order][2]:clients[blockedClient_order][0];
	str += ' "<#unblock_Comfirm2#>';
	
	if(confirm(str)){
		set_filter_rule("del");
		do_unblock_client();
	}
}

function do_block_client(){
	parent.showLoading();
	if(!checkDuplicateName(clients[this.selectedClientOrder][2], list_of_BlockedClient)){
		document.macfilterForm.action_mode.value = " Add ";
		document.macfilterForm.macfilter_list_x_0.value = clients[this.selectedClientOrder][2];
	}
	else{
		document.macfilterForm.target = "";
		document.macfilterForm.action_mode.value = " Restart ";
		document.macfilterForm.macfilter_list_x_0.disabled = true;
	}
	document.macfilterForm.submit();
}

function do_unblock_client(){
	parent.showLoading();
	document.macfilterForm.action_mode.value = " Del ";
	
	document.macfilterForm.submit();
}

function set_filter_rule(action){
	if(action == "add")
		;
	else if(action == "del"){
		for(var i = 0; i < list_of_BlockedClient.length; ++i){
			if(list_of_BlockedClient[i] == clients[this.selectedClientOrder][2]){
				free_options($("MFList_s"));
				add_option($("MFList_s"), null, i, 1);
			}
		}
	}
}

function done_validating(action, group_id){
	$("applyFrame").src = "";
	page_modified = 1;
	if(group_id == "MFList"){
		if(action == " Add ")
			refreshpage();
		else if(action == " Del ")
			refreshpage();
	}
	else if(group_id == "x_USRRuleList"){
		if(action == " Del "){
			submit_add_qosrule();
		}
		else if(action == " Add "){
			if(list_of_changedPriority.length > 0)
				submit_add_qosrule();
			else
				submit_apply_qosrule();
		}
		else if(action == " Restart "){
			if(document.macfilterForm.modified.value == "1")
				setTimeout("build_submitrule();", 3000);	// wait to finish restarting QoS.
			else
				//parent.refreshpage();
				refreshpage();
		}
	}
}

function submit_macfilter(){
	document.macfilterForm.target = "";
	document.macfilterForm.action_mode.value = " Restart ";
	document.macfilterForm.next_page.value = location.pathname;
	document.macfilterForm.submit();
}

function submit_apply_qosrule(){
	document.qosForm.action_mode.value = " Restart ";
	document.qosForm.action_script.value = "goonsetting";
	document.qosForm.submit();
}

function submit_add_qosrule(){
	document.qosForm.action_mode.value = " Add ";
	document.qosForm.action_script.value = "";
	
	document.qosForm.qos_service_name_x_0.value = "client_priority";
	document.qosForm.qos_ip_x_0.value = list_of_changedPriority[0][0];
	document.qosForm.qos_port_x_0.value = "";
	document.qosForm.qos_prio_x_0.value = list_of_changedPriority[0][1];
	
	list_of_changedPriority.shift();
	
	document.qosForm.submit();
}

function submit_del_qosrule(){
	var beDeletedOrder;
	
	for(var i = 0; i < list_of_changedPriority.length; ++i){
		beDeletedOrder = get_priority_list_order(list_of_changedPriority[i][0]);
		if(beDeletedOrder == -1)
			continue;
		
		add_option($("x_USRRuleList_s"), null, beDeletedOrder, 1);
	}
	
	document.qosForm.action_mode.value = " Del ";
	document.qosForm.action_script.value = "";
	document.qosForm.submit();
}

function build_submitrule(){
	if(list_of_changedPriority.length > 0)
		submit_del_qosrule()

;


	else if(document.macfilterForm.modified.value == "1"){
		if(list_type != "1"){
			submit_macfilter();
		}
		else
			refreshpage();
	}
	else
		refreshpage();
}

function applyRule(){
	/*if(document.form.hwnat_suggest.value != "0" && document.form.hwnat.value == "1" && sw_mode == "1")
		alert("<#BasicConfig_HWNAT_alert#>");*/
	parent.showLoading();
	build_submitrule();
}

function showLoading(restart_time2){
	parent.showLoading(restart_time2);
}

function networkmap_update(s){
	document.form.action_mode.value = "";
	document.form.action_script.value = s;
	document.form.flag.value = "nodetect";
	document.form.submit();
}
</script>
</head>

<body class="statusbody" >

<iframe name="applyFrame" id="applyFrame" src="" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<div id="LoadingBar" class="popup_bar_bg" ><#Device_Searching#>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="loadingBarBlock" class="loadingBarBlock" align="center">
</div>

<form method="post" name="macfilterForm" id="macfilterForm" action="/start_apply.htm" target="applyFrame">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;General;PrinterStatus;">
<input type="hidden" name="group_id" value="MFList">
<input type="hidden" name="current_page" value="/device-map/clients.asp">
<input type="hidden" name="next_page" value="/device-map/clients.asp">
<input type="hidden" name="modified" value="<% nvram_get_x("", "page_modified"); %>">
<!-- for enable rule in MACfilter -->
<input type="hidden" name="macfilter_enable_x" value="2">
<!-- for add rule in MACfilter -->
<input type="hidden" name="macfilter_list_x_0" value="">
<!-- for del rule in MACfilter -->
<select name="MFList_s" id="MFList_s" multiple="true" style="visibility:hidden; width:0px; height:0px;"></select>
</form>

<div id="unBlockedClients_table"></div>

<table id="Clients_table" width="95%" align="center" cellpadding="1"  class="table1px">
<tr>
    <td colspan="5" class="Tablehead"><#ConnectedClient#></td>
</tr>
</table>

<div id="navtxt" class="navtext" style="position:absolute; top:50px; left:-100px; visibility:hidden; font-family:Arial, Verdana"></div>
<br />

<div id="blockedClients_table"></div>
<table id="xClients_table" width="95%" align="center" class="table1px">
<tr>
    <td colspan="5" class="Tablehead"><#BlockedClient#></td>
</tr>
</table>

<br />


<input type="button" id="applyClient" class="button" onclick="applyRule();" value="<#CTL_apply#>" style="float:right; clear:right;">
<input type="button" id="refresh_list" class="button" onclick="networkmap_update('networkmap_refresh');" value="<#CTL_refresh#>" style="float:right;">

<p><div id="alert_block" class="DMhint" style="margin-top:40px;">
 	<a href="/Advanced_MACFilter_Content.asp" target="_parent"><#menu5_5_3#></a><#macfilter_alert_str1#>
</div></p>


<form method="post" name="qosForm" id="qosForm" action="/start_apply.htm" target="applyFrame">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="sid_list" value="PrinterStatus;">
<input type="hidden" name="group_id" value="x_USRRuleList">
<input type="hidden" name="current_page" value="/">
<input type="hidden" name="next_page" value="/device-map/clients.asp">

<input type="hidden" name="qos_service_name_x_0" value="">
<input type="hidden" name="qos_ip_x_0" value="">
<input type="hidden" name="qos_port_x_0" value="">
<input type="hidden" name="qos_prio_x_0" value="">

<select name="x_USRRuleList_s" id="x_USRRuleList_s" multiple="true" style="visibility:hidden; width:0px; height:0px;"></select>	
</form>

<form method="post" name="form" id="refreshForm" action="/start_apply.htm" target="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="current_page" value="/device-map/clients.asp">
<input type="hidden" name="next_page" value="/device-map/clients.asp">
<input type="hidden" name="flag" value="">
<input type="hidden" name="hwnat_suggest" value="">
<input type="hidden" name="hwnat" value="<% nvram_get_x("PrinterStatus","hwnat"); %>">
</form>

<script>
	// 0: disable, 1: Accept, 2: Reject.
	var list_type = '<% nvram_get_x("FirewallConfig", "macfilter_enable_x"); %>';
	//var list_type = '0';
	var list_of_BlockedClient = [<% get_nvram_list("FirewallConfig", "MFList"); %>];
	
	addClientTitle = $('Clients_table').insertRow(1);
	addClientTitle.insertCell(0).innerHTML = "<#Type#>"
  addClientTitle.insertCell(1).innerHTML = "<#Computer_Name#>";
	addClientTitle.insertCell(2).innerHTML = "<#LAN_IP#>";
	addClientTitle.insertCell(3).innerHTML = "<#Priority#>";
	
  if(list_type != "1"){
	  addClientTitle.insertCell(4).innerHTML = "<#Block#>";
	}
		  
  addClientTitle.style.textAlign = "center";
<!------------------>

	addblockedClientTitle = $('xClients_table').insertRow(1);
	addblockedClientTitle.style.textAlign = "center";
	addblockedClientTitle.insertCell(0).innerHTML = "<#Type#>";
	addblockedClientTitle.insertCell(1).innerHTML = "<#Computer_Name#>";
	addblockedClientTitle.insertCell(2).innerHTML = "<#LAN_IP#>";
	addblockedClientTitle.insertCell(3).innerHTML = "<#MAC_Address#>";
	
	if(list_type != "1")
		addblockedClientTitle.insertCell(4).innerHTML = "<#unBlock#>";
		
	if(list_of_BlockedClient.length == 0){
		var Nodata = $('xClients_table').insertRow(2).insertCell(0);
		Nodata.innerHTML = "<#Nodata#>";
		Nodata.colSpan = "5";
		Nodata.style.color = "#C00";
		Nodata.style.textAlign = "center";
	}
</script>

<script>
	initial();
	
	function loading() {        
    $("loadingBarBlock").style.display = "none";
  }
  if (window.attachEvent) {
    window.attachEvent('onload', loading);
  } else {
    window.addEventListener('load', loading, false);
  }
</script>
</body>
</html>
