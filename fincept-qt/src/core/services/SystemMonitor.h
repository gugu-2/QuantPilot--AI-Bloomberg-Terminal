#pragma once

#include <QObject>
#include <QTimer>
#include <QString>

namespace fincept::core::services {

struct SystemStats {
    double cpu_usage_percent = 0.0;
    qint64 memory_used_bytes = 0;
    int thread_count = 0;
};

class SystemMonitor : public QObject {
    Q_OBJECT
public:
    static SystemMonitor& instance();

    const SystemStats& current_stats() const { return stats_; }

signals:
    void stats_updated(const SystemStats& stats);

private:
    explicit SystemMonitor(QObject* parent = nullptr);
    ~SystemMonitor() override;

    void update_stats();

    QTimer timer_;
    SystemStats stats_;

    // Platform-specific handles
#ifdef Q_OS_WIN
    void* prev_sys_idle_ = nullptr;
    void* prev_sys_kernel_ = nullptr;
    void* prev_sys_user_ = nullptr;
    void* prev_proc_time_ = nullptr;
    void* prev_sys_time_ = nullptr;
    bool first_run_ = true;
#endif
};

} // namespace fincept::core::services
