#include "zdbg.hpp"
#include <iostream>

int main()
{
	Zdbg dbg;
	DWORD pid;
	std::cout << "Enter the pid to attach to" << std::endl;
	std::cin >> pid;
	dbg.attach(pid);
	std::list<DWORD> lst = dbg.enumerate_threads();

	for (auto t : lst) {
		auto thread_context = dbg.get_thread_context(t);

		std::cout << "[*] DUMPING registers for thread id " << t << std::endl;
		std::cout << "[**] RIP:  " << std::hex << thread_context.Rip << std::endl;
		std::cout << "[**] RSP:  " << std::hex << thread_context.Rsp << std::endl;
		std::cout << "[**] RBP:  " << std::hex << thread_context.Rbp << std::endl;
		std::cout << "[**] RAX:  " << std::hex << thread_context.Rax << std::endl;
		std::cout << "[**] RBX:  " << std::hex << thread_context.Rbx << std::endl;
		std::cout << "[**] RCX:  " << std::hex << thread_context.Rcx << std::endl;
		std::cout << "[**] RDX:  " << std::hex << thread_context.Rdx << std::endl;
		std::cout << "[**] RDI:  " << std::hex << thread_context.Rdi << std::endl;
		std::cout << "[**] R8:   " << std::hex << thread_context.R8  << std::endl;
		std::cout << "[**] R9:   " << std::hex << thread_context.R9  << std::endl;
		std::cout << "[**] R10:  " << std::hex << thread_context.R10 << std::endl;
		std::cout << "[**] R11:  " << std::hex << thread_context.R11 << std::endl;
		std::cout << "[**] R12:  " << std::hex << thread_context.R12 << std::endl;
		std::cout << "[**] R13:  " << std::hex << thread_context.R13 << std::endl;
		std::cout << "[**] R14:  " << std::hex << thread_context.R14 << std::endl;
		std::cout << "[**] R15:  " << std::hex << thread_context.R15 << std::endl;
		std::cout << "[*] END DUMP---------------------" << std::endl;
	}
	dbg.detach();
	return 0;
}