
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                              vsprintf.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "string.h"


char* ntos(char*s, int number);

/*
 *  为更好地理解此函数的原理，可参考 printf 的注释部分。
 */

/*======================================================================*
                                vsprintf
 *======================================================================*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;
	char	tmp[256];
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
		case 'x':
			itoa(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 's':
			break;
		case 'd':
			ntos(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		default:
			break;
		}
	}

	return (p - buf);
}


char* ntos(char*s, int number) {
	int i = 0;
	int n = number;
	if(n < 0) {
		s[i] = '-';
		i++;
		n = -n;
	}
	if(n == 0) {
		s[i] = '0';
		s[i+1] = '\0';
		return s;
	}
	int division = 1;
	int count = 0;
	while(n / division > 0) {
		division *= 10;
		count++;
	}
	division = division/10;
	int j;
	for(j = 0;j < count;j++){
		s[i] = '0' + n / division;
		n = n % division;
		division = division/10;
		i++;
	}
	s[i] = '\0';
	return s;
}

