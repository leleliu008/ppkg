# ppkg

A portable package manager for Unix-like system.

## Caveats

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- Please do NOT place your own files under `~/.ppkg` directory, as `ppkg` will change files under `~/.ppkg` directory without notice.

- Please do NOT run `ppkg` command in parallell to avoid generating dirty data.

## Two implementations

This software provides two implementations:

1. implemented in `POSIX Shell`, the source code is on `master` branch.
2. implemented in `C`, the source code is on `dev` branch.

In theroy, these two implementations should have the same behaver except for have bugs.

## Manually build packages using this software via GitHub Actions

In this way, you will be liberated from the rut of setting up the build environemt.

In this way, all you need to do is just clicking the buttons and waiting for finishing. After finishing, a url refers to a zip archive will be provided to download.

For more details please refer to <https://github.com/leleliu008/ppkg-package-manually-build>

## Install POSIX-Shell-based ppkg via curl

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
chmod a+x ppkg
./ppkg setup
```

## Install POSIX-Shell-based ppkg via wget

```bash
wget https://cdn.jsdelivr.net/gh/leleliu008/ppkg/ppkg
chmod a+x ppkg
./ppkg setup
```

## Build from C source
|dependency|required?|purpose|
|----|---------|-------|
|[cmake](https://cmake.org/)|required |for generating `build.ninja`|
|[ninja](https://ninja-build.org/)|required |for doing jobs that read from `build.ninja`|
|[pkg-config>=0.18](https://www.freedesktop.org/wiki/Software/pkg-config/)|required|for finding libraries|
||||
|[jansson](https://github.com/akheron/jansson)|required|for parsing and creating JSON.|
|[libyaml](https://github.com/yaml/libyaml/)|required|for parsing formula files whose format is YAML.|
|[libgit2](https://libgit2.org/)|required|for updating formula repositories.|
|[libcurl](https://curl.se/)|required|for http requesting support.|
|[openssl](https://www.openssl.org/)|required|for https requesting support and SHA-256 sum checking support.|
|[libarchive](https://www.libarchive.org/)|required|for uncompressing .zip and .tar.* files.|
|[zlib](https://www.zlib.net/)|required|for compress and uncompress data.|
|[pcre2](https://www.pcre.org/)||for Regular Expressions support. only required on OpenBSD.|


**[vcpkg](https://github.com/microsoft/vcpkg)**
```bash
# install g++ curl zip unzip tar git

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT="$PWD/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"

