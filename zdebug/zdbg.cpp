#include "zdbg.hpp"
#include <iostream>

Zdbg::Zdbg()
{
    h_process = nullptr;
    pid = 0;
    zdbg_active = false;
}

void Zdbg::load(LPCSTR path)
{
    startupinfo.dwFlags = 0x1;
    startupinfo.wShowWindow = 0x0;
    startupinfo.cb = sizeof(STARTUPINFO);

    if (CreateProcessA(path,
                       nullptr,
                       nullptr,
                       nullptr,
                       0,
                       0x00000001,
                       nullptr,
                       nullptr,
                       &startupinfo,
                       &processinfo)) {
        std::cout << "[*] Launched a process" << std::endl;
        std::cout << "[*] PID " << processinfo.dwProcessId << std::endl;
		h_process = process_open(processinfo.dwProcessId);
    } else {
        std::cout << "[-] Error : " << GetLastError() << std::endl;
    }
}

HANDLE Zdbg::process_open(DWORD id)
{
	HANDLE res = OpenProcess(PROCESS_ALL_ACCESS, false, id);
	return res;
}

HANDLE Zdbg::thread_open(DWORD id)
{
	HANDLE res = OpenThread(THREAD_ALL_ACCESS, false, id);
	if (res != nullptr) {
		return res;
	}
	else {
		std::cerr << "Could not obtain valid thread handle" << std::endl;
		return nullptr;
	}
}

std::list<DWORD> Zdbg::enumerate_threads()
{
	THREADENTRY32 thread_entry;
	std::list<DWORD> thread_list = {};
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);

	if (snapshot != nullptr) {
		thread_entry.dwSize = sizeof(THREADENTRY32);
		BOOL success = Thread32First(snapshot, &thread_entry);

		while (success) {
			if (thread_entry.th32OwnerProcessID == pid) {
				thread_list.push_back(thread_entry.th32ThreadID);
			}
			success = Thread32Next(snapshot, &thread_entry);
		}
		CloseHandle(snapshot);
		return thread_list;
	}
	else {
		return std::list<DWORD>();
	}
}

CONTEXT Zdbg::get_thread_context(DWORD id)
{
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	h_thread = thread_open(id);
	if (GetThreadContext(h_thread, &context)) {
		CloseHandle(h_thread);
		return context;
	}
	else {
		return CONTEXT();
	}
}

void Zdbg::attach(DWORD id)
{
	h_process = process_open(id);

	if (DebugActiveProcess(id)) {
		zdbg_active = true;
		pid = id;
		run();
	}
	else {
		std::cout << "[*] Unable to attach process" << std::endl;
	}
}

void Zdbg::get_debug_event()
{
	DEBUG_EVENT debug_event;
	if (WaitForDebugEvent(&debug_event, INFINITE)) {
//		system("pause");
//		zdbg_active = false;
		ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE);
	}
}


void Zdbg::run()
{
	while (zdbg_active) {
		get_debug_event();
	}
}

bool Zdbg::detach()
{

	if (DebugActiveProcessStop(pid)) {
		std::cout << "Finished debugging" << std::endl;
		return true;
	}
	else {
		std::cout << "There was an error" << std::endl;
		return false;
	}
}
