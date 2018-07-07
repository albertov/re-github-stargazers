#!/usr/bin/env bash

set -eu

ARGS=$(printf "%q"" " "$@")

mkdir -p .build-home
cat > .build-home/.tmux.conf <<EOF
  set-option -g prefix C-a
  bind-key C-a last-window
EOF

exec docker run --rm -it \
  -v $(pwd)/.build-home:/home/node \
  -v $(pwd):/home/node/src \
  reason $ARGS
