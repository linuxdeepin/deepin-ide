# deepin-ide

[![zh](https://img.shields.io/badge/lang-zh-red.svg)](./README.md)

deepin-ide is a lightweight integrated development environment, featured with multilingual and cross platform compatibility.

## Build dependencies

_The **dev** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

-  debhelper (>=9),
-  cmake,
-  qt5-qmake,
-  qtbase5-dev,
-  qttools5-dev,
-  qttools5-dev-tools,
-  lxqt-build-tools (>= 0.6.0~),
-  libssl-dev,
-  llvm (>=1:7~),
-  llvm-dev (>=1:7~),
-  libclang-dev (>=1:7~),
-  libutf8proc-dev,
-  libmicrohttpd-dev,
-  libjsoncpp-dev,
-  libargtable2-dev,
-  libhiredis-dev,
-  catch,
-  libzstd-dev,
-  libjson-c-dev,
-  libelf-dev,
-  libcapstone-dev,
-  libunwind-dev, 
-  libelfin-dev,
-  libdbus-1-dev,
-  libxi-dev,
-  qtscript5-dev,
-  libqt5scripttools5,
-  clang[!mips64],
-  doxygen,
-  libdtkgui-dev,
-  libdtkwidget-dev,
-  libdtkcore-dev,
-  libdtkcore5-bin,
-  libkf5syntaxhighlighting-dev

## Installation

   *If you install the IDE on Ubuntu by source code, please refer to the [DTK dependent installation](./docs/dtk-install-guide.en.md). After the installation of the dtk environment and the basic qt environment, you can install the IDE through the following source installation method.*

1. Make sure you have installed all dependencies.

``` shell
$ git clone https://github.com/linuxdeepin/deepin-ide.git
(gitee: $ git clone https://gitee.com/deepin-community/deepin-ide.git)
$ cd deepin-ide
$ sudo apt build-dep ./
```

2. Build:

```shell
$ cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
```

3. Install:

```shell
$ sudo cmake --build build --target install
```

The executable binary file could be found at `/usr/bin/deepin-ide`

## Usage

Execute `deepin-ide`

*Attention: After the installation of deepin-unicode, some features require the installation of dependency packages before they can be used normally.* 

## Supported Language Project

| Language   | Project Type      |
| ---------- | ----------------- |
| C/C++      | CMake             |
| Java       | Gradle、Maven     |
| Python     | Project Direcotry |
| JavaScript | Project Directory |

## Developed applications

| Application name    | Description                                         | Github Address                                               |
| ------------------- | --------------------------------------------------- | ------------------------------------------------------------ |
| deepin-draw         | A lightweight drawing tool for users to freely draw | https://github.com/linuxdeepin/deepin-draw<br />(https://gitee.com/deepin-community/deepin-draw.git) |
| deepin-image-viewer | Image viewing tool                                  | https://github.com/linuxdeepin/deepin-image-viewer<br />(https://gitee.com/deepin-community/deepin-image-viewer.git) |
| deepin-album        | A photo management tool                             | https://github.com/linuxdeepin/deepin-album<br />(https://gitee.com/deepin-community/deepin-album.git) |

## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Extended guide](./docs/extended-guide.md)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

deepin-ide is licensed under [GPL-3.0-or-later](LICENSE)

## 3rdparty support

| Third-party Name | Version    | Application location  | License     |
| ---------------- | ---------- | --------------------- | ----------- |
| cppdap           | NA         | C++调试适配器协议 SDK | Apache 2.0  |
| QScintilla       | 2.14       | 智能编辑器界面组件    | GPLv3       |
| lexilla          | 5.14       | 智能编辑器词法分析库  | HPND        |
| qtermwidget      | 0.14.1     | 终端组件库            | GPLV2+ BSD3 |
| GitQlient        | tag/v1.4.3 | Git展示客户端         | LGPL-2.1+   |
| json-rpc-cpp     | 1.4.1      | 通信中间件            | MIT         |
| DTK              | 5.6.8.9    | 界面显示              | LGPLv3+     |


## Preview

- language support

  ![](./docs/rc/language-support.png)

- code porting

  ![](./docs/rc/code-porting.png)

- debug mode

  ![](./docs/rc/debug-mode.png)

- code translation

  ![](./docs/rc/codegeex-translate.png)

- AI answer

  ![codegeex-answer](./docs/rc/codegeex-answer.png)
