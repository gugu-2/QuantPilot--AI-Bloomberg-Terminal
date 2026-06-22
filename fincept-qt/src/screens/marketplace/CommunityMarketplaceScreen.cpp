#include "screens/marketplace/CommunityMarketplaceScreen.h"
#include "core/plugins/PluginManager.h"
#include "core/logging/Logger.h"

#include <QLabel>
#include <QFrame>
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QVariantMap>
#include "core/events/EventBus.h"

namespace fincept::screens {

CommunityMarketplaceScreen::CommunityMarketplaceScreen(QWidget* parent) : QWidget(parent) {
    root_layout_ = new QVBoxLayout(this);
    
    auto* header = new QLabel("Community Marketplace", this);
    header->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF;");
    root_layout_->addWidget(header);
    
    auto* desc = new QLabel("Discover and install custom agents, themes, and plugins from the Fincept community.", this);
    desc->setStyleSheet("color: #AAAAAA; margin-bottom: 15px;");
    root_layout_->addWidget(desc);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    
    grid_widget_ = new QWidget(scroll);
    grid_layout_ = new QVBoxLayout(grid_widget_);
    grid_layout_->setAlignment(Qt::AlignTop);
    scroll->setWidget(grid_widget_);
    root_layout_->addWidget(scroll);
    
    // Mock Data
    packages_.append({"langgraph_sentiment", "LangGraph Sentiment Analyst", "FinceptTeam", "Runs the multi-source LangGraph sentiment agent (News, Reddit, StockTwits) directly from your Plugin Dashboard.", "2.1.0", 34200});
    packages_.append({"discord_notifier", "Discord Alerts Pro", "FinceptTeam", "Sends trade executions directly to your Discord webhook.", "1.2.0", 15400});
    packages_.append({"arbitrage_bot", "Triangular Arbitrage Agent", "QuantGod", "A fast LangGraph agent for crypto triangular arbitrage.", "2.0.1", 8230});
    packages_.append({"options_flow", "Unusual Options Flow Scanner", "DeltaTrader", "Scans OPRA feed for unusual block trades and sweeps.", "1.0.5", 4120});

    for (const auto& pkg : packages_) {
        auto* card = new QFrame(grid_widget_);
        card->setStyleSheet("QFrame { background: #1E1E1E; border: 1px solid #333333; border-radius: 4px; padding: 12px; margin-bottom: 8px; }");
        auto* card_layout = new QVBoxLayout(card);
        
        auto* top_row = new QHBoxLayout();
        auto* name = new QLabel(pkg.name, card);
        name->setStyleSheet("font-size: 16px; font-weight: bold; color: #55AAFF; border: none;");
        auto* stats = new QLabel(QString("v%1  |  %2 DLs").arg(pkg.version).arg(pkg.downloads), card);
        stats->setStyleSheet("color: #888888; font-size: 12px; border: none;");
        top_row->addWidget(name);
        top_row->addStretch();
        top_row->addWidget(stats);
        card_layout->addLayout(top_row);
        
        auto* author = new QLabel("by " + pkg.author, card);
        author->setStyleSheet("color: #BBBBBB; font-style: italic; border: none; margin-bottom: 5px;");
        card_layout->addWidget(author);
        
        auto* description = new QLabel(pkg.description, card);
        description->setStyleSheet("color: #DDDDDD; border: none;");
        description->setWordWrap(true);
        card_layout->addWidget(description);
        
        auto* btn = new QPushButton("Install", card);
        btn->setStyleSheet("background: #007ACC; color: white; padding: 6px 12px; font-weight: bold; border-radius: 3px; max-width: 100px; margin-top: 5px;");
        connect(btn, &QPushButton::clicked, this, [this, pkg, btn]() {
            install_package(pkg, btn);
        });
        card_layout->addWidget(btn);
        
        grid_layout_->addWidget(card);
    }

    core::events::EventBus::instance().subscribe("copilot.route", this, [this](const QVariantMap& args) {
        if (args["action"].toString() == "marketplace") {
            QString target = args["target"].toString().toLower();
            if (!target.isEmpty()) {
                QMetaObject::invokeMethod(this, [this, target]() {
                    // Try to find a matching package and auto-install it
                    for (int i = 0; i < packages_.size(); ++i) {
                        if (packages_[i].name.toLower().contains(target) || packages_[i].id.toLower().contains(target)) {
                            // Find the install button for this card. Layout structure: card(QVBoxLayout) -> [top_row(QHBoxLayout), author, desc, btn]
                            // To be safe, we just call install_package with a dummy button or search for it.
                            // The easiest way is to search for QPushButton children in grid_widget_
                            QList<QPushButton*> buttons = grid_widget_->findChildren<QPushButton*>();
                            if (i < buttons.size()) {
                                install_package(packages_[i], buttons[i]);
                            }
                            break;
                        }
                    }
                }, Qt::QueuedConnection);
            }
        }
    });
}

void CommunityMarketplaceScreen::install_package(const MarketplacePackage& pkg, QPushButton* btn) {
    btn->setText("Installing...");
    btn->setEnabled(false);
    btn->setStyleSheet("background: #444444; color: #AAAAAA; padding: 6px 12px; font-weight: bold; border-radius: 3px; max-width: 100px; margin-top: 5px;");
    
    // Simulate network download delay
    QTimer::singleShot(1500, this, [=]() {
        QString plugins_dir = core::plugins::PluginManager::instance().get_plugins_dir();
        QDir root_dir(plugins_dir);
        QString target_dir = root_dir.filePath(pkg.id);
        
        if (QDir(target_dir).exists()) {
            btn->setText("Already Installed");
            btn->setStyleSheet("background: #555555; color: #CCCCCC; padding: 6px 12px; font-weight: bold; border-radius: 3px; max-width: 100px; margin-top: 5px;");
            return;
        }
        root_dir.mkdir(pkg.id);
        
        // Write plugin.json
        QJsonObject manifest;
        manifest["id"] = pkg.id;
        manifest["name"] = pkg.name;
        manifest["description"] = pkg.description;
        manifest["version"] = pkg.version;
        manifest["script"] = "main.py";
        
        QFile manifest_file(target_dir + "/plugin.json");
        if (manifest_file.open(QIODevice::WriteOnly)) {
            manifest_file.write(QJsonDocument(manifest).toJson());
            manifest_file.close();
        }
        
        // Write mock python script
        QFile py_file(target_dir + "/main.py");
        if (py_file.open(QIODevice::WriteOnly)) {
            QString code;
            if (pkg.id == "langgraph_sentiment") {
                code = "import sys\n"
                       "import os\n"
                       "script_dir = os.path.dirname(os.path.abspath(__file__))\n"
                       "sys.path.append(os.path.abspath(os.path.join(script_dir, '../../scripts')))\n\n"
                       "from tradingagents.agents.analysts.sentiment_analyst import create_sentiment_analyst\n\n"
                       "print('🚀 Booting LangGraph Sentiment Analyst...')\n"
                       "print('Connecting to Reddit, StockTwits, and Yahoo Finance APIs...')\n"
                       "print('SUCCESS: LangGraph node instantiated and ready for target tickers!')\n";
            } else {
                code = QString("import time\nprint('Running installed package: %1')\n").arg(pkg.name);
            }
            py_file.write(code.toUtf8());
            py_file.close();
        }
        
        LOG_INFO("Marketplace", "Installed package: " + pkg.name);
        
        btn->setText("Installed");
        btn->setStyleSheet("background: #117711; color: white; padding: 6px 12px; font-weight: bold; border-radius: 3px; max-width: 100px; margin-top: 5px;");
        
        // The PluginManager's QFileSystemWatcher will automatically detect the new folder
        // and trigger the EventBus, updating the Plugin Dashboard instantly!
    });
}

} // namespace fincept::screens
