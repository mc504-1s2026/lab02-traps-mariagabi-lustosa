#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>

extern int _hartid[];
void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();

	hart_irq_enable();

	char line[256];
	size_t line_len = 0;
	char buf[256];

	serial_puts("> ");

	while(1) {
		size_t n = serial_read(buf);
		for (size_t i = 0; i < n; i++) {
			char c = buf[i];
			serial_putc(c);

			if (c ==  '\r') {
				serial_puts("\n");
				line[line_len] = '\0';

				if (strcmp(line, "uptime") == 0) {
					printk(LOG_INFO, "%lus\n", timer_read());
				} else if (strncmp(line, "echo ", 5) == 0) {
					serial_puts(line + 5);
					serial_puts("\n");
				} else if (strncmp(line, "alarm ", 6) == 0) {
					u64 secs = strtou64(line + 6, 10);
					timer_set_alarm(secs);
					serial_puts("alarm set\n");
				}

				line_len = 0;
				serial_puts("> ");
			} else {
				if (line_len < sizeof(line) - 1) {
				line[line_len++] = c;
				}
			}
		}
	}
}