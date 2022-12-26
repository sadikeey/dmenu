dmenu - dynamic menu
====================
dmenu is an efficient dynamic menu for X.


Requirements
------------
In order to build dmenu you need the Xlib header files.

- On Debian/Ubuntu
```sh
sudo apt install build-essential libx11-dev libxft-dev libxinerama-dev libfreetype6-dev libfontconfig1-dev
```

- On Arch Linux

```sh
sudo pacman -S base-devel libx11 libxft libxinerama freetype2 fontconfig
```

- On Fedora/RHEL

```sh
sudo dnf install make cmake gcc libX11-devel libXft-devel libXinerama-devel libXrandr-devel
```

- On Void Linux(case sensitive)

```sh
sudo xbps-install base-devel libX11-devel libXft-devel libXinerama-devel freetype-devel fontconfig-devel
```

Installation
------------
Edit config.mk to match your local setup (dmenu is installed into
the /usr/local namespace by default).

Afterwards enter the following command to build and install dmenu
(if necessary as root):

    make clean install


Running dmenu
-------------
See the man page for details.
