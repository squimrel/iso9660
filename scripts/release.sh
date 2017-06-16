#!/bin/sh

cd "$(git rev-parse --show-toplevel)"

readonly NAME="iso9660io"
readonly USER="$(git config user.name)"
VERSION="${1}"
if [ -n "${VERSION}" ]; then
  readonly VERSION
  git tag -s "${VERSION}" -m "Sign ${VERSION}" &&
  git push --tags ||
  exit 1
else
  readonly VERSION="$(sh ./scripts/version.sh)"
fi

if [ -d ./spec/ ]; then
  cd ./spec/
  fedpkg pull
  cd -
else
  fedpkg clone "${NAME}" ./spec/
fi

readonly SPECFILE="./spec/${NAME}.spec"
if [ -f "${SPECFILE}" ]; then
  if grep -q "^Version: ${VERSION}$" "${SPECFILE}"; then
    (>&2 echo ".spec file uses the up-to-date version of ${NAME}.")
    exit 1
  fi

  readonly CHANGELOG="$(sed "1,/^%changelog$/d" "${SPECFILE}")"
fi

update_spec() {
  sed -e "/@DESCRIPTION@/{r./description.txt" -e "d}" ./iso9660io.spec.in | \
    sed "s/@VERSION@/${VERSION}/g"

  pushd ./spec/ >/dev/null
  echo "* $(date +"%a %b %d %Y") ${USER} <$(git config user.email)> ${VERSION}-1"
  echo "- Update to version ${VERSION}"
  echo
  popd >/dev/null
  if [ -n "${CHANGELOG}" ]; then
    echo "${CHANGELOG}"
  fi
}

update_spec > "${SPECFILE}"

readonly TARBALL="${VERSION}.tar.gz"
if [ ! -f "${TARBALL}" ]; then
  spectool -g "${SPECFILE}"
fi
sha256sum --tag "${TARBALL}" > ./spec/sources

cd ./spec/
fedpkg commit -F clog
fedpkg mockbuild
# Let the user double-check and then push everything himself.
