#include <kernel/serial.h>
#include <kernel/panic.h>
#include <arch/io.h>
#include <kernel/mm.h>
#include <arch/csr.h>
#include <arch/plic.h>
#include <arch/spinlock.h>

#define SERIAL_BUF_SIZE 256

static char serial_buf[SERIAL_BUF_SIZE];
static size_t serial_buf_len = 0;
static struct spinlock serial_lock;

static void *base;



void serial_init()
{
	base = (void *)pa_to_va((phys_addr_t)SERIAL_BASE);
	iowrite8(SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR, base+SERIAL_FCR);
	iowrite8(SERIAL_IER_ERBFI, base+SERIAL_IER);
	spin_init(&serial_lock);
}

void serial_irq_enable()
{
	plic_irq_set_priority(IRQ_SERIAL, 1);
	plic_hart_set_threshold(0, 0);
	plic_hart_enable_irq(0, IRQ_SERIAL);
	csr_set(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq()
{
	u32 irq = plic_hart_claim_irq(0);
	if (irq == IRQ_SERIAL) {
		while (ioread8(base+SERIAL_LSR) & SERIAL_LSR_DTR) {
			char c = ioread8(base+SERIAL_RBR);

			u64 flags = spin_lock_irqsave(&serial_lock);
			if (serial_buf_len < SERIAL_BUF_SIZE) {
				serial_buf[serial_buf_len++] = c;
			}
			spin_unlock_irqrestore(&serial_lock, flags);
		}
	}
	plic_hart_complete_irq(0, irq);
}

size_t serial_read(char *buf)
{
	u64 flags = spin_lock_irqsave(&serial_lock);
	size_t n = serial_buf_len;
	for (size_t i = 0; i < n; i++) {
		buf[i] = serial_buf[i];
	}
	serial_buf_len = 0;
	spin_unlock_irqrestore(&serial_lock, flags);
	return n;
}

void serial_puts(char *str)
{
	for (size_t i = 0; str[i] != '\0'; i++) {
		serial_putc(str[i]);
	}
}

void serial_putc(char c)
{
	while (!(ioread8(base+SERIAL_LSR) & SERIAL_LSR_THRE)) {
	}
	iowrite8(c, base+SERIAL_THR);
}
