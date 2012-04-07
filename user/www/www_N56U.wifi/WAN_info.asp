<?xml version="1.0" ?>
<devicemap>
	<wan><% nvram_get_x("",  "wan_status_t"); %></wan>
  <wan><% detect_if_wan(); %></wan>
  <wan><% nvram_get_x("",  "manually_disconnect_wan"); %>0</wan>
	<wan><% nvram_get_x("", "wan0_ipaddr"); %></wan>
</devicemap>
