#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <list>

class Zdbg {

public:
    Zdbg();
    void load(LPCSTR path);
	HANDLE process_open(DWORD pid);
	HANDLE thread_open(DWORD tid);

	std::list<DWORD> enumerate_threads();
	CONTEXT get_thread_context(DWORD id, HANDLE t_handle);

	void attach(DWORD pid);
	void get_debug_event();
	void run();
	bool detach();

private:
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION processinfo;

    HANDLE h_process;
	HANDLE h_thread;
    DWORD pid;
	DWORD tid;
    BOOL zdbg_active;
	CONTEXT g_context;
};
