#!/usr/bin/env bash

export PATH=~/.platformio/penv/bin/:$PATH

set -e

UPLOAD="pio run -e esp32-2432S028Rv3 -t nobuild -t upload --upload-port"
MONITOR="pio device monitor --filter esp32_exception_decoder --port"

PORT1=/dev/ttyUSB0
PORT2=/dev/ttyUSB1

pio run -e esp32-2432S028Rv3

# Start a new tmux session and detach from it
tmux new-session -d -s mysession
tmux set -g mouse on

# Split the window into two horizontal panes
tmux split-window -h

# Optionally, you can adjust the size of the panes if needed:
# tmux resize-pane -D 10  # Resize the current pane down by 10 rows.

# Run commands in the first pane
tmux send-keys -t mysession:0.0 "${UPLOAD} ${PORT1} && ${MONITOR} ${PORT1}" C-m
tmux send-keys -t mysession:0.0 'exit' C-m

# Run commands in the second pane
tmux send-keys -t mysession:0.1 "${UPLOAD} ${PORT2} && ${MONITOR} ${PORT2}" C-m
tmux send-keys -t mysession:0.1 'exit' C-m

# Attach to the session
tmux attach-session -t mysession

# # This part checks if any pane has exited and closes the session if so
tmux set-option -t mysession pane-dead-status 'dead'
tmux set-hook -t mysession pane-died 'kill-session'
