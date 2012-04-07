toInternet = <% detect_wan_connection(); %>;

wan_subnet = '<% nvram_get_x("", "wan_subnet_t"); %>';
lan_subnet = '<% nvram_get_x("", "lan_subnet_t"); %>';
