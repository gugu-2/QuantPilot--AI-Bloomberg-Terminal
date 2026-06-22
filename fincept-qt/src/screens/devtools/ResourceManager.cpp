#include "ResourceManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFont>

namespace fincept::screens::devtools {

ResourceManager::ResourceManager(QWidget* parent) : QWidget(parent) {
    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(16, 16, 16, 16);
    main_layout->setSpacing(12);

    auto* group_box = new QGroupBox(tr("System Resource Usage"), this);
    auto* group_layout = new QVBoxLayout(group_box);

    QFont value_font;
    value_font.setFamily("Consolas");
    value_font.setPointSize(11);
    value_font.setBold(true);

    // CPU
    auto* cpu_layout = new QHBoxLayout();
    auto* cpu_title = new QLabel(tr("CPU Usage:"), this);
    cpu_title->setFixedWidth(100);
    cpu_bar_ = new QProgressBar(this);
    cpu_bar_->setRange(0, 100);
    cpu_bar_->setTextVisible(false);
    cpu_label_ = new QLabel("0.0 %", this);
    cpu_label_->setFont(value_font);
    cpu_label_->setFixedWidth(80);
    cpu_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    cpu_layout->addWidget(cpu_title);
    cpu_layout->addWidget(cpu_bar_);
    cpu_layout->addWidget(cpu_label_);

    // Memory
    auto* mem_layout = new QHBoxLayout();
    auto* mem_title = new QLabel(tr("RAM Usage:"), this);
    mem_title->setFixedWidth(100);
    mem_bar_ = new QProgressBar(this);
    mem_bar_->setRange(0, 100); // 0 to 100% (pseudo, we don't know total sys RAM here easily, so we just show raw MB)
    mem_bar_->setTextVisible(false);
    mem_label_ = new QLabel("0 MB", this);
    mem_label_->setFont(value_font);
    mem_label_->setFixedWidth(80);
    mem_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mem_layout->addWidget(mem_title);
    mem_layout->addWidget(mem_bar_);
    mem_layout->addWidget(mem_label_);

    // Threads
    auto* thread_layout = new QHBoxLayout();
    auto* thread_title = new QLabel(tr("Active Threads:"), this);
    thread_title->setFixedWidth(100);
    thread_label_ = new QLabel("N/A", this);
    thread_label_->setFont(value_font);
    
    thread_layout->addWidget(thread_title);
    thread_layout->addWidget(thread_label_);
    thread_layout->addStretch();

    group_layout->addLayout(cpu_layout);
    group_layout->addLayout(mem_layout);
    group_layout->addLayout(thread_layout);
    
    main_layout->addWidget(group_box);
    main_layout->addStretch();

    // Connect to monitor
    connect(&core::services::SystemMonitor::instance(), &core::services::SystemMonitor::stats_updated,
            this, &ResourceManager::on_stats_updated);
}

ResourceManager::~ResourceManager() = default;

void ResourceManager::on_stats_updated(const core::services::SystemStats& stats) {
    // Update CPU
    cpu_bar_->setValue(static_cast<int>(stats.cpu_usage_percent));
    cpu_label_->setText(QString::number(stats.cpu_usage_percent, 'f', 1) + " %");

    // Update Memory
    double mb = static_cast<double>(stats.memory_used_bytes) / (1024.0 * 1024.0);
    mem_label_->setText(QString::number(mb, 'f', 1) + " MB");
    
    // Pseudo progress based on arbitrary 2GB max for visual flair
    int mem_pct = static_cast<int>((mb / 2048.0) * 100);
    if (mem_pct > 100) mem_pct = 100;
    mem_bar_->setValue(mem_pct);

    // Update Threads
    if (stats.thread_count >= 0) {
        thread_label_->setText(QString::number(stats.thread_count));
    } else {
        thread_label_->setText("N/A (OS limited)");
    }
}

} // namespace fincept::screens::devtools
