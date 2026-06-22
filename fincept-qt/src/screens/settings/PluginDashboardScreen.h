#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include "core/plugins/PluginManager.h"

namespace fincept::screens {

class PluginDashboardScreen : public QWidget {
    Q_OBJECT
  public:
    explicit PluginDashboardScreen(QWidget* parent = nullptr);

  protected:
    void showEvent(QShowEvent* event) override;

  private slots:
    void on_plugins_reloaded();
    void refresh_ui();
    void run_plugin(const QString& script_path);

  private:
    QVBoxLayout* root_layout_;
    QWidget* grid_widget_;
    QVBoxLayout* grid_layout_;
};

} // namespace fincept::screens
