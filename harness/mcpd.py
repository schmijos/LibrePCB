"""Keep one `librepcb-cli mcp` process running and relay tool calls from a FIFO.

Usage: mcpd.py <librepcb-cli> <call log>
"""
import base64
import json
import os
import subprocess
import sys
import time

here = os.path.dirname(os.path.abspath(__file__))
req_path = os.path.join(here, "mcp.req")
resp_path = os.path.join(here, "mcp.resp")
img_dir = os.path.join(here, "images")
os.makedirs(img_dir, exist_ok=True)
cli, log_path = sys.argv[1], sys.argv[2]
server = subprocess.Popen([cli] + (["mcp"] if cli.endswith("librepcb-cli") else []), stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          text=True, bufsize=1)
next_id = 0


def rpc(method, params):
    global next_id
    next_id += 1
    server.stdin.write(json.dumps({"jsonrpc": "2.0", "id": next_id, "method": method,
                                   "params": params}) + "\n")
    server.stdin.flush()
    return json.loads(server.stdout.readline())


def log(entry):
    with open(log_path, "a") as f:
        f.write(json.dumps(entry) + "\n")


rpc("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                   "clientInfo": {"name": "mcpd", "version": "1"}})
for path in (req_path, resp_path):
    if os.path.exists(path):
        os.remove(path)
    os.mkfifo(path)
image_counter = 0
while True:
    with open(req_path) as f:
        data = f.readline()
    req = json.loads(data)
    start = time.time()
    is_error = False
    if req["method"] == "tools/list":
        resp = rpc("tools/list", {})
        out = "\n\n".join(f"{t['name']}: {t['description']}\n  input: {json.dumps(t['inputSchema'])}"
                          for t in resp["result"]["tools"])
    else:
        resp = rpc("tools/call", {"name": req["name"], "arguments": req["arguments"]})
        if "error" in resp:
            is_error = True
            out = f"PROTOCOL ERROR: {resp['error']['message']}"
        else:
            parts = []
            for c in resp["result"]["content"]:
                if c["type"] == "image":
                    image_counter += 1
                    fp = os.path.join(img_dir, f"{image_counter:03d}_{req['name']}.png")
                    open(fp, "wb").write(base64.b64decode(c["data"]))
                    parts.append(f"[PNG image saved to {fp} - view it with the Read tool]")
                else:
                    parts.append(c["text"])
            is_error = bool(resp["result"].get("isError"))
            out = ("TOOL ERROR: " if is_error else "") + "\n".join(parts)
    log({"t": round(start, 3), "dur": round(time.time() - start, 3),
         "name": req.get("name", "tools/list"), "args": req.get("arguments"),
         "error": is_error, "out": out[:4000]})
    with open(resp_path, "w") as f:
        f.write(out + "\n")
