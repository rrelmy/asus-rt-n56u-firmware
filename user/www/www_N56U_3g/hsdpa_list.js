function show_3G_modem_list(){
	modemlist = new Array(
			"AUTO"
			, "ASUS-T500"
			, "BandLuxe-C120"
			, "BandLuxe-C170"
			//, "Option-GlobeSurfer-Icon"
			, "Option-GlobeSurfer-Icon-7.2"
			, "Option-GlobeTrotter-GT-MAX-3.6"
			, "Option-GlobeTrotter-GT-MAX-7.2"
			, "Option-GlobeTrotter-EXPRESS-7.2"
			, "Option-iCON-210"
			, "Option-GlobeTrotter-HSUPA-Modem"
			, "Option-iCON-401"
			, "Vodafone-K3760"
			, "ATT-USBConnect-Quicksilver"
			, "Huawei-E1550"
			, "HUAWEI-E160G"
			, "Huawei-E161"
			, "HUAWEI-E169"
			, "Huawei-E176"
			, "Huawei-E180"
			, "HUAWEI-E220"
			, "Huawei-K3520"
			//, "MU-Q101"
			, "OPTION-ICON225"
			, "Alcatel-X200"
			/*, "Huawei-E630"
			, "Huawei-E270"
			, "Huawei-E1612"
			, "Huawei-E1690"
			, "Huawei-K3765"
			, "Huawei-K4505"
			, "ZTE-MF620"
			, "ZTE-MF622"
			, "ZTE-MF626"
			, "ZTE-MF628"
			, "ZTE-AC8710"
			, "ZTE-AC2710"
			, "ZTE-6535-Z"
			, "ZTE-K3520-Z"
			, "ZTE-MF110"
			, "ZTE-K3565"
			, "ONDA-MT503HS"
			, "ONDA-MT505UP"
			, "Novatel-Wireless-Ovation-MC950D-HSUPA"
			, "Novatel-U727"
			, "Novatel-MC990D"
			, "Novatel-U760"
			, "Alcatel-X020"
			, "AnyDATA-ADU-500A"
			, "Solomon-S3Gm-660"
			, "C-motechD-50"
			, "C-motech-CGU-628"
			, "Toshiba-G450"
			, "UTStarcom-UM175"
			, "Hummer-DTM5731"
			, "A-Link-3GU"
			, "Sierra-Wireless-Compass-597"
			, "Sierra-881U"
			, "Sony-Ericsson-W910i"
			, "Sony-Ericsson-MD400"
			, "LG-LDU-1900D"
			, "Samsung-SGH-Z810"
			, "MobiData-MBD-200HU"
			, "ST-Mobile"
			, "MyWave-SW006"
			, "Cricket-A600"
			, "EpiValley-SEC-7089"
			, "Samsung-U209"
			, "D-Link-DWM-162-U5"
			, "Novatel-MC760"
			, "Philips-TalkTalk"
			, "HuaXing-E600"
			, "C-motech-CHU-629S"
			, "Sagem-9520"
			, "Nokia-CS-15"
			, "Vodafone-MD950"
			, "Siptune-LM-75"//*/
			);

	free_options($("shown_modems"));
	for(var i = 0; i < modemlist.length; i++){
		$("shown_modems").options[i] = new Option(modemlist[i], modemlist[i]);
		if(modemlist[i] == modem)
			$("shown_modems").options[i].selected = "1";
	}
}

