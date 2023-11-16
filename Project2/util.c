#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void* myMalloc(size_t size)
{
	void* ptr = malloc(size);
	if (ptr)
	{
		return ptr;
	}
	else
	{
		perror("Failed to allocate memory ");
		exit(-1);
	}
}

#pragma warning( disable : 4996)


char* formatString(const char* fmt, ...)
{
	int len;
	va_list myargs;
	va_start(myargs, fmt);
	len = vsnprintf("", 0, fmt, myargs);
	va_end(myargs);

	char* buff = malloc(len + 1);
	
	va_start(myargs, fmt);
	len = vsnprintf(buff, len + 1, fmt, myargs);
	va_end(myargs);
	
	return buff;
}

char* substring(const char* str, int position, int length)
{
	char* buff = malloc(length + 1);
	strncpy(buff, str + position, length);
	buff[length] = '\0';
	return buff;
}
