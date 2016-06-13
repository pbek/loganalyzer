#!/usr/bin/env bash
#
# This is the build script for the openSUSE Build Service (OBS)
# https://build.opensuse.org/package/show/home:pbek:LogAnalyzer/desktop
#
# We will need some packages to execute this locally:
# sudo apt-get install osc xz
#
# A file ~/.oscrc will be generated upon first start of osc
#

# uncomment this if you want to force a version
#LOGANALYZER_VERSION=16.04

BRANCH=develop
#BRANCH=master

DATE=$(LC_ALL=C date +'%a, %d %b %Y %T %z')
PROJECT_PATH="/tmp/LogAnalyzer-$$"
CUR_DIR=$(pwd)


echo "Started the OBS source packaging process, using latest '$BRANCH' git tree"

if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi

mkdir $PROJECT_PATH
cd $PROJECT_PATH

echo "Project path: $PROJECT_PATH"

# checkout the source code
git clone --depth=50 git@github.com:pbek/loganalyzer.git LogAnalyzer -b $BRANCH
cd LogAnalyzer

# checkout submodules
git submodule update --init

# remove huge .git folder
rm -Rf .git

if [ -z $LOGANALYZER_VERSION ]; then
    # get version from version.h
    LOGANALYZER_VERSION=`cat src/version.h | sed "s/[^0-9,.]//g"`
else
    # set new version if we want to override it
    echo "#define VERSION \"$LOGANALYZER_VERSION\"" > src/version.h
fi

# set the release string
echo "#define RELEASE \"OBS\"" > src/release.h

# replace the version in the spec file
sed -i "s/VERSION-STRING/$LOGANALYZER_VERSION/g" build-systems/obs/loganalyzer.spec

# replace the version in the PKGBUILD file
sed -i "s/VERSION-STRING/$LOGANALYZER_VERSION/g" build-systems/obs/PKGBUILD

# replace the version in the dsc file
sed -i "s/VERSION-STRING/$LOGANALYZER_VERSION/g" build-systems/obs/loganalyzer.dsc

changelogText="Released version $LOGANALYZER_VERSION"

echo "Using version $LOGANALYZER_VERSION..."

loganalyzerSrcDir="loganalyzer-${LOGANALYZER_VERSION}"

# copy some needed files file
cp LICENSE.txt src
cp README.md src
cp CHANGELOG.md src

# rename the src directory
mv src $loganalyzerSrcDir

changelogPath=build-systems/obs/loganalyzer.bin

# create the changelog file
echo "-------------------------------------------------------------------" > $changelogPath
echo "$DATE - patrizio@bekerle.com" >> $changelogPath
echo "" >> $changelogPath
echo "- $changelogText" >> $changelogPath

cat $changelogPath

# create the Debian changelog file
debChangelogPath=build-systems/obs/debian.changelog
versionPart="$LOGANALYZER_VERSION-1debian"
echo "loganalyzer ($versionPart) debian; urgency=low" > $debChangelogPath
echo "" >> $debChangelogPath
echo "  * $changelogText" >> $debChangelogPath
echo "" >> $debChangelogPath
echo " -- Patrizio Bekerle <patrizio@bekerle.com>  $DATE" >> $debChangelogPath

cat $debChangelogPath

archiveFile="$loganalyzerSrcDir.tar.xz"

# archive the source code
echo "Creating archive $archiveFile..."
tar -cJf $archiveFile $loganalyzerSrcDir

echo "Checking out OBS repository..."

# checkout OBS repository
osc checkout home:pbek:LogAnalyzer desktop

obsRepoPath="home:pbek:LogAnalyzer/desktop"

# remove other archives
echo "Removing old archives..."
cd $obsRepoPath
osc rm *.xz
cd ../..

# copying new files to repository
mv $archiveFile $obsRepoPath
cp build-systems/obs/loganalyzer.bin $obsRepoPath
cp build-systems/obs/loganalyzer.spec $obsRepoPath
cp $debChangelogPath $obsRepoPath
cp build-systems/obs/PKGBUILD $obsRepoPath
cp $loganalyzerSrcDir/debian/control $obsRepoPath/debian.control
cp $loganalyzerSrcDir/debian/copyright $obsRepoPath/debian.copyright
cp $loganalyzerSrcDir/debian/compat $obsRepoPath/debian.compat
cp $loganalyzerSrcDir/debian/rules $obsRepoPath/debian.rules
cp $loganalyzerSrcDir/debian/loganalyzer.install $obsRepoPath/debian.loganalyzer.install
cp build-systems/obs/loganalyzer.dsc $obsRepoPath

cd $obsRepoPath

# add all new files
osc add $archiveFile

# add these files once
#osc add loganalyzer.bin
#osc add loganalyzer.spec
#osc add PKGBUILD
#osc add debian.changelog
#osc add debian.control
#osc add debian.rules
#osc add debian.copyright
#osc add debian.compat
#osc add debian.loganalyzer.install
#osc add loganalyzer.dsc

echo "Committing changes..."

# commit changes
osc commit -m "$changelogText"

# remove everything after we are done
if [ -d $PROJECT_PATH ]; then
    rm -rf $PROJECT_PATH
fi
