
User 使用情境:
Step0 : 移掉x_Setting=0 的判斷，避免WPS_info & WAN_info & QIS 等頁面的導頁問題。(IE要認證卻跳不出來)
Step1 : x_Setting=1 而且 無人login時，送start_apply & start_apply2 & wlconn_apply & setting_lan.htm 需要認證。(解決hacker issue)
Step2 : x_Setting=1 而且 無人login時，並非以上四個apply頁面時，無需認證。(只有這四個頁面會寫入資訊)
Step3 : login_state =2 (使用者的連線沒有斷)時，不需要認證。(因為hacker送出網頁會顯示說 "已經有使用者")

Hacker 使用情境:
Step1 : 使用者login_state =2 (使用者的連線沒有斷)時，hacker送start_apply，一定會看到裡面有使用者，
				所以hacker的login_state=3 並跳出已有使用者的頁面。(這部分的code已經在前面就被處理了)
Step2 : 使用者login_state =1 & 已經設定過 (使用者已登出)，hacker送start_apply，需要認證。

================================================================================================================================
                        if((login_state == 1 || login_state == 2)
                                        //&& !nvram_match("x_Setting", "1") // user : Step0
                                        && (strstr(url, "QIS_") != NULL   
                                                        || !strcmp(url, "survey.htm") 
                                                        || !strcmp(url, "ureip.asp")  
                                                        || !strcmp(url, "Logout.asp")  
                                                        || !strcmp(url, "aplist.asp")  
                                                        || !strcmp(url, "wlconn_apply.htm")  
                                                        || !strcmp(url, "result_of_get_changed_status.asp")
                                                        || !strcmp(url, "result_of_get_changed_status_QIS.asp")
                                                        || !strcmp(url, "detectWAN.asp")
                                                        || !strcmp(url, "WPS_info.asp")
                                                        || !strcmp(url, "WAN_info.asp")
                                                        || !strcmp(url, "result_of_detect_client.asp")
                                                        || !strcmp(url, "start_apply.htm")
                                                        || !strcmp(url, "start_apply2.htm")
                                                        || !strcmp(url, "detectWAN2.asp")
                                                        || !strcmp(url, "automac.asp")
                                                        || !strcmp(url, "setting_lan.htm")
                                                        )
                                        ){

/* hacker issue patch start */
                                                if((nvram_match("x_Setting", "1")) && login_state == 1){ // user : Step1, hacker : Step2
                                                                if (!strcmp(url, "start_apply.htm") || !strcmp(url, "start_apply2.htm") || !strcmp(url, "wlconn_apply.htm")){
                                                                temp_turn_off_auth = 0; // auth
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                redirect = 0;
                                                                }
                                                                else{ // user : Step2
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                temp_turn_off_auth = 1; // no auth
                                                                redirect = 0;
                                                                }
                                                }
                                                else{ // user : Step3
                                                                turn_off_auth_timestamp = request_timestamp;
                                                                temp_turn_off_auth = 1; // no auth
                                                                redirect = 0;
                                                }
/* hacker issue patch end */

================================================================================================================================



