#include "screens/ai_quant_lab/AgentActionCenter.h"
#include "core/events/EventBus.h"
#include "ui/theme/Theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

namespace fincept::screens {

AgentActionCenter::AgentActionCenter(QWidget* parent) : QWidget(parent) {
    setup_ui();

    // Subscribe to AI trade proposals
    core::events::EventBus::instance().subscribe(
        "ai.trade.proposed", this, [this](const QJsonObject& payload) {
            handle_trade_proposed(payload);
        });
}

AgentActionCenter::~AgentActionCenter() {
    core::events::EventBus::instance().unsubscribe("ai.trade.proposed", this);
}

void AgentActionCenter::setup_ui() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* title = new QLabel(tr("Agent Action Center (Human-In-The-Loop)"));
    title->setStyleSheet(QString("color:%1; font-size:18px; font-weight:bold;")
                             .arg(ui::colors::TEXT_PRIMARY()));
    layout->addWidget(title);

    auto* desc = new QLabel(tr("Review and authorize trades proposed by autonomous AI agents. Trades will not be executed until explicitly approved."));
    desc->setStyleSheet(QString("color:%1; font-size:12px;").arg(ui::colors::TEXT_SECONDARY()));
    desc->setWordWrap(true);
    layout->addWidget(desc);

    table_ = new QTableWidget(0, 6, this);
    table_->setHorizontalHeaderLabels({"Time", "Agent", "Ticker", "Action", "Quantity", "Reasoning"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setStyleSheet(QString("QTableWidget { background-color: %1; color: %2; }")
                              .arg(ui::colors::BG_ELEVATED())
                              .arg(ui::colors::TEXT_PRIMARY()));
    layout->addWidget(table_, 1);

    auto* btn_layout = new QHBoxLayout;
    btn_reject_ = new QPushButton(tr("Reject Trade"));
    btn_reject_->setStyleSheet(QString("background-color: %1; color: white; padding: 8px; font-weight: bold; border-radius: 4px;")
                                   .arg(ui::colors::TRADE_SELL()));
    btn_layout->addWidget(btn_reject_);

    btn_approve_ = new QPushButton(tr("Approve Trade"));
    btn_approve_->setStyleSheet(QString("background-color: %1; color: white; padding: 8px; font-weight: bold; border-radius: 4px;")
                                    .arg(ui::colors::TRADE_BUY()));
    btn_layout->addWidget(btn_approve_);
    
    layout->addLayout(btn_layout);

    connect(btn_approve_, &QPushButton::clicked, this, &AgentActionCenter::approve_selected);
    connect(btn_reject_, &QPushButton::clicked, this, &AgentActionCenter::reject_selected);
}

void AgentActionCenter::handle_trade_proposed(const QJsonObject& payload) {
    int row = table_->rowCount();
    table_->insertRow(row);

    QString time_str = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString agent = payload.value("agent").toString("TradingAgent");
    QString ticker = payload.value("ticker").toString();
    QString action = payload.value("action").toString();
    QString qty = QString::number(payload.value("quantity").toDouble());
    QString reasoning = payload.value("reasoning").toString();

    table_->setItem(row, 0, new QTableWidgetItem(time_str));
    table_->setItem(row, 1, new QTableWidgetItem(agent));
    table_->setItem(row, 2, new QTableWidgetItem(ticker));
    
    auto* action_item = new QTableWidgetItem(action);
    action_item->setForeground(action.toUpper() == "BUY" ? QColor(ui::colors::TRADE_BUY()) : QColor(ui::colors::TRADE_SELL()));
    table_->setItem(row, 3, action_item);
    
    table_->setItem(row, 4, new QTableWidgetItem(qty));
    table_->setItem(row, 5, new QTableWidgetItem(reasoning));

    // Store payload in the row for retrieval on approve
    table_->item(row, 0)->setData(Qt::UserRole, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

void AgentActionCenter::approve_selected() {
    int row = table_->currentRow();
    if (row < 0) return;

    QByteArray raw_payload = table_->item(row, 0)->data(Qt::UserRole).toByteArray();
    QJsonObject payload = QJsonDocument::fromJson(raw_payload).object();

    // Emit the authorization event
    core::events::EventBus::instance().publish("ai.trade.approved", payload);

    QMessageBox::information(this, tr("Trade Approved"), tr("The trade for %1 has been authorized and dispatched to the execution engine.").arg(payload.value("ticker").toString()));
    table_->removeRow(row);
}

void AgentActionCenter::reject_selected() {
    int row = table_->currentRow();
    if (row < 0) return;

    table_->removeRow(row);
}

} // namespace fincept::screens
