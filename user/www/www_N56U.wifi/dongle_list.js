function gen_list(){
	var country = document.getElementById('hsdpa_country').value;
	if(country == "AU"){
		isplist = new Array("Telstra", "Optus", "Bigpond", "Hutchison 3G", "Vodafone", "iburst", "Dodo", "Exetel", "Internode", "Three", "Three PrePaid", "TPG", "Virgin");
		apnlist = new Array("telstra.internet", "Internet", "telstra.bigpond", "3netaccess", "vfinternet.au", "internet", "dodolns1", "exetel1", "splns333a1", "3netaccess", "3services","internet", "virginbroadband")
		daillist = new Array("*99#", "*99#","*99#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#")
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "CA"){
		isplist = new Array("Rogers");
		apnlist = new Array("internet.com");
		daillist = new Array("");
		userlist = new Array("wapuser1");
		passlist = new Array("wap");
	}
	else if(country == "CN"){
		isplist = new Array("China Telecom", "China Mobile", "China Unicom");
		apnlist = new Array("", "cmnet", "");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "US"){
		isplist = new Array("Telecom Italia Mobile", "Bell Mobility" ,"Cellular One" ,"Cincinnati Bell" ,"T-Mobile (T-Zone)" ,"T-Mobile (Internet)" ,"Verizon");
		apnlist = new Array("proxy", "", "cellular1wap", "wap.gocbw.com", "wap.voicestream.com", "internet2.voicestream.com", "");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "IT"){
		isplist = new Array("OTelecom Italia Mobile");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "PO"){
		isplist = new Array("Optimus");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "UK"){
		isplist = new Array("Orangenet", "Vodafone", "O2", "T-mobile", "E-Plus", "o2 Germany", "vistream", "EDEKAmobil", "Tchibo", "mp3mobile", "Ring", "BILDmobil", "Alice");
		apnlist = new Array("orangenet", "web.vodafone.de", "internet.t-mobile", "internet.eplus.de", "internet", "internet.vistream.net", "data.access.de", "webmobil1", "gprs.gtcom.de", "internet.ring.de", "access.vodafone.de", "internet.partner1");
		daillist = new Array("", "*99***1#", "", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("", "", "", "tm", "eplus", "", "WAP", "", "", "", "", "", "");
		passlist = new Array("", "", "", "tm", "gprs", "", "Vistream", "", "", "", "", "", "");
	}
	else if(country == "IN"){
		isplist = new Array("IM2", "INDOSAT", "XL", "Telkomsel Flash", "3");
		apnlist = new Array("indosatm2", "indosat3g", "www.xlgprs.net", "flash", "3gprs");
		daillist = new Array("*99#", "*99#", "*99#", "*99#", "*99***1#");
		userlist = new Array("", "indosat", "xlgprs", "", "3gprs");
		passlist = new Array("", "indosat", "proxl", "", "3gprs");
	}
	else if(country == "MA"){
		isplist = new Array("Celcom", "Maxis");
		apnlist = new Array("celcom3g", "unet");
		daillist = new Array("*99***1#", "*99***1#");
		userlist = new Array("", "maxis");
		passlist = new Array("", "wap");
	}
	else if(country == "SG"){
		isplist = new Array("M1", "Singtel", "StarHub", "Power Grid");
		apnlist = new Array("sunsurf", "e-ideas", "", "");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "PH"){
		isplist = new Array("Globe", "Smart", "Sun Cellula");
		apnlist = new Array("internet.globe.com.ph", "internet", "minternet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "SA"){
		isplist = new Array("Vodacom", "MTN", "Cell-c");
		apnlist = new Array("internet", "", "internet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "HK"){
		isplist = new Array("SmarTone-Vodafone", "3 Hong Kong", "One2Free", "PCCW mobile", "All 3G ISP support", "New World", "3HK", "CSL", "People", "Sunday");
		apnlist = new Array("internet", "mobile.three.com.hk", "internet", "pccw", "", "ineternet", "ipc.three.com.hk", "internet", "internet", "internet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "TW"){
		isplist = new Array("Far Eastern", "Far Eastern(fetims)", "Chunghua Telecom", "Taiwan Mobile", "Vibo", "Taiwan Cellular");
		apnlist = new Array("internet", "fetims", "internet", "internet", "internet", "internet");
		daillist = new Array("*99#", "*99***1#", "*99***1#","*99#" ,"*99#" ,"*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "EG"){
		isplist = new Array("Etisalat");
		apnlist = new Array("internet");
		daillist = new Array("*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "DR"){
		isplist = new Array("Telmex");
		apnlist = new Array("internet.ideasclaro.com.do");
		daillist = new Array("*99#");
		userlist = new Array("claro");
		passlist = new Array("claro");
	}
	else if(country == "ES"){
		isplist = new Array("Telmex");
		apnlist = new Array("internet.ideasclaro");
		daillist = new Array("*99#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "BR"){
		isplist = new Array("Claro");
		apnlist = new Array("bandalarga.claro.com.br");
		daillist = new Array("*99***1#");
		userlist = new Array("claro");
		passlist = new Array("claro");
	}
	else if(country == "NE"){
		isplist = new Array("T-Mobile", "KPN", "Telfort", "Vodafone");
		apnlist = new Array("internet", "internet", "internet", "internet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "NO"){
		isplist = new Array("Telenor Mobile", "Netcom Mobile");
		apnlist = new Array("Telenor", "Netcom");
		daillist = new Array("*99***1#", "*99***1#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "RU"){
		isplist = new Array("BeeLine", "Megafon", "MTS", "PrimTel");
		apnlist = new Array("internet.beeline.ru", "internet.nw", "internet.mts.ru", "internet.primtel.ru");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("beeline (Anna)", "", "", "");
		passlist = new Array("beeline (Anna)", "", "", "");
	}
	else if(country == "TH"){
		isplist = new Array("TOT");
		apnlist = new Array("internet");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
	else{
		isplist = new Array("Not Support yet");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
}
		
function show_ISP_list(){
	gen_list();
	for(var i = 0; i < isplist.length; i++){
		$('hsdpa_isp').options[i] = new Option(isplist[i],isplist[i]);
		if(isplist[i] == isp)
				$('hsdpa_isp').options[i].selected = "1";
	}
	$('hsdpa_isp').options.length = isplist.length;
	show_APN_list();
}

function show_APN_list(){
	var ISPlist = document.getElementById('hsdpa_isp').value;
	var user = '<% nvram_get_x("General", "hsdpa_user"); %>';
	var pass = '<% nvram_get_x("General", "hsdpa_pass"); %>';
	
	if($('hsdpa_isp').value == isp){
		if(apn == "" && dialnum == "" && user == "" && pass == ""){
			gen_list();
			for(var i = 0; i < isplist.length; i++){
				if(isplist[i] == ISPlist){
					$('hsdpa_apn').value = apnlist[i];
					$('hsdpa_dialnum').value = daillist[i];
					$('hsdpa_user').value = userlist[i];
					$('hsdpa_pass').value = passlist[i];
				}
			}
		}
		else{
			$('hsdpa_apn').value = '<% nvram_get_x("General", "hsdpa_apn"); %>';
			$('hsdpa_dialnum').value = '<% nvram_get_x("General", "hsdpa_dialnum"); %>';
			$('hsdpa_user').value = '<% nvram_get_x("General", "hsdpa_user"); %>';
			$('hsdpa_pass').value = '<% nvram_get_x("General", "hsdpa_pass"); %>';
		}
	}
	else{
		gen_list();
		for(var i = 0; i < isplist.length; i++){
			if(isplist[i] == ISPlist){
				$('hsdpa_apn').value = apnlist[i];
				$('hsdpa_dialnum').value = daillist[i];
				$('hsdpa_user').value = userlist[i];
				$('hsdpa_pass').value = passlist[i];
			}
		}
	}
}

