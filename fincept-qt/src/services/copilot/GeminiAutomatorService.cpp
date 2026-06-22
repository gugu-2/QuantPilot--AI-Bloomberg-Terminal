#include "services/copilot/GeminiAutomatorService.h"
#include "services/llm/LlmService.h"
#include "core/events/EventBus.h"
#include "core/logging/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QtConcurrent>

namespace fincept::services::copilot {

GeminiAutomatorService& GeminiAutomatorService::instance() {
    static GeminiAutomatorService inst;
    return inst;
}

void GeminiAutomatorService::execute_command(const QString& user_text) {
    LOG_INFO("Copilot", "Executing command: " + user_text);
    
    QString prompt = QString("You are the Fincept Terminal Copilot.\n"
                             "The user wants to automate a software task.\n"
                             "User command: '%1'\n\n"
                             "Output ONLY valid JSON matching this schema:\n"
                             "{\n"
                             "  \"action\": \"alt_data\" | \"marketplace\" | \"plugins\",\n"
                             "  \"target\": \"<TICKER or PACKAGE or null>\"\n"
                             "}").arg(user_text);

    if (!fincept::ai_chat::LlmService::instance().is_configured()) {
        QJsonObject error_obj;
        error_obj["error"] = "No API Key configured. Please add one in Settings.";
        core::events::EventBus::instance().publish("copilot.error", error_obj);
        LOG_ERROR("Copilot", "Cannot execute command: LLM is not configured.");
        return;
    }
                             
    QtConcurrent::run([prompt]() {
        fincept::ai_chat::LlmService& llm = fincept::ai_chat::LlmService::instance();
        std::vector<fincept::ai_chat::ConversationMessage> history;
        
        auto response = llm.chat(prompt, history, false);
        
        if (response.success) {
            QString content = response.content.trimmed();
            
            if (content.startsWith("```json")) {
                content = content.mid(7);
                if (content.endsWith("```")) {
                    content = content.left(content.length() - 3);
                }
            } else if (content.startsWith("```")) {
                content = content.mid(3);
                if (content.endsWith("```")) {
                    content = content.left(content.length() - 3);
                }
            }
            
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &err);
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                QString action = obj["action"].toString();
                
                LOG_INFO("Copilot", "Parsed action: " + action);
                QMetaObject::invokeMethod(qApp, [obj]() {
                    core::events::EventBus::instance().publish("copilot.route", obj);
                }, Qt::QueuedConnection);
            } else {
                LOG_ERROR("Copilot", "Failed to parse JSON from Copilot: " + content);
                QJsonObject error_obj;
                error_obj["error"] = "Failed to understand command.";
                QMetaObject::invokeMethod(qApp, [error_obj]() {
                    core::events::EventBus::instance().publish("copilot.error", error_obj);
                }, Qt::QueuedConnection);
            }
        } else {
            LOG_ERROR("Copilot", "LLM request failed: " + response.error);
            QJsonObject error_obj;
            error_obj["error"] = "LLM request failed: " + response.error;
            QMetaObject::invokeMethod(qApp, [error_obj]() {
                core::events::EventBus::instance().publish("copilot.error", error_obj);
            }, Qt::QueuedConnection);
        }
    });
}

} // namespace fincept::services::copilot
