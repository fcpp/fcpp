#!/bin/bash
set -e
cd "$1"
shift 1
exec "$@"
