#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

namespace fincept::screens {

class AgentActionCenter : public QWidget {
    Q_OBJECT
  public:
    explicit AgentActionCenter(QWidget* parent = nullptr);
    ~AgentActionCenter() override;

  private slots:
    void approve_selected();
    void reject_selected();
    void handle_trade_proposed(const class QJsonObject& payload);

  private:
    void setup_ui();

    QTableWidget* table_;
    QPushButton* btn_approve_;
    QPushButton* btn_reject_;
};

} // namespace fincept::screens
