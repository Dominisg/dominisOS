#include <time.h>
#include <string.h>

size_t strftime(char *s, size_t max, const char *format,
                const struct tm *tm)
{
    int written = 0;
    size_t maxrem = 0;
    while (*format != '\0') {
		maxrem = max - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            memcpy(s + written, format, amount);
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;
        if (*format == 'Y') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = (tm->tm_year + 1900) / 1000 + '0'; 
            s[written++] = ((tm->tm_year + 1900) % 1000) / 100 + '0'; 
            s[written++] = ((tm->tm_year + 1900) % 100) / 10 + '0'; 
            s[written++] = (tm->tm_year + 1900) % 10  + '0'; 
		} else if (*format == 'y') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = ((tm->tm_year + 1900) % 100) / 10 + '0'; 
            s[written++] = (tm->tm_year + 1900) % 10  + '0'; 
		} else if (*format == 'm') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = tm->tm_month / 10 + '0'; 
            s[written++] = tm->tm_month % 10 + '0'; 
		} else if (*format == 'd') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = tm->tm_mday / 10 + '0'; 
            s[written++] = tm->tm_mday % 10 + '0'; 
		} else if (*format == 'H') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = tm->tm_hour / 10 + '0'; 
            s[written++] = tm->tm_hour % 10 + '0'; 
		} else if (*format == 'M') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = tm->tm_min / 10 + '0'; 
            s[written++] = tm->tm_min % 10 + '0'; 
		} else if (*format == 'S') {
			format++;
			if (maxrem < 2) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            s[written++] = tm->tm_sec / 10 + '0'; 
            s[written++] = tm->tm_sec % 10 + '0'; 
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
            memcpy(s + written, format, len);
			written += len;
			format += len;
		}
	}
    if (maxrem < 1) {
        // TODO: Set errno to EOVERFLOW.
        return -1;
    }
    s[written++]='\0';
    return written;
}


