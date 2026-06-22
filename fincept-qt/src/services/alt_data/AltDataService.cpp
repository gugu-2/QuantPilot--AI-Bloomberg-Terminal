#include "services/alt_data/AltDataService.h"
#include "python/PythonRunner.h"
#include "core/events/EventBus.h"
#include "core/logging/Logger.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace fincept::services::alt_data {

AltDataService& AltDataService::instance() {
    static AltDataService inst;
    return inst;
}

void AltDataService::fetch_sentiment(const QString& ticker) {
    LOG_INFO("AltData", "Fetching sentiment for " + ticker);
    fincept::PythonRunner::instance().run("alt_data/sentiment_scraper.py", {ticker}, [ticker](const fincept::PythonResult& res) {
        if (res.success) {
            auto data = fincept::PythonRunner::extract_json(res.output);
            if (!data.isEmpty() && data.keys().count() > 0) {
                core::events::EventBus::instance().publish("altdata.sentiment.updated", data);
            }
        } else {
            LOG_ERROR("AltData", "Sentiment script failed: " + res.error);
        }
    }, {}, 0, true);
}

void AltDataService::fetch_satellite_data(const QString& target) {
    LOG_INFO("AltData", "Fetching satellite data for " + target);
    fincept::PythonRunner::instance().run("alt_data/satellite_parser.py", {target}, [target](const fincept::PythonResult& res) {
        if (res.success) {
            auto data = fincept::PythonRunner::extract_json(res.output);
            if (!data.isEmpty() && data.keys().count() > 0) {
                core::events::EventBus::instance().publish("altdata.satellite.updated", data);
            }
        } else {
            LOG_ERROR("AltData", "Satellite script failed: " + res.error);
        }
    }, {}, 0, true);
}

} // namespace fincept::services::alt_data
