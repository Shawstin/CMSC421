#include <stdio.h>
#include <time.h>
#include <stdarg.h>

static FILE *s_logfile;
#define TIME_BUFFER_SIZE 26
#define TIME_FORMAT "%Y-%m-%d %H:%M:%S"



//Creates the logfile
void loginit(char *filename)
{
    char buffer[50];
    sprintf(buffer, "/tmp/%s", filename);
    s_logfile = fopen(buffer, "w+");
}

//Logs a message and is able to use variable arguments for format strings
void log_message(const char *format, ...)
{
    
    va_list ptr;
    va_start(ptr, format);

    time_t t;
    struct tm *tm_info;
    char buffer[TIME_BUFFER_SIZE];

    time(&t);
    tm_info = localtime(&t);

    strftime(buffer, TIME_BUFFER_SIZE, TIME_FORMAT, tm_info);

    fprintf(s_logfile, "%s : ", buffer);
    vfprintf(s_logfile, format, ptr);
    fprintf(s_logfile, "\n");

    fflush(s_logfile);

    va_end(ptr);
}

void log_close()
{
    fclose(s_logfile);
}



