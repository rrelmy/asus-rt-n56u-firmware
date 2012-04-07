<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<script>
<% get_changed_status(); %>
<% get_printer_info(); %>
<% wanlink(); %>

var flag = '<% get_parameter("flag"); %>';

var manually_stop_wan = '<% nvram_get_x("", "manually_disconnect_wan"); %>';
var ifWANConnect = <% detect_if_wan(); %>;
var wan_status_log = "<% get_wan_status_log(); %>";
var detect_dhcp_pppoe = "<% detect_dhcp_pppoe(); %>";
var detect_wan_conn = "<% nvram_get_x("", "link_internet"); %>";
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var qos_global_enable = '<% nvram_get_x("PrinterStatus", "qos_global_enable"); %>';
var qos_userdef_enable = '<% nvram_get_x("PrinterStatus", "qos_userdef_enable"); %>';
var qos_ubw = '<% nvram_get_x("PrinterStatus", "qos_ubw"); %>';
var qos_manual_ubw = '<% nvram_get_x("PrinterStatus", "qos_manual_ubw"); %>';
var qos_ready = 1;
if((qos_global_enable == '1' || qos_userdef_enable == '1')
		&& qos_ubw == '0'
		&& (qos_manual_ubw == '0' || qos_manual_ubw == ''))
	qos_ready = 0;

function initial(){
	var status_wanlink = "";
	
	if(wanlink_statusstr() == "Connected")
		status_wanlink = "Connected";
	else
		status_wanlink = "Disconnected";
	parent.set_changed_status(manually_stop_wan,
														ifWANConnect,
														status_wanlink,
														detect_dhcp_pppoe,
														wan_status_log,
														detect_wan_conn
														);
	
	parent.check_changed_status(flag);
}
</script>
</head>

<body onload="initial();">
</body>
</html>
