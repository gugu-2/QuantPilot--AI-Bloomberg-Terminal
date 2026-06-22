#include "screens/alt_data/AltDataHubScreen.h"
#include "services/alt_data/AltDataService.h"
#include "core/events/EventBus.h"

#include <QLabel>
#include <QJsonDocument>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVariantMap>
#include <QPointer>

namespace fincept::screens {

AltDataHubScreen::AltDataHubScreen(QWidget* parent) : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    
    // Left side: Sentiment
    auto* left_panel = new QFrame(this);
    left_panel->setStyleSheet("QFrame { background: #1E1E1E; border: 1px solid #333; border-radius: 4px; }");
    auto* left_layout = new QVBoxLayout(left_panel);
    
    auto* sent_lbl = new QLabel("Social Sentiment (Reddit & X)", left_panel);
    sent_lbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #55AAFF;");
    left_layout->addWidget(sent_lbl);
    
    auto* sent_h = new QHBoxLayout();
    sentiment_input_ = new QLineEdit(left_panel);
    sentiment_input_->setPlaceholderText("Enter Ticker (e.g. TSLA)");
    sentiment_input_->setStyleSheet("padding: 4px; border: 1px solid #555; background: #222; color: #FFF;");
    auto* sent_btn = new QPushButton("Fetch Sentiment", left_panel);
    sent_btn->setStyleSheet("background: #007ACC; color: white; padding: 6px; font-weight: bold; border-radius: 2px;");
    sent_h->addWidget(sentiment_input_);
    sent_h->addWidget(sent_btn);
    left_layout->addLayout(sent_h);
    
    sentiment_output_ = new QTextEdit(left_panel);
    sentiment_output_->setReadOnly(true);
    sentiment_output_->setStyleSheet("background: #111; color: #0F0; font-family: monospace; border: none;");
    left_layout->addWidget(sentiment_output_);
    
    layout->addWidget(left_panel);
    
    // Right side: Satellite
    auto* right_panel = new QFrame(this);
    right_panel->setStyleSheet("QFrame { background: #1E1E1E; border: 1px solid #333; border-radius: 4px; }");
    auto* right_layout = new QVBoxLayout(right_panel);
    
    auto* sat_lbl = new QLabel("Satellite Imagery (Retail Density)", right_panel);
    sat_lbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #55AAFF;");
    right_layout->addWidget(sat_lbl);
    
    auto* sat_h = new QHBoxLayout();
    satellite_input_ = new QLineEdit(right_panel);
    satellite_input_->setPlaceholderText("Enter Target (e.g. WMT)");
    satellite_input_->setStyleSheet("padding: 4px; border: 1px solid #555; background: #222; color: #FFF;");
    auto* sat_btn = new QPushButton("Analyze Imagery", right_panel);
    sat_btn->setStyleSheet("background: #007ACC; color: white; padding: 6px; font-weight: bold; border-radius: 2px;");
    sat_h->addWidget(satellite_input_);
    sat_h->addWidget(sat_btn);
    right_layout->addLayout(sat_h);
    
    satellite_output_ = new QTextEdit(right_panel);
    satellite_output_->setReadOnly(true);
    satellite_output_->setStyleSheet("background: #111; color: #0F0; font-family: monospace; border: none;");
    right_layout->addWidget(satellite_output_);
    
    layout->addWidget(right_panel);
    
    // Connect buttons
    connect(sent_btn, &QPushButton::clicked, this, &AltDataHubScreen::fetch_sentiment);
    connect(sat_btn, &QPushButton::clicked, this, &AltDataHubScreen::fetch_satellite);
    
    // Wire up Gemini Copilot Automator
    core::events::EventBus::instance().subscribe("copilot.route", this, [this](const QVariantMap& args) {
        if (args["action"].toString() == "alt_data") {
            QString ticker = args["target"].toString();
            if (!ticker.isEmpty()) {
                QPointer<AltDataHubScreen> self(this);
                QMetaObject::invokeMethod(this, [self, ticker]() {
                    if (self) {
                        self->sentiment_input_->setText(ticker);
                        self->fetch_sentiment();
                    }
                }, Qt::QueuedConnection);
            }
        }
    });
    
    // Subscribe to EventBus — use lambda capture to avoid Qt6 pointer-to-member
    // invokeMethod argument passing incompatibility (requires Q_ARG wrappers).
    core::events::EventBus::instance().subscribe("altdata.sentiment.updated", this, [this](const QJsonValue& val) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            QPointer<AltDataHubScreen> self(this);
            QMetaObject::invokeMethod(this, [self, obj]() {
                if (self) self->on_sentiment_updated(obj);
            }, Qt::QueuedConnection);
        }
    });

    core::events::EventBus::instance().subscribe("altdata.satellite.updated", this, [this](const QJsonValue& val) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            QPointer<AltDataHubScreen> self(this);
            QMetaObject::invokeMethod(this, [self, obj]() {
                if (self) self->on_satellite_updated(obj);
            }, Qt::QueuedConnection);
        }
    });
}

void AltDataHubScreen::fetch_sentiment() {
    QString ticker = sentiment_input_->text().trimmed();
    if (ticker.isEmpty()) return;
    sentiment_output_->append("Initializing sentiment scrapers for " + ticker + "...");
    services::alt_data::AltDataService::instance().fetch_sentiment(ticker);
}

void AltDataHubScreen::fetch_satellite() {
    QString target = satellite_input_->text().trimmed();
    if (target.isEmpty()) return;
    satellite_output_->append("Acquiring satellite pass for " + target + "...");
    services::alt_data::AltDataService::instance().fetch_satellite_data(target);
}

void AltDataHubScreen::on_sentiment_updated(const QJsonObject& data) {
    QJsonDocument doc(data);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    sentiment_output_->append("\n[RESULT]:\n" + jsonStr);
}

void AltDataHubScreen::on_satellite_updated(const QJsonObject& data) {
    QJsonDocument doc(data);
    QString jsonStr = doc.toJson(QJsonDocument::Indented);
    satellite_output_->append("\n[RESULT]:\n" + jsonStr);
}

} // namespace fincept::screens
