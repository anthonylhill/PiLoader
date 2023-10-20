
// include file for Raspberry Pi GPIO handling via file handles


// Pin modes:
#define INPUT (0)
#define OUTPUT (1)
#define LOW (0)
#define HIGH (1)

typedef struct {
		int		pin;
		char*	fn;
} pin_t;

static pin_t pinopen(int pin, int mode);
static void	 pinclose(pin_t pin);
static void	 pinwrite(pin_t pin, int value);
static int	 pinread(pin_t pin);


pin_t pinopen(int pin, int mode)
{
		char*	pinfn = malloc(1024);
		char	dirfn[1024];
		FILE*	dir = NULL;
		FILE*	fp = fopen("/sys/class/gpio/export", "w");
		if ( fp == NULL )
		{ 
			printf("\n** Error : fopen failed for gpio write access on pin %d, mode %d \n", pin, mode ) ;
			exit(0) ;
		}
		fprintf(fp, "%d", pin);
		fclose(fp);
		snprintf(dirfn, 1024, "/sys/class/gpio/gpio%d/direction", pin);
		snprintf(pinfn, 1024, "/sys/class/gpio/gpio%d/value", pin);
		while (dir == NULL) {
				dir = fopen(dirfn, "w");
		}
		if (mode == INPUT) {
				fprintf(dir, "in");
		} else {
				fprintf(dir, "out");
		}
		fclose(dir);
		return (pin_t) { pin, pinfn };
}

void pinclose(pin_t pin)
{
		FILE*	fp = fopen("/sys/class/gpio/unexport", "w");
		fprintf(fp, "%d", pin.pin);
		fclose(fp);
		free(pin.fn);
}

void pinwrite(pin_t pin, int value)
{
		FILE*	fp = fopen(pin.fn, "w");
		if (value == LOW) {
				fprintf(fp, "0");
		} else {
				fprintf(fp, "1");
		}
		fclose(fp);
}

int pinread(pin_t pin)
{
		char	buf[2];
		FILE*	fp = fopen(pin.fn, "r");
		size_t	read = fread(buf, 1, 2, fp);
		fclose(fp);
		if (read != 2) {
				return -1;
		} else {
				return (buf[0] == '1') ? HIGH : LOW;
		}
}
