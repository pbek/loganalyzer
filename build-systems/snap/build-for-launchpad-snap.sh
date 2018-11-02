#!/usr/bin/env bash
#
# This is the build/deploy script for the Launchpad Snap image
# https://launchpad.net/loganalyzer-snap
#
# We need a ssh key to push to the git repository
# git+ssh://pbek@git.launchpad.net/~pbek/loganalyzer-snap
#


# uncomment this if you want to force a version
#LOGANALYZER_VERSION=16.07.6

BRANCH=develop
#BRANCH=master

PROJECT_PATH="/tmp/LogAnalyzer-launchpad-snap-$$"
CUR_DIR=$(pwd)


echo "Started the Launchpad Snap packaging process, using latest '$BRANCH' git tree"

if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi

mkdir $PROJECT_PATH
cd $PROJECT_PATH

echo "Project path: $PROJECT_PATH"

# checkout Launchpad Snap repository
git clone --depth=5 git+ssh://pbek@git.launchpad.net/~pbek/loganalyzer-snap snap

# checkout the source code
git clone --depth=5 git@github.com:pbek/LogAnalyzer.git LogAnalyzer -b $BRANCH
cd LogAnalyzer

if [ -z $LOGANALYZER_VERSION ]; then
    # get version from version.h
    LOGANALYZER_VERSION=`cat src/version.h | sed "s/[^0-9,.]//g"`
fi

cd ../snap
cp ../LogAnalyzer/build-systems/snap/snapcraft/* . -R

# replace the version in the snapcraft.yaml file
sed -i "s/VERSION-STRING/$LOGANALYZER_VERSION/g" snapcraft.yaml


echo "Committing changes..."
git commit -m "releasing version $LOGANALYZER_VERSION" -a
git push

# remove everything after we are done
if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi
