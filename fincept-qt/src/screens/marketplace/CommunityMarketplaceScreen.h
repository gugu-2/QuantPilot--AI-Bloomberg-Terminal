#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QList>

namespace fincept::screens {

struct MarketplacePackage {
    QString id;
    QString name;
    QString author;
    QString description;
    QString version;
    int downloads;
};

class CommunityMarketplaceScreen : public QWidget {
    Q_OBJECT
  public:
    explicit CommunityMarketplaceScreen(QWidget* parent = nullptr);

  private slots:
    void install_package(const MarketplacePackage& pkg, QPushButton* btn);

  private:
    QVBoxLayout* root_layout_;
    QWidget* grid_widget_;
    QVBoxLayout* grid_layout_;
    QList<MarketplacePackage> packages_;
};

} // namespace fincept::screens
