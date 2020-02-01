#!/bin/bash -e

# Compatibility with Travis-CI and GitLab-CI
if [ ! -z ${TRAVIS_BUILD_DIR} ]; then
        export BUILD_DIR=${TRAVIS_BUILD_DIR}
else
        export BUILD_DIR="$(dirname "$(readlink -f "${0}")")/../../"
fi

export CPUS_USED=2
if command -v nproc >/dev/null; then
	export CPUS_USED=$(nproc)
fi

if [[ ${PLATFORM} == "ubuntu-touch" ]]; then
	export BUILD_SYSTEM="cmake"
elif [[ ${PLATFORM} == "" ]]; then
	# currently there's only linux-desktop & ut
	# otherwise other parameters (as TRAVIS_OS_NAME) could be checked
	export PLATFORM="linux-desktop"
fi

echo_env() {
	echo "PLATFORM=${PLATFORM}"
	echo "BUILD_SYSTEM=${BUILD_SYSTEM}"
	echo "CPUS_USED=${CPUS_USED}"
	echo "BUILD_DIR=$(readlink -f ${BUILD_DIR})"
}
