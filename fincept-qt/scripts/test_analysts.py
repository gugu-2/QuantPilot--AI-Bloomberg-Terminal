import sys
import os
from datetime import datetime
import traceback

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from tradingagents.agents import (
    create_market_analyst,
    create_sentiment_analyst,
    create_news_analyst,
    create_fundamentals_analyst
)
from tradingagents.llm_clients import create_llm_client
from tradingagents.default_config import DEFAULT_CONFIG

def main():
    ticker = "AAPL"
    date_str = datetime.now().strftime("%Y-%m-%d")
    
    print(f"Testing analysts for {ticker} on {date_str}...\n")
    
    config = DEFAULT_CONFIG.copy()
    # Use standard models for testing
    config["quick_think_llm"] = "gpt-4o"
    
    try:
        client = create_llm_client(
            provider="openai",
            model=config["quick_think_llm"]
        )
        llm = client.get_llm()
    except Exception as e:
        print(f"Failed to initialize LLM: {e}")
        return

    analysts = {
        "Fundamentals": create_fundamentals_analyst(llm),
        "Sentiment": create_sentiment_analyst(llm),
        "News": create_news_analyst(llm),
        "Technical (Market)": create_market_analyst(llm)
    }

    state = {
        "messages": [],
        "company_of_interest": ticker,
        "trade_date": date_str,
        "market_report": "",
        "sentiment_report": "",
        "news_report": "",
        "fundamentals_report": ""
    }

    for name, analyst_func in analysts.items():
        print(f"\n{'='*50}")
        print(f"Testing {name} Analyst...")
        print(f"{'='*50}")
        
        try:
            # We are not testing tool calling here for market, news, fundamentals
            # because they rely on the LangGraph ToolNode to actually execute tools.
            # But we can check if they successfully return tool calls or a report!
            result = analyst_func(state)
            
            # For Sentiment analyst, it doesn't use tools, so it returns the full report.
            # For others, they should return tool calls.
            messages = result.get("messages", [])
            last_msg = messages[-1] if messages else None
            
            if last_msg and hasattr(last_msg, "tool_calls") and last_msg.tool_calls:
                print(f"✅ SUCCESS: {name} Analyst generated {len(last_msg.tool_calls)} tool calls.")
                for tc in last_msg.tool_calls:
                    print(f"   - Tool: {tc['name']}")
            else:
                report_key = ""
                if name == "Fundamentals": report_key = "fundamentals_report"
                elif name == "Sentiment": report_key = "sentiment_report"
                elif name == "News": report_key = "news_report"
                elif name == "Technical (Market)": report_key = "market_report"
                
                report = result.get(report_key, "")
                print(f"✅ SUCCESS: {name} Analyst generated a direct report ({len(report)} chars).")
                print("\nReport excerpt:")
                print(report[:500] + "...\n")
                
        except Exception as e:
            print(f"❌ ERROR testing {name} Analyst:")
            traceback.print_exc()

if __name__ == "__main__":
    main()
