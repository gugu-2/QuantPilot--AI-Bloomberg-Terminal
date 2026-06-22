#include "core/plugins/PluginManager.h"
#include "core/events/EventBus.h"
#include "core/logging/Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

namespace fincept::core::plugins {

PluginManager& PluginManager::instance() {
    static PluginManager inst;
    return inst;
}

PluginManager::PluginManager() {
    QString app_data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    plugins_dir_ = QDir(app_data).filePath("plugins");
    QDir().mkpath(plugins_dir_);
}

void PluginManager::init() {
    watcher_.addPath(plugins_dir_);
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, &PluginManager::on_directory_changed);
    scan_plugins();
}

QString PluginManager::get_plugins_dir() const {
    return plugins_dir_;
}

void PluginManager::set_plugins_dir(const QString& new_dir) {
    if (plugins_dir_ == new_dir) return;
    
    if (!watcher_.directories().isEmpty()) {
        watcher_.removePaths(watcher_.directories());
    }
    
    plugins_dir_ = new_dir;
    QDir().mkpath(plugins_dir_);
    watcher_.addPath(plugins_dir_);
    
    scan_plugins();
}

void PluginManager::on_directory_changed(const QString& path) {
    Q_UNUSED(path);
    scan_plugins();
}

void PluginManager::scan_plugins() {
    QList<PluginMetadata> new_plugins;
    QStringList paths_to_watch;
    QDir dir(plugins_dir_);
    auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const auto& entry : entries) {
        QString plugin_path = dir.filePath(entry);
        QString manifest_path = plugin_path + "/plugin.json";
        
        QFile file(manifest_path);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                PluginMetadata meta;
                meta.id = obj.value("id").toString(entry);
                meta.name = obj.value("name").toString(entry);
                meta.description = obj.value("description").toString();
                meta.version = obj.value("version").toString("1.0.0");
                
                QString script_name = obj.value("script").toString("main.py");
                meta.script_path = plugin_path + "/" + script_name;
                
                new_plugins.append(meta);
                
                // Collect paths to watch — do NOT call watcher_.addPath() while
                // holding mutex_ (addPath can emit directoryChanged synchronously
                // on some platforms, which would re-enter scan_plugins and deadlock).
                if (!watcher_.directories().contains(plugin_path)) {
                    paths_to_watch.append(plugin_path);
                }
            }
        }
    }
    
    // Update plugin list under mutex
    {
        QMutexLocker locker(&mutex_);
        plugins_ = new_plugins;
    }
    
    // Add watcher paths AFTER releasing mutex to avoid potential deadlock
    if (!paths_to_watch.isEmpty()) {
        watcher_.addPaths(paths_to_watch);
    }
    
    LOG_INFO("PluginManager", QString("Scanned %1 dynamic plugins").arg(new_plugins.size()));
    core::events::EventBus::instance().publish("plugins.reloaded", QJsonObject());
}

QList<PluginMetadata> PluginManager::get_installed_plugins() const {
    QMutexLocker locker(&mutex_);
    return plugins_;
}

} // namespace fincept::core::plugins
