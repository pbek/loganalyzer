{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = [
      pkgs.buildPackages.qt5.qtbase
      pkgs.buildPackages.qt5.qmake
      pkgs.buildPackages.qt5.qttools
      pkgs.buildPackages.qt5.wrapQtAppsHook
    ];
}