function show_3G_country_list(){
	countrylist = new Array();
	countrylist[0] = new Array("Australia", "AU");
	countrylist[1] = new Array("Bulgaria", "BUL");
	countrylist[2] = new Array("Brazil", "BZ");
	countrylist[3] = new Array("Canada", "CA");
	countrylist[4] = new Array("China", "CN");
	countrylist[5] = new Array("Dominican Republic", "DR");
	countrylist[6] = new Array("Denmark", "DK");
	countrylist[7] = new Array("Germany", "DE");
	countrylist[8] = new Array("Egypt", "EG");
	countrylist[9] = new Array("El Salvador", "SV");
	countrylist[10] = new Array("Finland", "FI");
	countrylist[11] = new Array("Hong Kong", "HK");
	countrylist[12] = new Array("Indonesia", "IN");
	countrylist[13] = new Array("India", "INDI");
	countrylist[14] = new Array("Italy", "IT");
	countrylist[15] = new Array("Japan", "JP");
	countrylist[16] = new Array("Malaysia", "MA");
	countrylist[17] = new Array("Netherland", "NE");
	countrylist[18] = new Array("Norway", "NO");
	countrylist[19] = new Array("New Zealand", "NZ");
	countrylist[20] = new Array("Portugal", "PO");
	countrylist[21] = new Array("Poland", "POL");
	countrylist[22] = new Array("Philippine", "PH");
	countrylist[23] = new Array("Romania", "RO");
	countrylist[24] = new Array("Russia", "RU");
	countrylist[25] = new Array("Singapore", "SG");
	countrylist[26] = new Array("South Africa", "SA");
	countrylist[27] = new Array("Spain", "ES");
	countrylist[28] = new Array("Sweden", "SE");
	countrylist[29] = new Array("Thiland", "TH");
	countrylist[30] = new Array("Taiwan", "TW");
	countrylist[31] = new Array("Ukraine", "UA");
	countrylist[32] = new Array("UK", "UK");
	countrylist[33] = new Array("USA", "US");

	free_options($("isp_countrys"));
	for(var i = 0; i < countrylist.length; i++){
		$("isp_countrys").options[i] = new Option(countrylist[i][0], countrylist[i][1]);
		if(countrylist[i][1] == country)
			$("isp_countrys").options[i].selected = "1";
	}
}

