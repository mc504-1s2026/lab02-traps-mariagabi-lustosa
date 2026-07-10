#include <arch/timer.h>
#include <kernel/panic.h>
#include <kernel/serial.h>
#include <arch/csr.h>

static u64 uptime = 0;
static bool alarm_pending = false;
static u64 alarm_at = 0;

u64 timer_read()
{
	return uptime;
}

void timer_irq_enable()
{
	csr_write(CSR_STIMECMP, csr_read(CSR_TIME) + TIMER_FREQ);
	csr_set(CSR_SIE, CSR_SIE_STIE);
}

void timer_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
	u64 now = timer_read();
	alarm_pending = true;
	alarm_at = now + secs;
	csr_write(CSR_STIMECMP, csr_read(CSR_TIME) + TIMER_FREQ);
}

void timer_irq()
{
	uptime++;
	if (alarm_pending && uptime >= alarm_at) {
		alarm_pending = false;
		serial_puts("alarm\n");
	}
	csr_write(CSR_STIMECMP, csr_read(CSR_TIME) + TIMER_FREQ);
}
