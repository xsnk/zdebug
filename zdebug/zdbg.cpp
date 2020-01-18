#include "zdbg.hpp"
#include <iostream>

Zdbg::Zdbg()
{
    h_process = nullptr;
	h_thread = nullptr;
    pid = 0;
    zdbg_active = false;
	g_context = CONTEXT();
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
	tid = id;
	HANDLE res = OpenThread(THREAD_ALL_ACCESS, false, tid);
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

CONTEXT Zdbg::get_thread_context(DWORD id, HANDLE t_handle = nullptr)
{
	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

	t_handle = thread_open(id);
	if (GetThreadContext(t_handle, &context)) {
		CloseHandle(t_handle);
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

auto Zdbg::breakpoint_exception_handler() 
{
	std::cout << "Inside breakpoint handler" << std::endl;
	std::cout << "Exception address " << std::hex << exception_address << std::endl;
	return DBG_CONTINUE;
}

void Zdbg::get_debug_event()
{
	DEBUG_EVENT debug_event;
	auto continue_status = DBG_CONTINUE;
	if (WaitForDebugEvent(&debug_event, INFINITE)) {
		h_thread = thread_open(debug_event.dwThreadId);
		g_context = get_thread_context(tid, h_thread);
		std::cout << " Event Code " << debug_event.dwDebugEventCode
			<< " Thread ID " << debug_event.dwThreadId << std::endl;

		if (debug_event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
			exception = debug_event.u.Exception.ExceptionRecord.ExceptionCode;
			exception_address = debug_event.u.Exception.ExceptionRecord.ExceptionAddress;

			if (exception == EXCEPTION_ACCESS_VIOLATION) {
				std::cout << "--:: EXCEPTION VIOLATION DETECTED" << std::endl;
			}
			else if (exception == EXCEPTION_BREAKPOINT) {
				continue_status = breakpoint_exception_handler();
			}
			else if (exception == EXCEPTION_GUARD_PAGE) {
				std::cout << "--:: GAURD PAGE EXCEPTION DETECTED" << std::endl;
			}
			else if (exception == EXCEPTION_SINGLE_STEP) {
				std::cout << "Single stepping!" << std::endl;
			}
		}

		ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, continue_status);
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
