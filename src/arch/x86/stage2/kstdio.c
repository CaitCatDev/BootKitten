#include <types.h>
#include <stdarg.h>

static void (*PUTCH_FN)(uint8_t ch);

void kstdio_init(void (*PRINTCH)(uint8_t ch)) {
	PUTCH_FN = PRINTCH;
}

void put(uint8_t ch) {
	if(!PUTCH_FN) {
		return;
	}

	PUTCH_FN(ch);
}

void puts(const char *str) {	
	for(; *str; str++) {
		put(*str);
	}
}

void putsn(const char *str, uint64_t n) {
	for (uint64_t i = 0; i < n; ++i) {
		put(str[i]);
	}
}

void puth(uint64_t value, uint16_t nibbles) {
	static char hexchars[] = "0123456789abcdef";
	char buffer[20] = { 0 };
	if(nibbles >= (sizeof(uint64_t) * 2)) {
		nibbles = (sizeof(uint64_t) * 2);
	}

	while(nibbles) {
		buffer[nibbles - 1] = hexchars[value % 16];
		value /= 16;
		nibbles--;
	}
	puts(buffer);
}

void putd(uint64_t value) {
	uint64_t count = 0;
	uint64_t tmp = value;
	char buffer[21] = { 0 };

	if(value == 0) {
		put('0');
		return;
	}

	while(tmp) {
		count++;
		tmp /= 10;
	}

	while(count) {
		buffer[count - 1] = (value % 10) + 0x30;
		value /= 10;
		count--;
	}

	puts(buffer);
}

void vkprintf(const char *fmt, va_list args) {
	uint32_t arg32 = 0;
	uint64_t arg64 = 0;
	reg_t argreg = 0;
	const char *str = 0;
	while(*fmt) {
		switch(*fmt) {
			case '%': {
				fmt++;
				switch(*fmt) {
					case 'l':
						fmt++;
						switch(*fmt) {
							case 'x':
								arg64 = va_arg(args, uint64_t);
								puth(arg64, sizeof(uint64_t) * 2);
								fmt++;
								break;
							case 'd':
								arg64 = va_arg(args, uint64_t);
								putd(arg64);
								fmt++;
								break;
							default:
								put('?');
								fmt++;
						}
						break;
					case 'r':
					case 'p':
						puts("0x");
						argreg = va_arg(args, reg_t);
						puth(argreg, sizeof(reg_t)*2);
						fmt++;
						break;
					case 'x':
						arg32 = va_arg(args, uint32_t);
						puth(arg32, sizeof(uint32_t) * 2);
						fmt++;
						break;
					case 'd':
						arg32 = va_arg(args, uint32_t);
						putd(arg32);
						fmt++;
						break;
					case 's':
						str = va_arg(args, const char *);
						puts(str);
						fmt++;
						break;
					case '%':
						put('%');
						fmt++;
						break;
					default:
						put('?');
						fmt++;
				}
				break;
			}
			default:
				put(*fmt);
				fmt++;
		}
	}
}

void kprintf(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vkprintf(fmt, args);
	va_end(args);
}
