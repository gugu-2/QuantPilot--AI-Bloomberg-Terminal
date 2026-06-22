#pragma once
#include <QObject>
#include <QString>

namespace fincept::services::alt_data {

class AltDataService : public QObject {
    Q_OBJECT
  public:
    static AltDataService& instance();

    void fetch_sentiment(const QString& ticker);
    void fetch_satellite_data(const QString& target);

  private:
    AltDataService() = default;
    ~AltDataService() = default;
};

} // namespace fincept::services::alt_data
