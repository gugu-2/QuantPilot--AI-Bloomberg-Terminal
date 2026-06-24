# QuantPilot

<div align="center">

[![License: AGPL-3.0](https://img.shields.io/badge/license-AGPL--3.0-C06524)](https://github.com/QuantPilot/QuantPilot/blob/main/LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)](https://isocpp.org/)
[![Qt6](https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white)](https://www.qt.io/)
[![Python](https://img.shields.io/badge/Python-3.11+-3776AB?logo=python&logoColor=white)](https://www.python.org/)

### **Your AI Copilot for Quantitative Finance.**

State-of-the-art financial intelligence platform powered by an autonomous LangGraph trading agent framework, a Gemini-powered Natural Language Copilot, and unlimited data connectivity.

[📥 Download](https://github.com/QuantPilot/QuantPilot/releases) · [⚖️ License](https://github.com/QuantPilot/QuantPilot/blob/main/docs/COMMERCIAL_LICENSE.md) · [💬 Discussions](https://github.com/QuantPilot/QuantPilot/discussions)

</div>

---

## About

**QuantPilot** is a pure native C++20 desktop application designed for the AI era of finance. It uses **Qt6** for a blazing-fast UI and embedded **Python** for executing sophisticated, long-running LangGraph agent architectures. Rather than just displaying data, QuantPilot actively analyzes, reasons, and executes via its autonomous agent hierarchy and Gemini LLM backend.

---

## What's New: The AI Era

| **Feature** | **Description** |
|-------------|-----------------|
| 🧠 **Gemini Copilot Engine** | A floating natural language interface that converts plain English (e.g., "Run sentiment on AAPL") into strict JSON, instantly automating complex UI and research tasks via the global EventBus. |
| 🤖 **LangGraph Trading Agents** | Deploy multi-agent teams (Fundamental, Sentiment, Technical Analysts) that actively debate market conditions in the background before proposing trades. |
| 🧩 **Community Marketplace** | Hot-swappable AI plugins. Instantly download third-party Python algorithms, parsers, and custom LangChain agents directly into your workspace. |
| 🛡️ **Human-In-The-Loop Execution** | Deep C++ memory protections and sandbox timeouts keep AI scripts contained. Agents can only *propose* trades to the UI; no live money executes without a human click. |
| 🌐 **100+ Data Connectors** | DBnomics, Polygon, Kraken, Yahoo Finance, FRED, IMF, World Bank, AkShare, plus alternative-data overlays like Reddit/StockTwits sentiment tracking. |

---

## Deep Dive Documentation

QuantPilot has evolved massively. To understand the new autonomous and structural features, check out our newly minted documentation:

- 🏗️ [**AI Architecture & Flows**](docs/AI_ARCHITECTURE_FLOW.md): Mermaid.js diagrams showing how natural language traverses the application.
- 🤖 [**Agentic Framework (LangGraph)**](docs/AGENTIC_FRAMEWORK.md): How the Python multi-agent hierarchy reasons and debates.
- ✨ [**Gemini Copilot Integration**](docs/AI_COPILOT_INTEGRATION.md): The core C++ LLM Service and EventBus automation engine.
- 🧩 [**Marketplace Plugins Guide**](quantpilot-qt/docs/MARKETPLACE_PLUGINS.md): How to develop, manifest, and dynamically install new AI packages.

---

## Installation

### Option 1 — Download Installer (Recommended)

Latest release: **v4.0.3** — [View all releases](https://github.com/QuantPilot/QuantPilot/releases/tag/v4.0.3)

| Platform | Download | Run |
|----------|----------|-----|
| **Windows x64** | [QuantPilot-Windows-x64-setup.exe](https://github.com/QuantPilot/QuantPilot/releases/download/v4.0.3/QuantPilot-4.0.3-windows-x64-setup.exe) | Run installer → launch |
| **Linux x64** | [QuantPilot-Linux-x64.run](https://github.com/QuantPilot/QuantPilot/releases/download/v4.0.3/QuantPilot-4.0.3-linux-x64-setup.run) | `chmod +x` → run installer |
| **macOS Apple Silicon** | [QuantPilot-macOS-arm64.dmg](https://github.com/QuantPilot/QuantPilot/releases/download/v4.0.3/QuantPilot-4.0.3-macos-arm64-setup.dmg) | Open DMG → drag to Applications |

---

### Option 2 — Quick Start (One-Click Build)

Clone and run the setup script — it installs all dependencies and builds the app automatically:

```bash
# Linux / macOS
git clone https://github.com/QuantPilot/QuantPilot.git
cd QuantPilot
chmod +x setup.sh && ./setup.sh
```

---

### Option 3 — Build from Source (Manual)

> **Versions are pinned.** Use the exact versions below. Newer or older versions are unsupported and may fail to build or produce unstable binaries.

#### Prerequisites (exact versions)

| Tool | Pinned Version | Notes |
|------|----------------|-------|
| **Git** | latest | — |
| **CMake** | **3.27.7** | [Download](https://cmake.org/download/) |
| **Ninja** | **1.11.1** | [Download](https://github.com/ninja-build/ninja/releases) |
| **C++ compiler** | **MSVC 19.38** (VS 2022 17.8) / **GCC 12.3** / **Apple Clang 15.0** (Xcode 15.2) | C++20 required |
| **Qt** | **6.8.3** | [Qt Online Installer](https://www.qt.io/download-qt-installer) |
| **Python** | **3.11.9** | [python.org](https://www.python.org/downloads/release/python-3119/) |
| **Platform SDK** | Win10 SDK 10.0.22621.0 / macOS SDK 14.0 (deploy 11.0+) / glibc 2.31+ | — |

#### Build (using CMake presets — recommended)

```bash
git clone https://github.com/QuantPilot/QuantPilot.git
cd QuantPilot/quantpilot-qt
```

**Step 1 — Configure** (one-time, or after `CMakeLists.txt` changes):
```powershell
cmake --preset win-release      # Windows (PowerShell)
cmake --preset linux-release    # Linux
cmake --preset macos-release    # macOS
```

**Step 2 — Compile** (run this for every code change):
```powershell
cmake --build --preset win-release      # Windows
cmake --build --preset linux-release    # Linux
cmake --build --preset macos-release    # macOS
```

---

## License

> ⚠️ **Cloning, forking, or modifying this repository does NOT grant commercial rights.**
> A paid Commercial License is required for **any** business or internal company use — including forks that remove or replace APIs with your own data sources. See **[Commercial License](https://github.com/QuantPilot/QuantPilot/blob/main/docs/COMMERCIAL_LICENSE.md)** for binding terms.

**Dual Licensed: AGPL-3.0 (Open Source) + Commercial License**

| | |
|---|---|
| ✅ **Free under AGPL-3.0** | Personal use · Individual learning · Academic research · Open-source contributions to this repository |
| ❌ **Commercial License required** | Any business use (paid or free) · Internal company use · Startups at any stage · Hedge funds, brokerages, banks, fintechs · SaaS / hosted offerings · White-label or reselling |

© 2025–2026 QuantPilot AI. All rights reserved.
|---|---|
