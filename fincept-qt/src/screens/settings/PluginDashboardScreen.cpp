#include "screens/settings/PluginDashboardScreen.h"
#include "core/events/EventBus.h"
#include "core/logging/Logger.h"
#include "python/PythonRunner.h"
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QShowEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QFrame>

namespace fincept::screens {

PluginDashboardScreen::PluginDashboardScreen(QWidget* parent) : QWidget(parent) {
    root_layout_ = new QVBoxLayout(this);
    
    auto* header = new QLabel("Dynamic Plugins", this);
    header->setStyleSheet("font-size: 18px; font-weight: bold; color: #FFFFFF;");
    root_layout_->addWidget(header);
    
    auto* desc = new QLabel("Drop plugin folders (with plugin.json and main.py) into the plugins directory. They will appear here instantly.", this);
    desc->setStyleSheet("color: #AAAAAA; margin-bottom: 10px;");
    root_layout_->addWidget(desc);

    auto* open_dir_btn = new QPushButton("Open Plugins Directory", this);
    open_dir_btn->setStyleSheet("background: #333333; color: white; padding: 6px;");
    connect(open_dir_btn, &QPushButton::clicked, this, []() {
        QString dir = core::plugins::PluginManager::instance().get_plugins_dir();
        LOG_INFO("PluginDashboard", "Plugins dir: " + dir);
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });
    root_layout_->addWidget(open_dir_btn);
    
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    grid_widget_ = new QWidget(scroll);
    grid_layout_ = new QVBoxLayout(grid_widget_);
    grid_layout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(grid_widget_);
    
    root_layout_->addWidget(scroll);
    
    core::events::EventBus::instance().subscribe("plugins.reloaded", this, [this](const QJsonValue&) {
        QMetaObject::invokeMethod(this, "on_plugins_reloaded", Qt::QueuedConnection);
    });

    core::events::EventBus::instance().subscribe("copilot.route", this, [this](const QVariantMap& args) {
        if (args["action"].toString() == "plugins") {
            QString package_name = args["target"].toString().toLower();
            QMetaObject::invokeMethod(this, [this, package_name]() {
                refresh_ui();
                if (!package_name.isEmpty()) {
                    auto plugins = core::plugins::PluginManager::instance().get_installed_plugins();
                    for (const auto& p : plugins) {
                        if (p.name.toLower().contains(package_name) || p.id.toLower().contains(package_name)) {
                            run_plugin(p.script_path);
                            break;
                        }
                    }
                }
            }, Qt::QueuedConnection);
        }
    });
}

void PluginDashboardScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refresh_ui();
}

void PluginDashboardScreen::on_plugins_reloaded() {
    refresh_ui();
}

void PluginDashboardScreen::refresh_ui() {
    // Clear old layout
    while (QLayoutItem* item = grid_layout_->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
    
    auto plugins = core::plugins::PluginManager::instance().get_installed_plugins();
    
    if (plugins.isEmpty()) {
        auto* empty = new QLabel("No dynamic plugins found.", grid_widget_);
        empty->setStyleSheet("color: #666666; font-style: italic;");
        grid_layout_->addWidget(empty);
        return;
    }
    
    for (const auto& p : plugins) {
        auto* card = new QFrame(grid_widget_);
        card->setStyleSheet("QFrame { background: #1A1A1A; border: 1px solid #333333; border-radius: 4px; padding: 10px; margin-bottom: 5px; }");
        auto* card_layout = new QVBoxLayout(card);
        
        auto* name = new QLabel(QString("%1 (v%2)").arg(p.name, p.version), card);
        name->setStyleSheet("font-size: 14px; font-weight: bold; color: #55AAFF;");
        card_layout->addWidget(name);
        
        auto* desc = new QLabel(p.description, card);
        desc->setStyleSheet("color: #CCCCCC;");
        desc->setWordWrap(true);
        card_layout->addWidget(desc);
        
        auto* btn = new QPushButton("Run Plugin", card);
        btn->setStyleSheet("background: #007ACC; color: white; padding: 5px; font-weight: bold; border-radius: 3px; max-width: 120px;");
        QString script_path = p.script_path;
        connect(btn, &QPushButton::clicked, this, [this, script_path]() {
            run_plugin(script_path);
        });
        card_layout->addWidget(btn);
        
        grid_layout_->addWidget(card);
    }
}

void PluginDashboardScreen::run_plugin(const QString& script_path) {
    LOG_INFO("PluginDashboard", "Executing dynamic plugin: " + script_path);
    // Execute as background task, but we need run_absolute because it's not in the scripts dir.
    // PythonRunner::instance().run uses `scripts/` prefix if no '/' is present.
    // Let's check how PythonRunner handles absolute paths.
    // If we just pass the absolute path, does it work? Wait, PythonRunner::run prepends scripts/ if it's a relative path without dir prefix.
    // For absolute path on Windows (e.g. C:/...), it has a colon. Let's look at PythonRunner.cpp.
    PythonRunner::instance().run(script_path, {}, [](const PythonResult& res) {
        if (res.success) {
            LOG_INFO("PluginDashboard", "Plugin execution success:\n" + res.output);
        } else {
            LOG_ERROR("PluginDashboard", "Plugin execution failed:\n" + res.error);
        }
    }, {}, 0, true);
}

} // namespace fincept::screens
