# LogAnalyzer Installation

## Install on Ubuntu Linux (15.04 and newer)

You can add the Ubuntu Launchpad repository and install LogAnalyzer with these 
commands:

```bash
sudo add-apt-repository ppa:pbek/loganalyzer
sudo apt-get update
sudo apt-get install loganalyzer
```

## Install as snap

You can install the LogAnalyzer snap by typing: `snap install loganalyzer-pbek` and then run it with `loganalyzer-pbek`.

[Snaps](https://snapcraft.io) are working on many Linux distributions like Ubuntu, Arch Linux, Debian, Fedora, openSUSE, Gentoo Linux, OpenWRT, open embedded and yocto project.

## Install on Debian Linux

### Debian 8.0

Run the following shell commands trust the repository.

```bash
wget http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Debian_8.0/Release.key -O - | sudo apt-key add -
```

Run the following shell commands to add the repository and install LogAnalyzer from there.

```bash
sudo echo 'deb http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Debian_8.0/ /' >> /etc/apt/sources.list.d/loganalyzer.list  
sudo apt-get update  
sudo apt-get install loganalyzer
```

If you use this repository for other Debian Linux versions please make sure 
that you have **Qt** installed at least at **version 5.3**.

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=Debian_8.0)


## Install on openSUSE Linux

### openSUSE 13.2

Run the following shell commands as root to trust the repository.

```bash
su -  
rpm --import http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_13.2/repodata/repomd.xml.key
```

Run the following shell commands as root to add the repository and install LogAnalyzer from there.

```bash
zypper addrepo -f http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_13.2/home:pbek:LogAnalyzer.repo  
zypper refresh  
zypper install loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=openSUSE_13.2)

### openSUSE Leap 42.1

Run the following shell commands as root to trust the repository.

```bash
su -  
rpm --import http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_Leap_42.1/repodata/repomd.xml.key
```

Run the following shell commands as root to add the repository and install LogAnalyzer from there.

```bash
zypper addrepo -f http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_Leap_42.1/home:pbek:LogAnalyzer.repo  
zypper refresh  
zypper install loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=openSUSE_Leap_42.1)

### openSUSE Tumbleweed

Run the following shell commands as root to trust the repository.

```bash
su -  
rpm --import http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_Tumbleweed/repodata/repomd.xml.key
```

Run the following shell commands as root to add the repository and install LogAnalyzer from there.

```bash
zypper addrepo -f http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_Tumbleweed/home:pbek:LogAnalyzer.repo  
zypper refresh  
zypper install loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=openSUSE_Tumbleweed)


## Install on Fedora Linux

### Fedora 23

Run the following shell commands as root to trust the repository.

```bash
su -  
rpm --import http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Fedora_23/repodata/repomd.xml.key
```

Run the following shell commands as root to add the repository and install LogAnalyzer from there.

```bash
wget http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Fedora_23/home:pbek:LogAnalyzer.repo -O /etc/yum.repos.d/LogAnalyzer.repo  
dnf clean expire-cache  
dnf install loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=Fedora_23)

### Fedora 22

Run the following shell commands as root to trust the repository.

```bash
su -  
rpm --import http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Fedora_22/repodata/repomd.xml.key
```

Run the following shell commands as root to add the repository and install LogAnalyzer from there.

```bash
wget http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Fedora_22/home:pbek:LogAnalyzer.repo -O /etc/yum.repos.d/LogAnalyzer.repo  
dnf clean expire-cache  
dnf install loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=Fedora_22)


## Install on Arch Linux

### pacman

Add the following lines to your `/etc/pacman.conf` with `sudo nano /etc/pacman.conf`:

```bash
[home_pbek_LogAnalyzer_Arch_Extra]  
SigLevel = Optional TrustAll  
Server = http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/Arch_Extra/$arch
```

Run the following shell commands to trust the repository:

```bash
wget http://download.opensuse.org/repositories/home:/pbek:/LogAnalyzer/openSUSE_13.2/repodata/repomd.xml.key -O - | sudo pacman-key --add -  
sudo pacman-key --lsign-key FFC43FC94539B8B0
```

Synchronize your package database and install the package with `pacman`:

```bash
sudo pacman -Syy loganalyzer
```

[Direct Download](https://build.opensuse.org/package/binaries/home:pbek:LogAnalyzer/desktop?repository=Arch_Extra)

Of course you can also use this repository with other Arch Linux based distributions, like Manjaro.


## Install on OS X

You will find releases for OS X at
[LogAnalyzer releases](https://github.com/pbek/loganalyzer/releases).
Look for the latest file named `LogAnalyzer-<version-number>.dmg`.


## Install on Microsoft Windows

You will find releases for Windows at
[LogAnalyzer releases](https://github.com/pbek/loganalyzer/releases).
Look for the latest file named `LogAnalyzer.zip`.