vcpkg install curl openssl libgit2 libarchive libyaml jansson

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build   build.d
cmake --install build.d
```

**[Ubuntu](https://ubuntu.com/)**

```bash
apt -y update
apt -y install git cmake ninja-build pkg-config gcc libcurl4 libcurl4-openssl-dev libgit2-dev libarchive-dev libyaml-dev libjansson-dev zlib1g-dev

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Fedora](https://getfedora.org/)**

```bash
dnf -y update
dnf -y install git cmake ninja-build pkg-config gcc libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[ArchLinux](https://archlinux.org/)**

```bash
pacman -Syyuu --noconfirm
pacman -S     --noconfirm git cmake ninja pkg-config gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[AlpineLinux](https://www.alpinelinux.org/)**

```bash
apk add git cmake ninja pkgconf gcc libc-dev curl-dev openssl-dev libgit2-dev libarchive-dev yaml-dev jansson-dev zlib-dev

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[VoidLinux](https://voidlinux.org/)**

```bash
xbps-install -Suy xbps
xbps-install -Suy cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Gentoo Linux](https://www.gentoo.org/)**

```bash
emerge dev-vcs/git cmake dev-util/ninja gcc pkg-config net-misc/curl dev-libs/libgit2 libarchive dev-libs/libyaml dev-libs/jansson dev-libs/zlib

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[openSUSE](https://www.opensuse.org/)**

```bash
zypper update  -y  
zypper install -y git cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel libjansson-devel zlib-devel

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[macOS](https://www.apple.com/macos/)**

```bash
brew update
brew install git cmake pkg-config ninja curl jansson libyaml libgit2 libarchive zlib

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/local/opt/openssl@1.1/lib/pkgconfig:/usr/local/opt/curl/lib/pkgconfig:/usr/local/opt/libarchive/lib/pkgconfig"

CMAKE_EXE_LINKER_FLAGS='-L/usr/local/lib -L/usr/local/opt/openssl@1.1/lib -lssl -liconv -framework CoreFoundation -framework Security'
CMAKE_FIND_ROOT_PATH="$(brew --prefix openssl@1.1);$(brew --prefix curl);$(brew --prefix libarchive)"

cmake \
    -S . \
    -B build.d \
    -G Ninja \
    -DCMAKE_INSTALL_PREFIX=./output \
    -DCMAKE_EXE_LINKER_FLAGS="$CMAKE_EXE_LINKER_FLAGS" \
    -DCMAKE_FIND_ROOT_PATH="$CMAKE_FIND_ROOT_PATH"

cmake --build   build.d
cmake --install build.d
```

**[FreeBSD](https://www.freebsd.org/)**

```bash
pkg install -y git cmake ninja pkgconf gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[OpenBSD](https://www.openbsd.org/)**

```bash
pkg_add git cmake ninja pkgconf llvm curl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[NetBSD](https://www.netbsd.org/)**

```bash
pkgin -y install git mozilla-rootcerts cmake ninja-build pkg-config clang curl openssl libgit2 libarchive libyaml jansson zlib

mozilla-rootcerts install

git clone --depth=1 https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

## ~/.ppkg

all relevant directories and files are located under `~/.ppkg` directory.

## ppkg command usage

- **show help of this command**

    ```bash
    ppkg -h
    ppkg --help
    ```

- **show version of this command**

    ```bash
    ppkg -V
    ppkg --version
    ```

- **generate url-transform sample**

    ```bash
    ppkg gen-url-transform-sample
    ```

- **install essential tools**

    ```bash
    ppkg setup
    ```

    This command is actually to do two things:

  - install [uppm](https://github.com/leleliu008/uppm) to `~/.ppkg/core`
  - install other essential tools (listed below) that are used by this shell script via [uppm](https://github.com/leleliu008/uppm)

    - [GNU Bash](https://www.gnu.org/software/bash/manual/bash.html)
    - [GNU CoreUtils](https://www.gnu.org/software/coreutils/manual/coreutils.html)
    - [GNU FindUtils](https://www.gnu.org/software/findutils/manual/html_mono/find.html)
    - [GNU awk](https://www.gnu.org/software/gawk/manual/gawk.html)
    - [GNU sed](https://www.gnu.org/software/sed/manual/sed.html)
    - [GNU grep](https://www.gnu.org/software/grep/manual/grep.html)
    - [curl](https://curl.se/docs/manpage.html)
    - [git](https://git-scm.com/docs/git)
    - [GNU tar](https://www.gnu.org/software/tar/manual/tar.html)
    - [gzip](https://www.gnu.org/software/gzip/manual/gzip.html)
    - [lzip](https://www.nongnu.org/lzip/)
    - [bzip2](https://linux.die.net/man/1/bzip2)
    - [xz](https://linux.die.net/man/1/xz)
    - [zip](https://linux.die.net/man/1/zip)
    - [unzip](https://linux.die.net/man/1/unzip)
    - [yq](https://mikefarah.gitbook.io/yq/)
    - [jq](https://stedolan.github.io/jq/manual/)
    - [tree](https://linux.die.net/man/1/tree)

- **show basic information about your current running operation system**

    ```bash
    ppkg sysinfo
    ```

- **show basic information about this software**

    ```bash
    ppkg env
    ```

- **integrate `zsh-completion` script**

    ```bash
    ppkg integrate zsh
    ppkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
    ppkg integrate zsh -v
    ```

    This software provides a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

- **update all available formula repositories**

    ```bash
    ppkg update
    ```

- **search all available packages whose name matches the given regular expression partten**

    ```bash
    ppkg search curl
    ppkg search lib
    ```

- **show information of the given package**

    ```bash
    ppkg info curl
    ppkg info curl --yaml
    ppkg info curl --json
    ppkg info curl version
    ppkg info curl license
    ppkg info curl summary
    ppkg info curl web-url
    ppkg info curl git-url
    ppkg info curl git-sha
    ppkg info curl git-ref
    ppkg info curl src-url
    ppkg info curl src-sha

    ppkg info curl installed-dir
    ppkg info curl installed-files
    ppkg info curl installed-version
    ppkg info curl installed-timestamp-unix
    ppkg info curl installed-timestamp-iso-8601
    ppkg info curl installed-timestamp-rfc-3339
    ppkg info curl installed-timestamp-iso-8601-utc
    ppkg info curl installed-timestamp-rfc-3339-utc

    ppkg info @all
    ```

- **show formula of the given package**

    ```bash
    ppkg formula curl
    ppkg formula curl --yaml
    ppkg formula curl --json
    ppkg formula curl --path
    ppkg formula curl version
    ppkg formula curl license
    ppkg formula curl summary
    ppkg formula curl web-url
    ppkg formula curl git-url
    ppkg formula curl git-sha
    ppkg formula curl git-ref
    ppkg formula curl src-url
    ppkg formula curl src-sha
    ```

- **show receipt of the given installed package**

    ```bash
    ppkg receipt curl
    ppkg receipt curl --yaml
    ppkg receipt curl --json
    ppkg receipt curl --path
    ppkg receipt curl version
    ppkg receipt curl license
    ppkg receipt curl summary
    ppkg receipt curl web-url
    ppkg receipt curl git-url
    ppkg receipt curl git-sha
    ppkg receipt curl git-ref
    ppkg receipt curl src-url
    ppkg receipt curl src-sha
    ```

- **show packages that are depended by the given package**

    ```bash
    ppkg depends curl

    ppkg depends curl -t dot
    ppkg depends curl -t box
    ppkg depends curl -t png
    ppkg depends curl -t svg

    ppkg depends curl -o curl-dependencies.dot
    ppkg depends curl -o curl-dependencies.txt
    ppkg depends curl -o curl-dependencies.png
    ppkg depends curl -o curl-dependencies.svg

    ppkg depends curl -t dot -o dependencies/
    ppkg depends curl -t box -o dependencies/
    ppkg depends curl -t png -o dependencies/
    ppkg depends curl -t svg -o dependencies/
    ```

- **download resources of the given package to the local cache**

    ```bash
    ppkg fetch curl
    ppkg fetch @all

    ppkg fetch curl -v
    ppkg fetch @all -v
    ```

- **install packages**

    ```bash
    ppkg install curl
    ppkg install curl bzip2 -v
    ```

    **Note:** C and C++ compiler should be installed by yourself using your system's default package manager before running this comand.

- **reinstall packages**

    ```bash
    ppkg reinstall curl
    ppkg reinstall curl bzip2 -v
    ```

- **uninstall packages**

    ```bash
    ppkg uninstall curl
    ppkg uninstall curl bzip2 -v
    ```

- **upgrade the outdated packages**

    ```bash
    ppkg upgrade
    ppkg upgrade curl
    ppkg upgrade curl bzip2 -v
    ```

- **upgrade this software**

    ```bash
    ppkg upgrade-self
    ppkg upgrade-self -v
    ```

- **list all avaliable formula repositories**

    ```bash
    ppkg formula-repo-list
    ```

- **create a new empty formula repository**

    ```bash
    uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo
    uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=master
    uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=main --pin
    uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --unpin --disable
    uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --enable
    ```

- **create a new empty formula repository then sync with server**

    ```bash
    uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo
    uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=master
    uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=main --pin
    uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --unpin --disable
    uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --enable
    ```

- **delete the given formula repository**

    ```bash
    uppm formula-repo-del my_repo
    ```

- **sync the given formula repository with server**

    ```bash
    uppm formula-repo-sync my_repo
    ```

- **show information of the given formula repository**

    ```bash
    uppm formula-repo-info my_repo
    ```

- **change the config of the given formula repository**

    ```bash
    uppm formula-repo-conf my_repo --url=https://github.com/leleliu008/uppm-formula-repository-my_repo
    uppm formula-repo-conf my_repo --branch=main
    uppm formula-repo-conf my_repo --pin
    uppm formula-repo-conf my_repo --unpin
    uppm formula-repo-conf my_repo --enable
    uppm formula-repo-conf my_repo --disable
    ```

- **list all available packages**

    ```bash
    ppkg ls-available
    ```

- **list all installed packages**

    ```bash
    ppkg ls-installed
    ```

- **list all outdated packages**

    ```bash
    ppkg ls-outdated
    ```

- **check if the given package is available**

    ```bash
    ppkg is-available curl
    ```

- **check if the given package is installed**

    ```bash
    ppkg is-installed curl
    ```

- **check if the given package is outdated**

    ```bash
    ppkg is-outdated  curl
    ```

- **list installed files of the given installed package in a tree-like format**

    ```bash
    ppkg tree curl
    ppkg tree curl -L 3
    ```

- **show logs of the given installed package**

    ```bash
    ppkg logs curl
    ```

- **pack the given installed package**

    ```bash
    ppkg pack curl
    ppkg pack curl -t tar.xz
    ppkg pack curl -t tar.gz
    ppkg pack curl -t tar.lz
    ppkg pack curl -t tar.bz2
    ppkg pack curl -t zip

    ppkg pack curl -t zip -o a/
    ppkg pack curl -o a/xx.zip
    ```

- **delete the unused cached files**

    ```bash
    ppkg cleanup
    ```

## environment variables

- **HOME**

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

- **PATH**

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

- **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

- **PPKG_URL_TRANSFORM**

    ```bash
    export PPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    you can generate a url-transform sample via `ppkg gen-url-transform-sample`

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

- **PPKG_XTRACE**

    for debugging purposes.

    enables `set -x`:

    ```bash
    export PPKG_XTRACE=1
    ```

- **other relevant environment variables**

    |utility|reference|
    |-|-|
    |[cmake](https://cmake.org/)|[reference](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html)|
    |[cargo](https://doc.rust-lang.org/cargo/)|[reference](https://doc.rust-lang.org/cargo/reference/environment-variables.html)|
    |[go](https://golang.org/)|[reference](https://golang.org/doc/install/source#environment)|
    |[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)|[reference](https://www.linuxhowtos.org/manpages/1/pkg-config.htm#lbAF)|
    |[aclocal](https://www.gnu.org/software/automake/manual/html_node/configure.html)|[reference](https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html)|

    example:

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

## ppkg formula

a ppkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ppkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ppkg formula's filename suffix must be `.yml`

a ppkg formula'a filename prefix would be treated as the package name.

a ppkg formula'a filename prefix must match regular expression partten `^[A-Za-z0-9+-._@]{1,50}$`

a ppkg formula's file content must follow [the ppkg formula scheme](https://github.com/leleliu008/ppkg-formula-repository-offical-core)

## ppkg formula repository

a ppkg formula repository is a git repository.

a ppkg formula repository's root dir should have a `formula` named sub dir, this repository's formulas all should be located in this dir.

a ppkg formula repository's local path is `~/.ppkg/repos.d/${PPKGFormulaRepoName}`

**Note:**

- please do NOT directly modify the formulas since your changes may be lost after the formula repository is updated!
- ppkg supports multiple formula repositories.

## ppkg formula repository's config

After a ppkg formula repository is successfully fetched from server to local, a config file for this repository would be created at `~/.ppkg/repos.d/${PPKGFormulaRepoName}/.ppkg-formula-repo.yml`

a typical ppkg formula repository's config as following:

```yml
url: https://github.com/leleliu008/ppkg-formula-repository-offical-core
branch: master
pinned: 0
enabled: 1
timestamp-created: 1673684639
timestamp-updated: 1673684767
```

If a ppkg formula repository is `pinned`, which means it would not be updated.

If a ppkg formula repository is `disabled`, which means ppkg would not search formulas in this formula repository.

## ppkg offical formula repository

ppkg offical formula repository's url: <https://github.com/leleliu008/ppkg-formula-repository-offical-core>

ppkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `ppkg update` command.

**Note:** If you find that a package is not in ppkg offical formula repository yet, PR is welcomed.
