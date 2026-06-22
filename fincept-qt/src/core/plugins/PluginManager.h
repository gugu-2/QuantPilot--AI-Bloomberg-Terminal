#pragma once
#include <QObject>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QMutex>

namespace fincept::core::plugins {

struct PluginMetadata {
    QString id;
    QString name;
    QString description;
    QString script_path;
    QString version;
};

class PluginManager : public QObject {
    Q_OBJECT
  public:
    static PluginManager& instance();

    void init();
    QList<PluginMetadata> get_installed_plugins() const;
    QString get_plugins_dir() const;
    void set_plugins_dir(const QString& new_dir);

  private slots:
    void on_directory_changed(const QString& path);
    void scan_plugins();

  private:
    PluginManager();
    ~PluginManager() = default;

    QFileSystemWatcher watcher_;
    QString plugins_dir_;
    QList<PluginMetadata> plugins_;
    mutable QMutex mutex_;
};

} // namespace fincept::core::plugins
