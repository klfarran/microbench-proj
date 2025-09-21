#ifndef CONTEXT_SWITCH_H
#define CONTEXT_SWITCH_H

	unsigned long long measure_syscall_overhead();
	unsigned long long measure_syscall_pipe_overhead();
	void* ping_thread(void* arg);
	unsigned long long measure_thread_switch_overhead();
	

#endif