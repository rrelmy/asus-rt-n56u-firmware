function show_4G_modem_list(){
	modemlist = new Array(
			"AUTO"
			, "Samsung U200"
			, "Beceem BCMS250"
			);

	free_options($("shown_modems"));
	for(var i = 0; i < modemlist.length; i++){
		$("shown_modems").options[i] = new Option(modemlist[i], modemlist[i]);
		if(modemlist[i] == modem)
			$("shown_modems").options[i].selected = "1";
	}
}

function show_4G_country_list(){
	countrylist = new Array();
	countrylist[0] = new Array("Russia", "RU");
	countrylist[1] = new Array("Taiwan", "TW");

	free_options($("isp_countrys"));
	for(var i = 0; i < countrylist.length; i++){
		$("isp_countrys").options[i] = new Option(countrylist[i][0], countrylist[i][1]);
		if(countrylist[i][1] == country)
			$("isp_countrys").options[i].selected = "1";
	}
}

function gen_4G_list(){
	country = $("isp_countrys").value;

	if(country == "TW"){
		isplist = new Array("GMC");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
	else if(country == "RU"){
		isplist = new Array("Yota");
		apnlist = new Array("");
		daillist = new Array("");
		userlist = new Array("");
		passlist = new Array("");
	}
}
