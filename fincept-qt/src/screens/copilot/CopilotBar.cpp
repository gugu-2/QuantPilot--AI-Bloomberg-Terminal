#include "screens/copilot/CopilotBar.h"
#include "services/copilot/GeminiAutomatorService.h"
#include "core/events/EventBus.h"
#include "services/llm/LlmService.h"
#include "ui/theme/ThemeManager.h"
#include <QHBoxLayout>
#include <QVariantMap>
#include <QTimer>

namespace fincept::screens {

CopilotBar::CopilotBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(40);
    setStyleSheet(QString("background: %1; border-bottom: 1px solid %2;")
                  .arg(ui::theme::ThemeManager::instance().color(ui::theme::ThemeTokens::BG_BASE).name())
                  .arg(ui::theme::ThemeManager::instance().color(ui::theme::ThemeTokens::BORDER).name()));
    
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    
    input_ = new QLineEdit(this);
    input_->setPlaceholderText("✨ Gemini Copilot: Type a command (e.g. 'Run sentiment on AAPL' or 'Open marketplace')");
    input_->setStyleSheet(QString("background: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 10px; font-size: 14px;")
                          .arg(ui::theme::ThemeManager::instance().color(ui::theme::ThemeTokens::BG_ELEVATED_1).name())
                          .arg(ui::theme::ThemeManager::instance().color(ui::theme::ThemeTokens::TEXT_PRIMARY).name())
                          .arg(ui::theme::ThemeManager::instance().color(ui::theme::ThemeTokens::BORDER).name()));
    
    layout->addWidget(input_);
    
    connect(input_, &QLineEdit::returnPressed, this, &CopilotBar::on_return_pressed);

    core::events::EventBus::instance().subscribe("copilot.route", this, [this](const QVariantMap&) {
        QPointer<CopilotBar> self(this);
        QMetaObject::invokeMethod(this, [self]() {
            if (!self) return;
            self->input_->setPlaceholderText("✨ Gemini Copilot: Type a command (e.g. 'Run sentiment on AAPL')");
            self->input_->setEnabled(true);
        }, Qt::QueuedConnection);
    });

    core::events::EventBus::instance().subscribe("copilot.error", this, [this](const QVariantMap& args) {
        QString err = args["error"].toString();
        QPointer<CopilotBar> self(this);
        QMetaObject::invokeMethod(this, [self, err]() {
            if (!self) return;
            self->input_->setPlaceholderText("⚠️ Error: " + err);
            self->input_->setEnabled(true);
            QTimer::singleShot(3000, self, [self]() {
                if (!self) return;
                self->input_->setPlaceholderText("✨ Gemini Copilot: Type a command (e.g. 'Run sentiment on AAPL')");
            });
        }, Qt::QueuedConnection);
    });
}

void CopilotBar::on_return_pressed() {
    QString text = input_->text().trimmed();
    if (!text.isEmpty()) {
        if (!fincept::ai_chat::LlmService::instance().is_configured()) {
            input_->setPlaceholderText("⚠️ Error: No API Key configured. Please add one in Settings.");
            input_->clear();
            QTimer::singleShot(3000, this, [this]() {
                input_->setPlaceholderText("✨ Gemini Copilot: Type a command (e.g. 'Run sentiment on AAPL')");
            });
            return;
        }
        
        input_->setPlaceholderText("✨ Thinking...");
        input_->setEnabled(false);
        services::copilot::GeminiAutomatorService::instance().execute_command(text);
        input_->clear();
    }
}

} // namespace fincept::screens
