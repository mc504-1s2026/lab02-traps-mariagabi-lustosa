#include <kernel/trap.h>
#include <kernel/panic.h>
#include <kernel/serial.h>
#include <arch/csr.h>
#include <arch/timer.h>

extern void trap_entry();

void handle_irq(u64 scause)
{
	if (scause == TRAP_TIMER_IRQ) {
		timer_irq();
	} else if (scause == TRAP_EXTERNAL_IRQ) {
		serial_irq();
	}
}

void handle_exception()
{
	u64 scause = csr_read(CSR_SCAUSE);
	u64 stval = csr_read(CSR_STVAL);
	u64 sepc = csr_read(CSR_SEPC);

	if (scause == EXCEPTION_INST_ACCESS_FAULT) {
		error("instruction access fault at %lx (stval %lx)\n", sepc, stval);
	} else if (scause == EXCEPTION_LOAD_ACCESS_FAULT) {
		error("load access fault at %lx (stval %lx)\n", sepc, stval);
	} else if (scause == EXCEPTION_STORE_ACCESS_FAULT) {
		error("store access fault at %lx (stval %lx)\n", sepc, stval);
	} else if (scause == EXCEPTION_INST_PAGE_FAULT) {
		error("instruction page fault at %lx (stval %lx)\n", sepc, stval);
	} else if (scause == EXCEPTION_LOAD_PAGE_FAULT) {
		error("load page fault at %lx (stval %lx)\n", sepc, stval);
	} else if (scause == EXCEPTION_STORE_PAGE_FAULT) {
		error("store page fault at %lx (stval %lx)\n", sepc, stval);
	} else {
		error("unhandled exception %lx at %lx (stval %lx)\n", scause, sepc, stval);
	}
}

void trap_setup()
{
	csr_write(CSR_STVEC, (u64)trap_entry);
	hart_irq_disable();
}

void handle_trap()
{
	u64 scause = csr_read(CSR_SCAUSE);

	if (scause & TRAP_IRQ_BIT) {
		handle_irq(scause);
	} else {
		handle_exception();
	}
}

void hart_irq_enable()
{
	csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

u64 hart_irq_save()
{
	return csr_read_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

void hart_irq_restore(u64 hart_irq_ret)
{
	if (hart_irq_ret & CSR_SSTATUS_SIE) {
		hart_irq_enable();
	}
}

void hart_irq_disable()
{
	csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);

}
