#include "get_time.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * this finction is used to get current time in a formatted way.
 * @return
 */
char *_get_time(void) {
	
	time_t rawtime;
	time (&rawtime);
	struct tm* table;

	char *time = (char *) malloc(100);
	// Get Time
	table = localtime(&rawtime);

	int day = table->tm_mday;
	int month = table->tm_mon + 1;
	int year = table->tm_year + 1900;
	int hour = table->tm_hour;
	int min = table->tm_min;
	int sec = table->tm_sec;

	sprintf(time, "%d/%d/%d %d:%d:%d", day, month, year, hour, min, sec);

	return time;
} 