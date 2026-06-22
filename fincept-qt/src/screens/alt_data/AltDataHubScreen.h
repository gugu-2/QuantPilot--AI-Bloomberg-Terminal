#pragma once
#include <QWidget>
class QLineEdit;
class QTextEdit;
#include <QJsonObject>

namespace fincept::screens {

class AltDataHubScreen : public QWidget {
    Q_OBJECT
  public:
    explicit AltDataHubScreen(QWidget* parent = nullptr);

  private slots:
    void fetch_sentiment();
    void fetch_satellite();
    void on_sentiment_updated(const QJsonObject& data);
    void on_satellite_updated(const QJsonObject& data);

  private:
    QLineEdit* sentiment_input_;
    QTextEdit* sentiment_output_;
    
    QLineEdit* satellite_input_;
    QTextEdit* satellite_output_;
};

} // namespace fincept::screens
