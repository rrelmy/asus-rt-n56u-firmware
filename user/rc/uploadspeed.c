#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <shutils.h>
#include <nvram/bcmnvram.h>

/*
 1  * * *					// case 1
 2  * * 1.1.1.1  22.33 ms			// case 2
 3  * 4.4.4.4  55.55 ms  66.66 ms		// case 3
 4  * 7.7.7.7  88.88 ms *			// case 4
 5  9.9.9.9  11.11 ms * *			// case 5
 6  2.2.2.2  33.33 ms * 44.44 ms		// case 6
 7  5.5.5.5  66.66 ms  77.77 ms *		// case 7
 8  8.8.8.8  99.99 ms  11.11 ms  22.22 ms	// case 8
 9  3.3.3.3  44.44 ms  55.55 ms			// case 9
10  6.6.6.6  77.77 ms				// case 10
11  1.1.1.1  11.11 ms *				// case 11
*/

#define DETECT_UL_SPD_FILE "/tmp/detect_ul_spd"
#define TTL_MAX 5
#define PAYLOAD 38
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

/**************************************************************************/

int count_of_ms(const char *buf)
{
	char *p; 
	char copy[256];
	int count = 0;
                                
	memcpy(copy, buf, 256);
	p = (char *) copy;
                                
	while (p = strstr(p, "ms"))
	{
		count++;
		p = p + 2;
	}

	return count;
}

