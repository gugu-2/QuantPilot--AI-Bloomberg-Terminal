#pragma once
#include <QWidget>
#include <QLineEdit>

namespace fincept::screens {

class CopilotBar : public QWidget {
    Q_OBJECT
  public:
    explicit CopilotBar(QWidget* parent = nullptr);

  private slots:
    void on_return_pressed();

  private:
    QLineEdit* input_;
};

} // namespace fincept::screens