function gen_3G_list(){
	country = $("isp_countrys").value;

	if(country == "AU"){
		isplist = new Array("Telstra", "Optus", "Bigpond", "Hutchison 3G", "Vodafone", "iburst", "Dodo", "Exetel", "Internode", "Three", "Three PrePaid", "TPG", "Virgin", "A1", "3", "Orange", "T-Mobile Austria", "YESSS!");
		apnlist = new Array("telstra.internet", "Internet", "telstra.bigpond", "3netaccess", "vfprepaymbb", "internet", "DODOLNS1", "exetel1", "splns333a1", "3netaccess", "3services","internet", "VirginBroadband", "A1.net", "drei.at", "web.one.at", "gprsinternet", "web.yesss.at")
		daillist = new Array("*99#", "*99#","*99#", "*99***1#", "*99#", "*99***1#", "*99#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#")
		userlist = new Array("", "", "", "", "", "", "", "", "", "", "", "", "", "ppp@a1plus.at", "", "web", "GPRS", "");
		passlist = new Array("", "", "", "", "", "", "", "", "", "", "", "", "", "ppp", "", "web", "", "");
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
		daillist = new Array("*99***1#", "*99#", "*99***1#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "US"){
		isplist = new Array("Telecom Italia Mobile", "Bell Mobility" ,"Cellular One" ,"Cincinnati Bell" ,"T-Mobile (T-Zone)" ,"T-Mobile (Internet)" ,"Verizon", "AT&T", "Sprint");
		apnlist = new Array("proxy", "", "cellular1wap", "wap.gocbw.com", "wap.voicestream.com", "internet2.voicestream.com", "", "broadband", "");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "#777", "*99***1#", "#777");
		userlist = new Array("", "", "", "", "", "", "", "", "");
		passlist = new Array("", "", "", "", "", "", "", "", "");
	}
	else if(country == "IT"){
		isplist = new Array("OTelecom Italia Mobile", "Vodafone", "TIM", "Wind", "Tre");
		apnlist = new Array("", "web.omnitel.it", "ibox.tim.it", "internet.wind", "tre.it");
		daillist = new Array("", "", "*99#", "*99#", "*99#");
		userlist = new Array("", "", "", "", "");
		passlist = new Array("", "", "", "", "");
	}
	else if(country == "PO"){
		isplist = new Array("Optimus", "TMN", "Optimus");
		apnlist = new Array("", "internet", "myconnection");
		daillist = new Array("", "*99#", "*99#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "UK"){
		isplist = new Array("internetvpn", "O2", "Vodavone", "Orange");
		apnlist = new Array("orangenet", "", "internet", "internetvpn");
		daillist = new Array("*99#", "", "*99#", "*99#");
		userlist = new Array("", "", "web", "");
		passlist = new Array("", "", "web", "");
	}
	else if(country == "DE"){
		isplist = new Array("Vodafone", "T-mobile", "E-Plus", "o2 Germany", "vistream", "EDEKAmobil", "Tchibo", "mp3mobile", "Ring", "BILDmobil", "Alice", "Congstar", "klarmobil", "callmobile", "REWE", "simply", "Tangens", "ja!mobil", "penny", "Milleni.com", "PAYBACK", "smobil", "BASE", "Blau", "MEDION-Mobile", "simyo", "uboot", "vybemobile", "Aldi Talk", "o2", "o2 Prepaid", "Fonic", "igge&ko", "PTTmobile", "solomo", "sunsim", "telesim");
		apnlist = new Array("web.vodafone.de", "internet.t-mobile", "internet.eplus.de", "internet", "internet.vistream.net", "data.access.de", "webmobil1", "gprs.gtcom.de", "internet.ring.de", "access.vodafone.de", "internet.partner1", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "internet.t-mobile", "web.vodafone.de", "web.vodafone.de", "web.vodafone.de", "internet.eplus.de", "internet.eplus.de", "internet.eplus.de", "internet.eplus.de", "internet.eplus.de", "internet.eplus.de", "internet.eplus.de", "internet", "pinternet.interkom.de", "pinternet.interkom.de", "internet.vistream.net", "internet.vistream.net", "internet.vistream.net", "internet.vistream.net", "internet.vistream.net");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("vodafone", "tm", "eplus", "", "WAP", "", "", "", "", "", "", "tm", "tm", "tm", "tm", "tm", "tm", "tm", "tm", "vodafone", "vodafone", "vodafone", "eplus", "eplus", "eplus", "eplus", "eplus", "eplus", "eplus", "", "", "", "WAP", "WAP", "WAP", "WAP", "WAP");
		passlist = new Array("nternet", "tm", "gprs", "", "Vistream", "", "", "", "", "", "", "tm", "tm", "tm", "tm", "tm", "tm", "tm", "tm", "internet", "internet", "internet", "gprs", "gprs", "gprs", "gprs", "gprs", "gprs", "gprs", "", "", "", "Vistream", "Vistream", "Vistream", "Vistream", "Vistream");
	}
	else if(country == "IN"){
		isplist = new Array("IM2", "INDOSAT", "XL", "Telkomsel Flash", "3");
		apnlist = new Array("indosatm2", "indosat3g", "www.xlgprs.net", "flash", "3gprs");
		daillist = new Array("*99#", "*99#", "*99#", "*99#", "*99***1#");
		userlist = new Array("", "indosat", "xlgprs", "", "3gprs");
		passlist = new Array("", "indosat", "proxl", "", "3gprs");
	}
	else if(country == "MA"){
		isplist = new Array("Celcom", "Maxis", "Digi");
		apnlist = new Array("celcom3g", "unet", "3gdgnet");
		daillist = new Array("*99***1#", "*99***1#", "*99#");
		userlist = new Array("", "maxis", "");
		passlist = new Array("", "wap", "");
	}
	else if(country == "SG"){
		isplist = new Array("M1", "Singtel", "StarHub", "Power Grid");
		apnlist = new Array("sunsurf", "e-ideas", "shinternet", "");
		daillist = new Array("*99#", "*99***1#", "*99#", "*99***1#");
		userlist = new Array("65", "", "", "");
		passlist = new Array("user123", "", "", "");
	}
	else if(country == "PH"){
		isplist = new Array("Globe", "Smart", "Sun Cellula");
		apnlist = new Array("internet.globe.com.ph", "internet", "minternet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "SA"){
		isplist = new Array("Vodacom", "MTN", "Cell-c");
		apnlist = new Array("internet", "internet", "internet");
		daillist = new Array("*99#", "*99#", "*99#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "HK"){
		isplist = new Array("SmarTone-Vodafone", "3 Hong Kong", "One2Free", "PCCW mobile", "All 3G ISP support", "New World", "3HK", "CSL", "People", "Sunday");
		apnlist = new Array("internet", "mobile.three.com.hk", "internet", "pccw", "", "ineternet", "ipc.three.com.hk", "internet", "internet", "internet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99#", "*99***1#", "*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("", "", "", "", "", "", "", "", "");
		passlist = new Array("", "", "", "", "", "", "", "", "");
	}
	else if(country == "TW"){
		isplist = new Array("Far Eastern", "Far Eastern(fetims)", "Chunghua Telecom", "Taiwan Mobile", "Vibo", "Taiwan Cellular");
		apnlist = new Array("internet", "fetims", "internet", "internet", "internet", "internet");
		daillist = new Array("*99#", "*99***1#", "*99***1#","*99#" ,"*99#" ,"*99***1#");
		userlist = new Array("", "", "", "", "", "");
		passlist = new Array("", "", "", "", "", "");
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
	else if(country == "SV"){
		isplist = new Array("Telmex");
		apnlist = new Array("internet.ideasclaro");
		daillist = new Array("*99#");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "NE"){
		isplist = new Array("T-Mobile", "KPN", "Telfort", "Vodafone");
		apnlist = new Array("internet", "internet", "internet", "internet");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("", "", "", "");
		passlist = new Array("", "", "", "");
	}
	else if(country == "RU"){
		isplist = new Array("BeeLine", "Megafon", "MTS", "PrimTel");
		apnlist = new Array("internet.beeline.ru", "internet.nw", "internet.mts.ru", "internet.primtel.ru");
		daillist = new Array("*99***1#", "*99***1#", "*99***1#", "*99***1#");
		userlist = new Array("beeline", "", "", "");
		passlist = new Array("beeline", "", "", "");
	}
  else if(country == "UA"){
    isplist = new Array("BeeLine", "Kyivstar Contract", "Kyivstar Prepaid", "Kyivstar XL", "Kyivstart 3G", "Djuice", "MTS", "Utel");
    apnlist = new Array("internet.beeline.ua", "www.kyivstar.net", "www.ab.kyivstar.net", "xl.kyivstar.net", "3g.kyivstar.net", "www.djuice.com.ua", "", "3g.utel.ua");
    daillist = new Array("*99#", "*99#", "*99#", "*99#", "*99#", "*99#", "#777", "*99#");
    userlist = new Array("", "", "", "", "", "", "mobile", "");
    passlist = new Array("", "", "", "", "", "", "internet", "");
  }
	else if(country == "TH"){
		isplist = new Array("TOT", "TH GSM");
		apnlist = new Array("internet", "internet");
		daillist = new Array("", "*99#");
		userlist = new Array("", "");
		passlist = new Array("", "");
	}
  else if(country == "POL"){
  	isplist = new Array("Play", "Cyfrowy Polsat", "ERA", "Orange", "Plus", "Heyah", "Aster");
    apnlist = new Array("internet", "multi.internet", "internet", "internet", "internet", "internet", "aster.internet");
    daillist = new Array("*99#", "*99***1#", "*99***1#", "*99#", "*99***1#", "*99***1#", "*99#");
    userlist = new Array("", "", "erainternet", "internet", "plusgsm", "heyah", "internet");
    passlist = new Array("", "", "erainternet", "internet", "plusgsm", "heyah", "internet");
  }
	else if(country == "INDI"){
		isplist = new Array("Reliance", "Tata", "MTS", "Airtel", "Idea", "MTNL");
		apnlist = new Array("reliance", "TATA", "", "airtelgprs.com", "Internet", "gprsppsmum");
		daillist = new Array("#777", "#777", "#777", "*99#", "*99***1#", "*99#");
		userlist = new Array("", "internet", "internet@internet.mtsindia.in", "", "", "mtnl");
		passlist = new Array("", "internet", "mts", "", "", "mtnl123");
	}
  else if(country == "BZ"){
    isplist = new Array("Vivo", "Tim", "Oi", "Claro");
    apnlist = new Array("zap.vivo.com.br", "tim.br", "gprs.oi.com.br", "bandalarga.claro.com.br");
    daillist = new Array("*99#", "*99#", "*99***1#", "*99***1#");
    userlist = new Array("vivo", "tim", "oi", "claro");
    passlist = new Array("vivo", "tim", "oi", "claro");
  }
  else if(country == "RO"){
    isplist = new Array("Vodafone", "Orange", "Cosmote", "RCS-RDS");
    apnlist = new Array("internet.vodafone.ro", "internet", "broadband", "internet");
    daillist = new Array("*99#", "*99#", "*99#", "*99#");
    userlist = new Array("internet.vodafone.ro", "", "", "");
    passlist = new Array("vodafone", "", "", "");
  }
  else if(country == "BUL"){
    isplist = new Array("Globul", "Mtel", "Vivacom");
    apnlist = new Array("internet.globul.bg", "inet-gprs.mtel.bg", "internet.vivatel.bg");
    daillist = new Array("359891000", "359881000", "359871000");
    userlist = new Array("globul", "", "vivatel");
    passlist = new Array("", "", "vivatel");
  }
	else if(country == "NZ"){
		isplist = new Array("CHT", "Vodafone NZ", "2 Degrees");
		apnlist = new Array("internet", "www.vodafone.net.nz", "2degrees");
		daillist = new Array("*99#", "*99#", "*99#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "FI"){
		isplist = new Array("Sonera", "Elisa", "Saunalahti", "DNA");
		apnlist = new Array("internet", "internet", "internet.saunalahti", "internet");
		daillist = new Array("*99#", "*99#", "*99#", "*99#");
		userlist = new Array("", "", "", "");
		passlist = new Array("", "", "", "");
	}
	else if(country == "SE"){
		isplist = new Array("3", "Telia", "Telenor", "Tele2");
		apnlist = new Array("data.tre.se", "online.telia.se", "internet.telenor.se", "internet.tele2.se");
		daillist = new Array("*99#", "*99#", "*99#", "*99#");
		userlist = new Array("", "", "", "");
		passlist = new Array("", "", "", "");
	}
	else if(country == "NO"){
		isplist = new Array("Tele2", "Telenor", "Netcom");
		apnlist = new Array("internet.tele2.se", "Telenor", "internet.netcom.no");
		daillist = new Array("*99#", "*99#", "*99#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "DK"){
		isplist = new Array("3", "TDC", "Orange");
		apnlist = new Array("data.tre.dk", "internet", "web.orange.dk");
		daillist = new Array("*99#", "*99#", "*99#");
		userlist = new Array("", "", "");
		passlist = new Array("", "", "");
	}
	else if(country == "ES"){
		isplist = new Array("Vodafone", "Movistar", "Simyo", "Ono");
		apnlist = new Array("airtelnet.es", "movistar.es", "gprs-service.com", "internet.ono.com");
		daillist = new Array("*99#", "*99#", "*99#", "*99#");
		userlist = new Array("vodafone", "movistar", "", "");
		passlist = new Array("vodafone", "movistar", "", "");
	}
	else if(country == "JP"){
		isplist = new Array("Softbank", "b-mobile", "AU");
		apnlist = new Array("emb.ne.jp", "dm.jplat.net", "au.NET");
		daillist = new Array("*99***1#", "*99***1#", "*99**24#");
		userlist = new Array("em", "bmobile@u300", "au@au-win.ne.jp");
		passlist = new Array("em", "bmobile", "au");
	}
	else{
		isplist = new Array("Not Support yet");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
}
