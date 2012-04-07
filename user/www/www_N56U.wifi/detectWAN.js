// Use AJAX to detect WAN connection
var http_request = false;

function makeRequest(url) {
	http_request = new XMLHttpRequest();
	if (http_request && http_request.overrideMimeType)
		http_request.overrideMimeType('text/xml');
	else
		return false;

	http_request.onreadystatechange = alertContents;
	http_request.open('GET', url, true);
	http_request.send(null);
}

var xmlDoc_ie;

function makeRequest_ie(file)
{
	xmlDoc_ie = new ActiveXObject("Microsoft.XMLDOM");
	xmlDoc_ie.async = false;
	if (xmlDoc_ie.readyState==4)
	{
		xmlDoc_ie.load(file);
		refresh_wpsinfo(xmlDoc_ie);
	}
}

function alertContents()
{
	if (http_request != null && http_request.readyState != null && http_request.readyState == 4)
	{
		if (http_request.status != null && http_request.status == 200)
		{
			var xmldoc_mz = http_request.responseXML;
			refresh_wpsinfo(xmldoc_mz);
		}
	}
}

var msecs;
var timerID = null;
var timerRunning = false;
var timeout = 1000;
var delay = 5000;
var stopFlag = 0;

function resetTimer()
{
	if (stopFlag == 1)
	{
		stopFlag = 0;
		InitializeTimer();
	}
}

function InitializeTimer()
{
	msecs = timeout;
	StopTheClock();
	StartTheTimer();
}

function StopTheClock()
{
	if(timerRunning)
		clearTimeout(timerID);
	timerRunning = false;
}

function StartTheTimer()
{
	if (msecs==0)
	{
		StopTheClock();

    		if (stopFlag==1)
    			return;

		updateWPS();
		msecs = timeout;
		StartTheTimer();
	}
	else
	{
		//self.status = msecs;
		msecs = msecs - 1000;
		timerRunning = true;
		timerID = self.setTimeout("StartTheTimer()", delay);
	}
}

function detectWANstatus()
{
	updateWPS();
	InitializeTimer();
}

function updateWPS()
{
	var ie = window.ActiveXObject;

	if (ie)
		makeRequest_ie('/WAN_info.asp');
	else
		makeRequest('/WAN_info.asp');
}

function refresh_wpsinfo()
{	
	/*var devicemapXML = xmldoc.getElementsByTagName("devicemap");
	var wanStatus = devicemapXML[0].getElementsByTagName("wan");

	var NV_wan_status_t = wanStatus[0].firstChild.nodeValue;  //nvram: wan_status_t [Connected, Disconnected]
	var HK_detect_if_wan = wanStatus[1].firstChild.nodeValue; //hook: detect_if_wan [1,0]
	var NV_ManuallyWanStatus = wanStatus[2].firstChild.nodeValue;	//nvram: manually control the connection.
	
	if(NV_wan_status_t == "Connected"
			&& HK_detect_if_wan == "1"
			&& NV_ManuallyWanStatus != "10"){*/
	if(parent.$("internetStatus").innerHTML == "<#Connected#>"){
		$("connectstatus").innerHTML="<#Connected#>";
		$("connectbutton").value="<#Disconnect#>";
		$("connectbutton").style.display = "";
	}
	else if(parent.$("internetStatus").innerHTML == "<#Disconnected#>"){
		$("connectstatus").innerHTML="<#Disconnected#>";
		$("connectbutton").value="<#Connect#>";
		$("connectbutton").style.display = "";
	}
	else{
		$("connectstatus").innerHTML="<#QKSet_detect_freshbtn#>...";
		$("connectbutton").style.display = "none";
	}
}