int detect_upload_speed(double *upload_speed_p)
{
	FILE *fp = NULL;
	char hop_ipaddr[TTL_MAX][30];
	double delta[TTL_MAX], double_delta_total;
	char str_cmd[128], buf[256], detect_ul_spd_ip[16] = "8.8.8.8";
	char idx[8], ipaddr[16], delta1[16], delta2[16], delta3[16], ms1[16], ms2[16], ms3[16], empty1[8], empty2[8];
	char str_delta1[16], str_delta2[16], str_delta3[16], str_delta_candidate[16];
	double double_delta1, double_delta2, double_delta3, double_delta[16];
	double double_delta_candidate, double_delta_max, double_delta_min, double_delta_avg, upload_speed;
	int i, num_of_delta, valid_hop = 0;

	fprintf(stderr, "## detecting ##\n\n");

	for (i = 0; i < TTL_MAX; i++)
		memset(&hop_ipaddr[i], 0, sizeof(hop_ipaddr[i]));

	snprintf(str_cmd, sizeof(str_cmd), "traceroute -n -w 2 -m %d %s %d > %s", TTL_MAX, detect_ul_spd_ip, PAYLOAD, DETECT_UL_SPD_FILE);
	remove(DETECT_UL_SPD_FILE);

	nvram_set("no_internet_detect", "1");
	if (pids("tcpcheck"))
		system("killall -SIGTERM tcpcheck");
	if (pids("traceroute"))
		system("killall traceroute");

//	fprintf(stderr, "str_cmd: %s\n", str_cmd);
	system(str_cmd);
//	snprintf(str_cmd, sizeof(str_cmd), "cat %s", DETECT_UL_SPD_FILE);
//	system(str_cmd);
//	fprintf(stderr, "\n");

	if ((fp = fopen(DETECT_UL_SPD_FILE, "r")) != NULL) 
	{
		i = 0;

		while (i < TTL_MAX)
		{
			/* get ip */
			if ( fgets(buf, sizeof(buf), fp) != NULL ) 
			{
//				fprintf(stderr, "buf: %s\n", buf);
				if (strstr(buf, "traceroute to"))
				{
					continue;
				}

				if (strstr(buf, "* * *"))	// case 1
				{
					i++;
					continue;
				}

				if (!strstr(buf, "*"))		// case 8
				{
//					num_of_delta = 3;
					num_of_delta = count_of_ms(buf);
					sscanf(buf, "%s %s %s %s %s %s %s %s", idx, ipaddr, delta1, ms1, delta2, ms2, delta3, ms3);
				}
				else if (strstr(buf, "  * * "))// case 2
				{
					num_of_delta = 1;
					sscanf(buf, "%s %s %s %s %s %s", idx, empty1, empty2, ipaddr, delta1, ms1);
				}
				else if( strstr(buf, " * *"))	// case 5
				{
					num_of_delta = 1;
					sscanf(buf, "%s %s %s %s %s %s", idx, ipaddr, delta1, ms1, empty1, empty2);
				}
				else if	(strstr(buf, "  * "))
				{
					if (strstr(buf, "ms *"))// case 4
					{
						num_of_delta = 1;
						sscanf(buf, "%s %s %s %s %s %s", idx, empty1, ipaddr, delta1, ms1, empty2);
					}
					else			// case 3
					{
						num_of_delta = 2;
						sscanf(buf, "%s %s %s %s %s %s %s", idx, empty1, ipaddr, delta1, ms1, delta2, ms2);
					}
				}
				else if (strstr(buf, " * "))	// case 6
				{
					num_of_delta = 2;
					sscanf(buf, "%s %s %s %s %s %s %s", idx, ipaddr, delta1, ms1, empty1, delta2, ms2);
				}
				else
				{
					num_of_delta = count_of_ms(buf);
					if (num_of_delta == 2)	// case 7
						sscanf(buf, "%s %s %s %s %s %s %s", idx, ipaddr, delta1, ms1, delta2, ms2, empty1);
					else			// case 11
						sscanf(buf, "%s %s %s %s %s", idx, ipaddr, delta1, ms1, empty1);
				}

//				if (!strcmp(idx, "1"))		// skip first hop
//					continue;

//				fprintf(stderr, "%s %s %s %s %s\n", idx, ipaddr, delta1, (num_of_delta >= 2) ? delta2 : "", (num_of_delta == 3) ? delta3 : "");

				strcpy(hop_ipaddr[valid_hop], ipaddr);
				double_delta1 = atof(delta1);
				double_delta2 = atof(delta2);
				double_delta3 = atof(delta3);
				sprintf(str_delta1, "%.3f", double_delta1);
				sprintf(str_delta2, "%.3f", double_delta2);
				sprintf(str_delta3, "%.3f", double_delta3);

				if (num_of_delta == 1)
					double_delta_candidate = double_delta1;
				else if (num_of_delta == 2)
					double_delta_candidate = min(double_delta1, double_delta2);
				else
				{
					double_delta_candidate = min(double_delta1, double_delta2);
					double_delta_candidate = min(double_delta_candidate, double_delta3);
				}
				sprintf(str_delta_candidate, "%.3f", double_delta_candidate);
				delta[valid_hop] = double_delta_candidate;

//				fprintf(stderr, "%s %s %s %s %s\n", idx, ipaddr, str_delta1, (num_of_delta >= 2) ? str_delta2 : "", (num_of_delta == 3) ? str_delta3 : "");
				fprintf(stderr, "#%s. hop:%15s, min delta:%8s ms\n", idx, hop_ipaddr[valid_hop], str_delta_candidate);

				valid_hop++;
				i++;
			}
			else
				break;
		}

		fclose(fp);

		fprintf(stderr, "\nhop count: %12d\n", valid_hop);

		if (valid_hop <= 1)
			goto END_calc;

		i = 0;
		double_delta_max = 0.000;
		double_delta_min = 0.000;
		double_delta_total = 0.000;

		while (i < valid_hop)
		{
//			fprintf(stderr, "delta[%d]: %.3f ms\n", i, delta[i]);

			if (delta[i] > double_delta_max)
				double_delta_max = delta[i];

			if (i == 0)
				double_delta_min = delta[i];
			else if (delta[i] < double_delta_min)
				double_delta_min = delta[i];

//			fprintf(stderr, "delta max: %.3f ms\n", double_delta_max);
//			fprintf(stderr, "delta min: %.3f ms\n", double_delta_min);

			double_delta_total += delta[i];
//			fprintf(stderr, "tmp total delta: %.3f ms\n", double_delta_total);

			i++;
		}

		if (valid_hop > 2)
		{
			double_delta_avg = double_delta_total / valid_hop;
			if ((valid_hop > 2) && (double_delta_min * 1.166 < double_delta_avg))
			{
				fprintf(stderr, "skip delta:  %10.3f ms\n", double_delta_min);
				valid_hop--;
				double_delta_total -= double_delta_min;
			}
			if ((valid_hop > 2) && (double_delta_max > double_delta_avg * 1.166))
			{
				fprintf(stderr, "skip delta:  %10.3f ms\n", double_delta_max);
				valid_hop--;
				double_delta_total -= double_delta_max;
			}
		}

		if (valid_hop > 0)
		{
			fprintf(stderr, "total delta: %10.3f ms\n", double_delta_total);
			fprintf(stderr, "total payload: %8d bytes\n", PAYLOAD * valid_hop);

			if ((double_delta_total > 0.000) && ((PAYLOAD * valid_hop) > 0))
				upload_speed = ((PAYLOAD * valid_hop) / double_delta_total * 1000000 / 1024) * 0.900;
			else
				upload_speed = 0.000;

			fprintf(stderr, "upload speed:%10.3f kb/s (90.0%%)\n", upload_speed);
		}
		else
		{
END_calc:
			upload_speed = 0.000;

			fprintf(stderr, "upload speed:%10.3f kb/s\n", upload_speed);
		}

		logmessage("rc", "upload speed:%10.3f kb/s", upload_speed);
	}
	else
	{
		nvram_set("no_internet_detect", "0");
		*upload_speed_p = 0.000;
		return 0;
	}

	fprintf(stderr, "\n## end of detecting ##\n");

	if ((valid_hop > 0) && (double_delta_total > 0.000))
	{
		nvram_set("no_internet_detect", "0");
		*upload_speed_p = upload_speed;
		return 1;
	}
	else
	{
		nvram_set("no_internet_detect", "0");
		*upload_speed_p = 0.000;
		return 0;
	}
}

