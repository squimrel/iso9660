#!/bin/sh
readonly VERSION="$(git describe --abbrev=0 --tags 2>/dev/null || echo 0)"
echo "${VERSION}" | tr -d "\n"
