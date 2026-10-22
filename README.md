# Blind evaluation of the LibrePCB MCP server

Evidence for the pull request that adds the MCP server `librepcb-mcp`. This
branch is not meant to be merged. During the six main runs the server was still
the subcommand `librepcb-cli mcp`. The final run (`555_v_*`, `555_final_run.md`)
used `librepcb-mcp` from the pull request. The harness starts either one.

- `img/`: final boards (realistic and layer renders) and schematics of every run,
  rendered through the MCP tools themselves. `gf_0_original.png` is the original
  gfroerli board for comparison.
- `reports/`: the unedited final reports of the six blind agents.
- `briefs/`: the design briefs and the exact prompt every agent got.
- `harness/`: the relay (`mcpd.py`) that kept one `librepcb-cli mcp` process per
  run and logged every call, and the client (`mcp_call.py`) the agents used.

Each agent ran in its own directory with a fresh workspace, the 39 official
libraries pre-installed, no internet and no access to the LibrePCB sources.
