#pragma once
#include <QObject>
#include <QString>

namespace fincept::services::copilot {

class GeminiAutomatorService : public QObject {
    Q_OBJECT
  public:
    static GeminiAutomatorService& instance();

    /// Sends the command text to Gemini and routes the UI programmatically.
    void execute_command(const QString& user_text);

  private:
    GeminiAutomatorService() = default;
    ~GeminiAutomatorService() = default;
};

} // namespace fincept::services::copilot
