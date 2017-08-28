#!/bin/sh
# If the first argument is not a revision it'll release a new version.
# In any case it packages the project at revision specified as if it's a
# package of the latest version.

readonly RELEASE="f$(rpm -E "%fedora")"
readonly ROOT="$(git rev-parse --show-toplevel)"
cd "${ROOT}"

readonly NAME="iso9660io"
readonly SUMMARY="ISO 9660 manipulation using C++11"
TAG="${1}"
REVISION="${TAG}"

if [ -n "${TAG}" ] && ! git rev-parse "${TAG}" &>/dev/null; then
  readonly TAG
  readonly REVISION
  if [ "${2}" != "release" ]; then
    read  -n 1 -p "Press enter to release version ${TAG}.."
    echo "Press Ctrl-C in the next two seconds to abort the release of version ${TAG}."
    sleep 2
  fi
  git tag -s "${TAG}" -m "Sign ${VERSION}" &&
  git push --tags || exit 1
else
  readonly TAG="$(sh ./scripts/version.sh)"
  if [ -z "${TAG}" ]; then
    REVISION="HEAD"
  fi
  readonly REVISION
fi
readonly VERSION="$(echo "${TAG}" | cut -d "-" -f2-)"
readonly TARBALL="${TAG}.tar.gz"
readonly ARCHIVE="${ROOT}/build/${TARBALL}"
git archive --format=tar.gz "--prefix=${TAG}/" "${REVISION}" \
  > "${ARCHIVE}"

update_spec() {
  package="${1}"
  sed -e "/@DESCRIPTION@/{r${ROOT}/description.txt" -e "d}" \
    "${ROOT}/${package}.spec.in" | sed "s/@VERSION@/${VERSION}/g" | \
    sed "s/@SUMMARY@/${SUMMARY}/g" | sed "s/%{shortname}/${NAME}/g"

  pushd "${package}" >/dev/null
  echo "* $(date +"%a %b %d %Y") $(git config user.name) <$(git config user.email)> ${VERSION}-1"
  echo "- Update to version ${VERSION}"
  echo
  popd >/dev/null
  if [ -n "${changelog}" ]; then
    echo "${changelog}"
  fi
}

update_package() {
  package="${1}"
  if [ -d "${package}" ]; then
    pushd "${package}" >/dev/null
    fedpkg pull 2>/dev/null || echo "pull failed."
    popd >/dev/null
  else
    echo
    if ! fedpkg clone "${package}" 2>/dev/null; then
      mkdir -p "${package}"
      pushd "${package}" >/dev/null
      git init
      popd >/dev/null
      echo "clone failed."
    fi
  fi

  specfile="./${package}/${package}.spec"
  if [ -f "${specfile}" ]; then
    if grep -q "^Version: ${VERSION}$" "${specfile}"; then
      (>&2 echo "${package}.spec file uses the up-to-date version of ${NAME}.")
      return
    fi
    changelog="$(sed "1,/^%changelog$/d" "${specfile}")"
  fi

  update_spec "${package}" > "${specfile}"

  if [ ! -f "${TARBALL}" ]; then
    spectool -g "${specfile}" || cp "${ARCHIVE}" "${TARBALL}"
    ls
  fi
  sha256sum --tag "${TARBALL}" > ./${package}/sources
  fedpkg commit -F clog 2>/dev/null || echo "commit failed."
}

build_package() {
  package="${1}"
  pushd "${package}" >/dev/null
  # Use local archive for build.
  cp "${ARCHIVE}" .
  sha256sum --tag "${TARBALL}" > sources
  fedpkg --release "${RELEASE}" local
  popd >/dev/null
}

mkdir -p ./package/
cd ./package/

update_package "${NAME}"
update_package "mingw-${NAME}"

build_package "${NAME}"
build_package "mingw-${NAME}"
