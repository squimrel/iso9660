#!/bin/sh
# Inline all local includes to create a single header file.
readonly SOURCE="${1}"
readonly DESTINATION="${2}"
readonly INCLUDE='^#include "./include/'

include_files() {
  filename="${1}"
  echo "${filename}"
  includes=$(cat "${filename}" | grep "${INCLUDE}" | cut -d '"' -f2 | tr "\n" " ")
  for header in ${includes}; do
    include_files "${header}"
  done
}
order_uniq() {
  tac | cat -n | sort -k2 -k1n  | uniq -f1 | sort -nk1,1 | cut -f2-
}
cat $(include_files "${SOURCE}" | order_uniq) | grep -v "${INCLUDE}" \
  > "${DESTINATION}"
