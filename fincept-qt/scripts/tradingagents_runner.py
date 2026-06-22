import sys
import json
import datetime
import traceback
import os
import threading

# Ensure the local scripts directory is in path so we can import tradingagents
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from tradingagents.graph.trading_graph import TradingAgentsGraph
from tradingagents.default_config import DEFAULT_CONFIG

# Hard wall-clock timeout for the full agent debate (10 minutes).
# LangGraph debates can run 2-5+ min; this prevents the C++ PythonRunner
# from waiting forever or having to kill the process mid-debate.
TIMEOUT_SECONDS = 600

def _emit(payload: dict):
    """Emit a JSON payload in the format expected by PythonRunner::extract_json()."""
    print("\n__fincept_data__:" + json.dumps(payload), flush=True)

def _timeout_handler():
    """Called by threading.Timer if the debate takes too long."""
    _emit({"error": f"TradingAgents debate exceeded {TIMEOUT_SECONDS}s wall-clock timeout."})
    os._exit(1)

def main():
    if len(sys.argv) < 2:
        _emit({"error": "Missing ticker argument"})
        return

    ticker = sys.argv[1]
    date_str = datetime.datetime.now().strftime("%Y-%m-%d")

    # Start the hard timeout watchdog
    watchdog = threading.Timer(TIMEOUT_SECONDS, _timeout_handler)
    watchdog.daemon = True
    watchdog.start()

    try:
        config = DEFAULT_CONFIG.copy()

        # Inject the LLM provider from environment if set by Fincept's credential vault
        # e.g. TRADINGAGENTS_LLM_PROVIDER=google, TRADINGAGENTS_DEEP_THINK_LLM=gemini-2.0-flash
        # (these are already handled by default_config._apply_env_overrides)

        # Run graph in non-debug mode to reduce stdout clutter
        ta = TradingAgentsGraph(debug=False, config=config)

        # Propagate runs the full langgraph debate and returns (final_state, decision_dict)
        _, decision = ta.propagate(ticker, date_str)

        # The decision dict typically looks like: {"action": "buy", "quantity": 100, "reasoning": "..."}
        action = str(decision.get("action", "HOLD")).upper()
        quantity = float(decision.get("quantity", 0))
        reasoning = str(decision.get("reasoning", "No reasoning provided"))

        payload = {
            "agent": "QuantPilot_TradingAgents_v1",
            "ticker": ticker.upper(),
            "action": action,
            "quantity": quantity,
            "reasoning": reasoning,
            "date": date_str,
            "status": "proposal"  # HITL: never auto-execute, always requires human approval
        }

        # Emit with the __fincept_data__: prefix so PythonRunner::extract_json() captures it
        _emit(payload)

    except Exception as e:
        err_msg = str(e) + "\n" + traceback.format_exc()
        _emit({"error": err_msg})
        sys.exit(1)
    finally:
        watchdog.cancel()

if __name__ == "__main__":
    main()
