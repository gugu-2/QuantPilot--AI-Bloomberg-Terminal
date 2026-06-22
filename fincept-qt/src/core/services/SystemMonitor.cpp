#include "SystemMonitor.h"

#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace fincept::core::services {

SystemMonitor& SystemMonitor::instance() {
    static SystemMonitor instance;
    return instance;
}

SystemMonitor::SystemMonitor(QObject* parent) : QObject(parent) {
#ifdef Q_OS_WIN
    prev_sys_idle_ = new FILETIME{0, 0};
    prev_sys_kernel_ = new FILETIME{0, 0};
    prev_sys_user_ = new FILETIME{0, 0};
    prev_proc_time_ = new FILETIME{0, 0};
    prev_sys_time_ = new FILETIME{0, 0};
#endif

    connect(&timer_, &QTimer::timeout, this, &SystemMonitor::update_stats);
    timer_.start(1000); // Update every second
}

SystemMonitor::~SystemMonitor() {
#ifdef Q_OS_WIN
    delete static_cast<FILETIME*>(prev_sys_idle_);
    delete static_cast<FILETIME*>(prev_sys_kernel_);
    delete static_cast<FILETIME*>(prev_sys_user_);
    delete static_cast<FILETIME*>(prev_proc_time_);
    delete static_cast<FILETIME*>(prev_sys_time_);
#endif
}

void SystemMonitor::update_stats() {
#ifdef Q_OS_WIN
    HANDLE hProcess = GetCurrentProcess();
    
    // Memory and Threads
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        stats_.memory_used_bytes = pmc.PrivateUsage;
    }
    
    // Quick estimation for threads in the current process (rough, actually needs Toolhelp32)
    // For simplicity, we just fallback to QThread::idealThreadCount() or similar if not using snapshot.
    // To keep it light, we won't query thread count via snapshot every second.
    stats_.thread_count = -1; // Placeholder
    
    // CPU Usage
    FILETIME ftime, fsys, fuser;
    FILETIME idleTime, kernelTime, userTime;
    
    if (GetSystemTimeAsFileTime(&ftime) &&
        GetSystemTimes(&idleTime, &kernelTime, &userTime) &&
        GetProcessTimes(hProcess, &ftime, &ftime, &fsys, &fuser)) {
        
        ULARGE_INTEGER sys, user, last_sys, last_user;
        sys.LowPart = fsys.dwLowDateTime;
        sys.HighPart = fsys.dwHighDateTime;
        user.LowPart = fuser.dwLowDateTime;
        user.HighPart = fuser.dwHighDateTime;
        
        ULARGE_INTEGER now, last_time;
        now.LowPart = ftime.dwLowDateTime;
        now.HighPart = ftime.dwHighDateTime;
        
        if (!first_run_) {
            auto* p_sys = static_cast<FILETIME*>(prev_sys_time_);
            auto* p_proc = static_cast<FILETIME*>(prev_proc_time_);
            
            last_sys.LowPart = p_sys->dwLowDateTime;
            last_sys.HighPart = p_sys->dwHighDateTime;
            
            last_time.LowPart = p_proc->dwLowDateTime;
            last_time.HighPart = p_proc->dwHighDateTime;
            
            ULONGLONG sys_diff = (sys.QuadPart + user.QuadPart) - last_sys.QuadPart;
            ULONGLONG time_diff = now.QuadPart - last_time.QuadPart;
            
            if (time_diff > 0) {
                // Number of processors
                SYSTEM_INFO sysInfo;
                GetSystemInfo(&sysInfo);
                stats_.cpu_usage_percent = (sys_diff * 100.0) / time_diff;
                stats_.cpu_usage_percent /= sysInfo.dwNumberOfProcessors;
            }
        }
        
        first_run_ = false;
        
        auto* p_sys = static_cast<FILETIME*>(prev_sys_time_);
        p_sys->dwLowDateTime = sys.LowPart;
        p_sys->dwHighDateTime = sys.HighPart;
        
        auto* p_proc = static_cast<FILETIME*>(prev_proc_time_);
        p_proc->dwLowDateTime = now.LowPart;
        p_proc->dwHighDateTime = now.HighPart;
    }
#else
    // Fallback for non-Windows (dummy data or zero for now to ensure compilation)
    stats_.cpu_usage_percent = 0.0;
    stats_.memory_used_bytes = 0;
    stats_.thread_count = 0;
#endif

    emit stats_updated(stats_);
}

} // namespace fincept::core::services
