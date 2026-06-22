#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include "core/services/SystemMonitor.h"

namespace fincept::screens::devtools {

class ResourceManager : public QWidget {
    Q_OBJECT
public:
    explicit ResourceManager(QWidget* parent = nullptr);
    ~ResourceManager() override;

private slots:
    void on_stats_updated(const core::services::SystemStats& stats);

private:
    QLabel* cpu_label_ = nullptr;
    QProgressBar* cpu_bar_ = nullptr;
    QLabel* mem_label_ = nullptr;
    QProgressBar* mem_bar_ = nullptr;
    QLabel* thread_label_ = nullptr;
};

} // namespace fincept::screens::devtools
