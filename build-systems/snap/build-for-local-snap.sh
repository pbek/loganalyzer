#!/usr/bin/env bash
#
# This is the build script for a local loganalyzer snap image and upload to the
# store
# https://myapps.developer.ubuntu.com/dev/click-apps/
#
# We need to be logged in to ubuntu one with:
# snapcraft login
#


# uncomment this if you want to force a version
#LOGANALYZER_VERSION=16.07.3

BRANCH=develop
#BRANCH=master

PROJECT_PATH="/tmp/LogAnalyzer-local-snap-$$"
CUR_DIR=$(pwd)


echo "Started the Snap building process, using latest '$BRANCH' git tree"

if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi

mkdir $PROJECT_PATH
cd $PROJECT_PATH

echo "Project path: $PROJECT_PATH"

# checkout the source code
git clone --depth=5 https://github.com/pbek/loganalyzer.git LogAnalyzer -b $BRANCH
cd LogAnalyzer

if [ -z $LOGANALYZER_VERSION ]; then
    # get version from version.h
    LOGANALYZER_VERSION=`cat src/version.h | sed "s/[^0-9,.]//g"`
fi

cd build-systems/snap/snapcraft

# replace the version in the snapcraft.yaml file
sed -i "s/VERSION-STRING/$LOGANALYZER_VERSION/g" snapcraft.yaml

snapcraft update

echo "Building snap..."
snapcraft

#echo "Uploading snap..."
#snapcraft push loganalyzer_${LOGANALYZER_VERSION}_amd64.snap --release stable

#echo "Releasing snap..."
# snapcraft release loganalyzer --release ${REVISION} stable

# this may work in the future (we need to set a channel when releasing)
#snapcraft push loganalyzer_${LOGANALYZER_VERSION}_amd64.snap --release=Stable
#snapcraft release <snap-name> <revision> <channel-name>

# remove everything after we are done
if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi
