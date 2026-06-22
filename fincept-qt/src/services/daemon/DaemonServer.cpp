#include "services/daemon/DaemonServer.h"
#include "core/logging/Logger.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QByteArray>
#include <QCoreApplication>
#include <QTimer>

namespace fincept::services::daemon {

DaemonServer::DaemonServer(QObject* parent) : QObject(parent), server_(new QTcpServer(this)) {
    connect(server_, &QTcpServer::newConnection, this, &DaemonServer::on_new_connection);
}

DaemonServer::~DaemonServer() {
    server_->close();
}

bool DaemonServer::start(quint16 port) {
    if (server_->listen(QHostAddress::Any, port)) {
        LOG_INFO("Daemon", QString("REST API Server listening on port %1").arg(port));
        return true;
    }
    LOG_ERROR("Daemon", "Failed to start REST API server: " + server_->errorString());
    return false;
}

void DaemonServer::on_new_connection() {
    QTcpSocket* socket = server_->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &DaemonServer::on_ready_read);
    connect(socket, &QTcpSocket::disconnected, socket, [this, socket]() {
        buffers_.remove(socket);
        socket->deleteLater();
    });
    
    QTimer::singleShot(5000, socket, &QTcpSocket::disconnectFromHost);
}

void DaemonServer::on_ready_read() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !buffers_.contains(socket) && !socket->bytesAvailable()) return;
    if (!socket) return;

    // Safe append — only if socket is a known tracked connection
    if (!buffers_.contains(socket) && socket->bytesAvailable() == 0) return;
    buffers_[socket].append(socket->readAll());

    // Do not use buffers_[socket] directly after this — use .value() to avoid
    // auto-inserting a default QByteArray for unknown/nullptr sockets.
    const QByteArray& buf = buffers_.value(socket);
    if (!buf.contains("\r\n\r\n")) {
        return; // Wait for full HTTP headers
    }
    
    QByteArray requestData = buffers_.take(socket);
    QString request(requestData);
    
    QByteArray cors_headers = "Access-Control-Allow-Origin: *\r\n"
                              "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type\r\n";

    // Handle OPTIONS preflight (for browser-based clients / local webapps)
    if (request.startsWith("OPTIONS")) {
        QByteArray http_response = "HTTP/1.1 204 No Content\r\n"
                                   + cors_headers +
                                   "Connection: close\r\n\r\n";
        socket->write(http_response);
        socket->disconnectFromHost();
        return;
    }
                              
    if (request.startsWith("GET /status")) {
        QJsonObject response;
        response["status"] = "OK";
        response["uptime"] = "Running";
        response["platform"] = "QuantPilot Headless Daemon";
        response["version"] = QCoreApplication::applicationVersion();
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        
        QJsonDocument doc(response);
        QByteArray body = doc.toJson(QJsonDocument::Compact);
        
        QByteArray http_response = "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: application/json\r\n"
                                   + cors_headers +
                                   "Content-Length: " + QByteArray::number(body.size()) + "\r\n"
                                   "Connection: close\r\n\r\n" + body;
        
        socket->write(http_response);
    } else {
        QByteArray http_response = "HTTP/1.1 404 Not Found\r\n"
                                   + cors_headers +
                                   "Connection: close\r\n\r\n";
        socket->write(http_response);
    }
    socket->disconnectFromHost();
}

} // namespace fincept::services::daemon
