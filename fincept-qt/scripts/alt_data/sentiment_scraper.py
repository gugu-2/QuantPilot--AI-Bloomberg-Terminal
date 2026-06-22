"""Real sentiment scraper using StockTwits + Reddit public APIs.

Replaces the previous mock-data implementation. No API keys required —
StockTwits exposes a public symbol stream and Reddit has public JSON search
endpoints. Both degrade gracefully on network failure.

Output format uses the __fincept_data__: prefix required by
PythonRunner::extract_json() in the C++ backend.
"""
import sys
import os
import json

# Ensure tradingagents dataflows are importable from this script location
_script_dir = os.path.dirname(os.path.abspath(__file__))
_tradingagents_root = os.path.abspath(os.path.join(_script_dir, "..", "tradingagents"))
sys.path.insert(0, os.path.abspath(os.path.join(_script_dir, "..")))


def _emit(payload: dict):
    print("\n__fincept_data__:" + json.dumps(payload), flush=True)


def main():
    if len(sys.argv) < 2:
        _emit({"error": "Missing ticker argument"})
        return

    ticker = sys.argv[1].upper()

    # --- StockTwits ---
    stocktwits_summary = "<stocktwits unavailable>"
    stocktwits_bullish = 0
    stocktwits_bearish = 0
    stocktwits_total = 0
    try:
        from tradingagents.dataflows.stocktwits import fetch_stocktwits_messages
        raw = fetch_stocktwits_messages(ticker, limit=30)
        stocktwits_summary = raw[:500] if len(raw) > 500 else raw
        # Parse counts from the summary line: "Bullish: N (X%) · Bearish: M (Y%) ..."
        if raw and not raw.startswith("<"):
            first_line = raw.split("\n")[0]
            import re
            bull_m = re.search(r"Bullish:\s*(\d+)", first_line)
            bear_m = re.search(r"Bearish:\s*(\d+)", first_line)
            total_m = re.search(r"Total:\s*(\d+)", first_line)
            stocktwits_bullish = int(bull_m.group(1)) if bull_m else 0
            stocktwits_bearish = int(bear_m.group(1)) if bear_m else 0
            stocktwits_total = int(total_m.group(1)) if total_m else 0
    except Exception as e:
        stocktwits_summary = f"<stocktwits error: {e}>"

    # --- Reddit ---
    reddit_summary = "<reddit unavailable>"
    reddit_post_count = 0
    try:
        from tradingagents.dataflows.reddit import fetch_reddit_posts
        raw_reddit = fetch_reddit_posts(ticker)
        reddit_summary = raw_reddit[:500] if len(raw_reddit) > 500 else raw_reddit
        if not raw_reddit.startswith("<"):
            import re
            counts = re.findall(r"(\d+) recent posts", raw_reddit)
            reddit_post_count = sum(int(c) for c in counts)
    except Exception as e:
        reddit_summary = f"<reddit error: {e}>"

    # --- Compute aggregate sentiment score ---
    if stocktwits_total > 0:
        score = round((stocktwits_bullish - stocktwits_bearish) / stocktwits_total, 3)
    else:
        score = 0.0

    if score > 0.2:
        sentiment = "BULLISH"
    elif score < -0.2:
        sentiment = "BEARISH"
    else:
        sentiment = "NEUTRAL"

    payload = {
        "module": "sentiment",
        "ticker": ticker,
        "score": score,
        "sentiment": sentiment,
        "stocktwits_bullish": stocktwits_bullish,
        "stocktwits_bearish": stocktwits_bearish,
        "stocktwits_total": stocktwits_total,
        "reddit_posts": reddit_post_count,
        "stocktwits_summary": stocktwits_summary,
        "reddit_summary": reddit_summary,
        "source": "StockTwits API + Reddit Public Search"
    }

    _emit(payload)


if __name__ == "__main__":
    main()
