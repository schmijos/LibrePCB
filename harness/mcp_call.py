"""Call one LibrePCB MCP tool.

Usage:
  mcp_call.py --list                 List all tools with their input schema
  mcp_call.py <tool> '<json args>'   Call a tool, e.g. mcp_call.py parts_list '{}'
  mcp_call.py <tool> @args.json      Read the arguments from a JSON file
"""
import fcntl
import json
import os
import sys

here = os.path.dirname(os.path.abspath(__file__))
if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
    print(__doc__)
    sys.exit(1)
if sys.argv[1] == "--list":
    req = {"method": "tools/list"}
else:
    raw = sys.argv[2] if len(sys.argv) > 2 else "{}"
    if raw.startswith("@"):
        raw = open(raw[1:]).read()
    req = {"method": "tools/call", "name": sys.argv[1], "arguments": json.loads(raw)}
with open(os.path.join(here, "mcp.lock"), "w") as lock:
    fcntl.flock(lock, fcntl.LOCK_EX)
    with open(os.path.join(here, "mcp.req"), "w") as f:
        f.write(json.dumps(req) + "\n")
    with open(os.path.join(here, "mcp.resp")) as f:
        print(f.read().rstrip())
