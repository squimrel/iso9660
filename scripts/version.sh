#!/bin/sh
readonly VERSION="$(git describe --abbrev=0 --tags 2>/dev/null || echo 0)"
echo "${VERSION}" | cut -d "-" -f2 | tr -d "\n"
