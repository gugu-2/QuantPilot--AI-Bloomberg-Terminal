#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

namespace fincept::services::daemon {

class DaemonServer : public QObject {
    Q_OBJECT
  public:
    explicit DaemonServer(QObject* parent = nullptr);
    ~DaemonServer() override;

    bool start(quint16 port = 8080);

  private slots:
    void on_new_connection();
    void on_ready_read();

  private:
    QTcpServer* server_;
    QHash<QTcpSocket*, QByteArray> buffers_;
};

} // namespace fincept::services::daemon
