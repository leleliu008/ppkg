# ppkg

A portable package builder/manager for Unix-like system.

## Caveats

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- Please do NOT place your own files under `~/.ppkg` directory, as `ppkg` will change files under `~/.ppkg` directory without notice.

- Please do NOT run `ppkg` command in parallel so as not to generate dirty data.

## Two implementations

This software provides two implementations:

|implemented in language|branch|
|-|-|
|implemented in `POSIX Shell`|[master](https://github.com/leleliu008/ppkg/tree/master)|
|implemented in `C`|[c](https://github.com/leleliu008/ppkg/tree/c)|

In theroy, these two implementations should have the same behaver except for have bugs.

## Using ppkg via GitHub Actions

In this way, you will be liberated from the rut of setting up the build environmemt.

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

## Build from C source locally dependencies

|dependency|required?|purpose|
|----|---------|-------|
|[GCC](https://gcc.gnu.org/) or [LLVM+clang](https://llvm.org/)|required |for compiling C source code|
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

## Build from C source locally via [ppkg](https://github.com/leleliu008/ppkg)

```bash
ppkg install ppkg
```

## Build from C source locally via [xcpkg](https://github.com/leleliu008/xcpkg)

```bash
xcpkg install ppkg
```

## Build from C source locally using [vcpkg](https://github.com/microsoft/vcpkg)

```bash
# install g++ curl zip unzip tar git

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT="$PWD/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"

vcpkg install curl openssl libgit2 libarchive libyaml jansson

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build   build.d
cmake --install build.d
```

## Build from C source locally via [HomeBrew](https://brew.sh/)

```bash
brew install --HEAD leleliu008/fpliu/ppkg
```

## Build from C source locally using your system's default package manager

**[Ubuntu](https://ubuntu.com/)**

```bash
apt -y update
apt -y install git cmake ninja-build pkg-config gcc libcurl4 libcurl4-openssl-dev libgit2-dev libarchive-dev libyaml-dev libjansson-dev zlib1g-dev

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Fedora](https://getfedora.org/)**

```bash
dnf -y update
dnf -y install git cmake ninja-build pkg-config gcc libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[ArchLinux](https://archlinux.org/)**

```bash
pacman -Syyuu --noconfirm
pacman -S     --noconfirm git cmake ninja pkg-config gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[AlpineLinux](https://www.alpinelinux.org/)**

```bash
apk add git cmake ninja pkgconf gcc libc-dev curl-dev openssl-dev libgit2-dev libarchive-dev yaml-dev jansson-dev zlib-dev

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[VoidLinux](https://voidlinux.org/)**

```bash
xbps-install -Suy xbps
xbps-install -Suy cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel jansson-devel zlib-devel

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[Gentoo Linux](https://www.gentoo.org/)**

```bash
emerge dev-vcs/git cmake dev-util/ninja gcc pkg-config net-misc/curl dev-libs/libgit2 libarchive dev-libs/libyaml dev-libs/jansson dev-libs/zlib

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[openSUSE](https://www.opensuse.org/)**

```bash
zypper update  -y
zypper install -y git cmake ninja gcc pkg-config libcurl-devel libgit2-devel libarchive-devel libyaml-devel libjansson-devel zlib-devel

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[FreeBSD](https://www.freebsd.org/)** and **[DragonFlyBSD](https://www.dragonflybsd.org/)**

```bash
pkg install -y git cmake ninja pkgconf gcc curl openssl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[OpenBSD](https://www.openbsd.org/)**

```bash
pkg_add git cmake ninja pkgconf llvm curl libgit2 libarchive libyaml jansson zlib

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
cd ppkg

cmake -S . -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build   build.d
cmake --install build.d
```

**[NetBSD](https://www.netbsd.org/)**

```bash
pkgin -y install git mozilla-rootcerts cmake ninja-build pkg-config clang curl openssl libgit2 libarchive libyaml jansson zlib

mozilla-rootcerts install

git clone --depth=1 --branch=c https://github.com/leleliu008/ppkg
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
    - [BSD tar](https://man.archlinux.org/man/core/libarchive/bsdtar.1.en)
    - [patchelf](https://github.com/NixOS/patchelf)
    - [sysinfo](https://github.com/leleliu008/C-examples/tree/master/utils/sysinfo)
    - [tree](https://linux.die.net/man/1/tree)
    - [curl](https://curl.se/docs/manpage.html)
    - [git](https://git-scm.com/docs/git)
    - [jq](https://stedolan.github.io/jq/manual/)
    - [yq](https://mikefarah.gitbook.io/yq/)
    - [d2](https://github.com/terrastruct/d2)
    - [fzf](https://github.com/junegunn/fzf)
    - [bat](https://github.com/sharkdp/bat)
    - [xxd](https://raw.githubusercontent.com/vim/vim/master/runtime/doc/xxd.man)

- **show basic information about this software**

    ```bash
    ppkg about
    ```

- **show basic information about your current running operation system**

    ```bash
    ppkg sysinfo
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

- **list all available formula repositories**

    ```bash
    ppkg formula-repo-list
    ```

- **create a new empty formula repository**

    ```bash
    ppkg formula-repo-init my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo
    ppkg formula-repo-init my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --branch=master
    ppkg formula-repo-init my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --branch=main --pin
    ppkg formula-repo-init my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --unpin --disable
    ppkg formula-repo-init my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --enable
    ```

- **create a new empty formula repository then sync with server**

    ```bash
    ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo
    ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --branch=master
    ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --branch=main --pin
    ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --unpin --disable
    ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo --enable
    ```

- **delete the given formula repository**

    ```bash
    ppkg formula-repo-del my_repo
    ```

- **sync the given formula repository with server**

    ```bash
    ppkg formula-repo-sync my_repo
    ```

- **show information of the given formula repository**

    ```bash
    ppkg formula-repo-info my_repo
    ```

- **change the config of the given formula repository**

    ```bash
    ppkg formula-repo-conf my_repo --url=https://github.com/leleliu008/ppkg-formula-repository-my_repo
    ppkg formula-repo-conf my_repo --branch=main
    ppkg formula-repo-conf my_repo --pin
    ppkg formula-repo-conf my_repo --unpin
    ppkg formula-repo-conf my_repo --enable
    ppkg formula-repo-conf my_repo --disable
    ```

- **search all available packages whose name matches the given regular expression partten**

    ```bash
    ppkg search curl
    ppkg search curl -v
    ppkg search curl -p macos
    ```

- **show information of the given available package**

    ```bash
    ppkg info-available curl
    ppkg info-available curl --yaml
    ppkg info-available curl --json
    ppkg info-available curl version
    ppkg info-available curl license
    ppkg info-available curl summary
    ppkg info-available curl web-url
    ppkg info-available curl git-url
    ppkg info-available curl git-sha
    ppkg info-available curl git-ref
    ppkg info-available curl src-url
    ppkg info-available curl src-sha
    ```

- **show information of the given installed package**

    ```bash
    ppkg info-installed freebsd-13.2-amd64/curl
    ppkg info-installed freebsd-13.2-amd64/curl --prefix
    ppkg info-installed freebsd-13.2-amd64/curl --files
    ppkg info-installed freebsd-13.2-amd64/curl --yaml
    ppkg info-installed freebsd-13.2-amd64/curl --json
    ppkg info-installed freebsd-13.2-amd64/curl version
    ppkg info-installed freebsd-13.2-amd64/curl license
    ppkg info-installed freebsd-13.2-amd64/curl summary
    ppkg info-installed freebsd-13.2-amd64/curl web-url
    ppkg info-installed freebsd-13.2-amd64/curl git-url
    ppkg info-installed freebsd-13.2-amd64/curl git-sha
    ppkg info-installed freebsd-13.2-amd64/curl git-ref
    ppkg info-installed freebsd-13.2-amd64/curl src-url
    ppkg info-installed freebsd-13.2-amd64/curl src-sha
    ppkg info-installed freebsd-13.2-amd64/curl builtat
    ppkg info-installed freebsd-13.2-amd64/curl builtat-iso-8601
    ppkg info-installed freebsd-13.2-amd64/curl builtat-rfc-3339
    ppkg info-installed freebsd-13.2-amd64/curl builtat-iso-8601-utc
    ppkg info-installed freebsd-13.2-amd64/curl builtat-rfc-3339-utc
    ```

- **show packages that are depended by the given package**

    ```bash
    ppkg depends curl

    ppkg depends curl -t d2
    ppkg depends curl -t dot
    ppkg depends curl -t box
    ppkg depends curl -t png
    ppkg depends curl -t svg

    ppkg depends curl -o curl-dependencies.d2
    ppkg depends curl -o curl-dependencies.dot
    ppkg depends curl -o curl-dependencies.txt
    ppkg depends curl -o curl-dependencies.png
    ppkg depends curl -o curl-dependencies.svg

    ppkg depends curl -t d2  -o dependencies/
    ppkg depends curl -t dot -o dependencies/
    ppkg depends curl -t box -o dependencies/
    ppkg depends curl -t png -o dependencies/
    ppkg depends curl -t svg -o dependencies/
    ```

- **download resources of the given package to the local cache**

    ```bash
    ppkg fetch curl
    ppkg fetch curl -v
    ```

- **install the given packages**

    ```bash
    ppkg install curl
    ppkg install curl bzip2 -v
    ```

    **Note:** C and C++ compiler should be installed by yourself using your system's default package manager before running this command.

- **reinstall the given packages**

    ```bash
    ppkg reinstall curl
    ppkg reinstall curl bzip2 -v
    ```

- **uninstall the given packages**

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

- **check if the given package is available**

    ```bash
    ppkg is-available curl -p macos
    ```

- **check if the given package is installed**

    ```bash
    ppkg is-installed curl
    ```

- **check if the given package is outdated**

    ```bash
    ppkg is-outdated  curl
    ```

- **list all available packages**

    ```bash
    ppkg ls-available
    ppkg ls-available -v
    ppkg ls-available -p macos
    ```

- **list all installed packages**

    ```bash
    ppkg ls-installed
    ppkg ls-installed -v
    ppkg ls-installed --target=linux-musl-x86_64
    ```

- **list all outdated packages**

    ```bash
    ppkg ls-outdated
    ppkg ls-outdated -v
    ppkg ls-outdated --target=linux-musl-x86_64
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

- **export the given installed package as anthoer package format (e.g. deb, rpm, pkg, apk, etc)**

    ```bash
    ppkg export.deb curl
    ppkg export.deb curl -o .
    ppkg export.deb curl -o a/
    ppkg export.deb curl -o curl-8.1.2-linux-x86_64.deb

    ppkg export.rpm curl
    ppkg export.rpm curl -o .
    ppkg export.rpm curl -o a/
    ppkg export.rpm curl -o curl-8.1.2-linux-x86_64.rpm

    ppkg export.pkg curl
    ppkg export.pkg curl -o .
    ppkg export.pkg curl -o a/
    ppkg export.pkg curl -o curl-8.1.2-linux-x86_64.pkg.tar.xz

    ppkg export.apk curl
    ppkg export.apk curl -o .
    ppkg export.apk curl -o a/
    ppkg export.apk curl -o curl-8.1.2-linux-x86_64.apk
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

- **GOPROXY**

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

- **PPKG_HOME**

    If this environment variable is not set or set a empty string, `$HOME/.ppkg` will be used as the default value.

    ```bash
    export PPKG_HOME=$HOME/ppkg-home
    ```

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

- **PPKG_DEFAULT_TARGET**

    Some ACTIONs of ppkg are associated with an installed package which need `PACKAGE-SPEC` to be specified.

    **PACKAGE-SPEC** : a formatted string that has form: `<TARGET-PLATFORM>/<PACKAGE-NAME>`, represents an installed package.

    **PACKAGE-NAME** : should match the regular expression pattern `^[A-Za-z0-9+-_.@]{1,50}$`

    **TARGET-PLATFORM** : a formatted string that has form: `<TARGET-PLATFORM-NAME>-<TARGET-PLATFORM-VERSION>-<TARGET-PLATFORM-ARCH>`

    **TARGET-PLATFORM-ARCH** : indicates which cpu arch was built for. value might be any one of `x86_64` `amd64` `arm64` `aarch64`, `ppc64le`, `riscv64`, `s390x`

    **TARGET-PLATFORM-NAME** : indicates which platform name was built for. value shall be any one of `linux` `macos` `freebsd` `netbsd` `openbsd` `dragonflybsd`

    **TARGET-PLATFORM-VERSION** : indicates which platform version was built with.

    To simplify the usage, you are allowed to omit `<TARGET-PLATFORM>/`. If `<TARGET-PLATFORM>/` is omitted, environment variable `PPKG_DEFAULT_TARGET` would be checked, if this environment variable is not set, then your current running os target will be used as the default.

    **Example**:

    ```bash
    export PPKG_DEFAULT_TARGET=linux-glibc-x86_64
    export PPKG_DEFAULT_TARGET=linux-musl-x86_64
    export PPKG_DEFAULT_TARGET=macos-13.0-arm64
    export PPKG_DEFAULT_TARGET=macos-13.0-x86_64
    export PPKG_DEFAULT_TARGET=freebsd-13.2-amd64
    export PPKG_DEFAULT_TARGET=openbsd-7.4-amd64
    export PPKG_DEFAULT_TARGET=netbsd-9.3-amd64
    ```

**Note:** some commonly used environment variables are override by this software, these are `CC`, `CXX`, `CPP`, `AS`, `AR`, `LD`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_PATH`, `ACLOCAL_PATH`

## ppkg formula scheme

a ppkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ppkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ppkg formula's filename suffix must be `.yml`

a ppkg formula'a filename prefix would be treated as the package name.

a ppkg formula'a filename prefix must match the regular expression partten `^[A-Za-z0-9+-._@]{1,50}$`

a ppkg formula's file content only has one level mapping and shall has following KEY:

|KEY|required?|overview|
|-|-|-|
|`pkgtype`|optional|indicates what type of this package. value shall be any one of `exe`, `pie`, `lib`, `exe+lib`.<br>To `exe` type package, `ppkg` would add `--static -static` options to `LDFLAGS` if `--static` install option is given.<br>To `pie` type package, it means that it doesn't support fully statically linking, it is supposed to be dynamically linked.<br>If this mapping is not present, `ppkg` will determine the package type by package name, if a package name starts/ends with `lib`, it would be recognized as type `lib`, otherwise, it would be recognized as type `exe`|
|`summary`|required|describe this package in one sentence.|
|`license`|optional|a space-separated list of [SPDX license short identifiers](https://spdx.github.io/spdx-spec/v2.3/SPDX-license-list/#a1-licenses-with-short-identifiers)|
|`version`|optional|the version of this package.<br>If this mapping is not present, it will be calculated from `src-url`, if `src-url` is also not present, it will be calculated from running time as format `date +%Y.%m.%d`|
||||
|`web-url`|optional|the home webpage of this package.<br>If this mapping is not present, `git-url` must be present.|
||||
|`git-url`|optional|the source code git repository.<br>If `src-url` is not present, this mapping must be present.|
|`git-ref`|optional|reference: <https://git-scm.com/book/en/v2/Git-Internals-Git-References> <br>example values: `HEAD` `refs/heads/master` `refs/heads/main` `refs/tags/v1`, default value is `HEAD`|
|`git-sha`|optional|the full git commit id, 40-byte hexadecimal string, if `git-ref` and `git-sha` both are present, `git-sha` takes precedence over `git-ref`|
|`git-nth`|optional|tell `ppkg` that how many depth commits would you like to be fetched. default is `1`, this would save your time and storage. If you want to fetch all commits, set this to `0`|
||||
|`src-url`|optional|the source code download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/src` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/src`<br>also support format like `dir://DIR`|
|`src-uri`|optional|the mirror of `src-url`.|
|`src-sha`|optional|the `sha256sum` of source code.<br>`src-sha` and `src-url` must appear together.|
||||
|`fix-url`|optional|the patch file download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/fix` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/fix`.|
|`fix-uri`|optional|the mirror of `fix-url`.|
|`fix-sha`|optional|the `sha256sum` of patch file.<br>`fix-sha` and `fix-url` must appear together.|
|`fix-opt`|optional|options to be passed to `patch` command. default value is `-p1`.|
||||
|`patches`|optional|multiple lines of `<fix-sha>\|<fix-url>[\|fix-uri][\|fix-opt]`.|
||||
|`res-url`|optional|other resource download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/res` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/res`.|
|`res-uri`|optional|the mirror of `res-url`.|
|`res-sha`|optional|the `sha256sum` of resource file.<br>`res-sha` and `res-url` must appear together.|
||||
|`reslist`|optional|multiple lines of `<res-sha>\|<res-url>[\|res-uri][\|unpack-dir][\|N]`. `unpack-dir` is relative to `$PACKAGE_WORKING_DIR/res`, default value is empty. `N` is `--strip-components=N`|
||||
|`dep-pkg`|optional|a space-separated list of   `ppkg packages` that are depended by this package when installing and/or runtime, which will be installed via [ppkg](https://github.com/leleliu008/ppkg).|
|`dep-upp`|optional|a space-separated list of   `uppm packages` that are depended by this package when installing and/or runtime, which will be installed via [uppm](https://github.com/leleliu008/uppm).|
|`dep-pym`|optional|a space-separated list of `python packages` that are depended by this package when installing and/or runtime, which will be installed via [pip3](https://github.com/pypa/pip).|
|`dep-plm`|optional|a space-separated list of    `perl modules` that are depended by this package when installing and/or runtime, which will be installed via [cpan](https://metacpan.org/dist/CPAN/view/scripts/cpan).|
||||
|`ccflags`|optional|append to `CFLAGS`|
|`xxflags`|optional|append to `CXXFLAGS`|
|`ppflags`|optional|append to `CPPFLAGS`|
|`ldflags`|optional|append to `LDFLAGS`|
||||
|`bsystem`|optional|build system name.<br>values can be one or a combination of `autogen` `autotools` `configure` `cmake` `cmake+gmake` `cmake+ninja` `meson` `xmake` `gmake` `ninja` `cargo` `go` `rake`|
|`bscript`|optional|the directory where the build script is located in, relative to `PACKAGE_WORKING_DIR`. build script such as `configure`, `Makefile`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc.|
|`binbstd`|optional|whether to build in the directory where the build script is located in, otherwise build in other directory.<br>value shall be `0` or `1`. default value is `0`.|
|`movable`|optional|whether can be moved/copied to other locations.<br>value shall be `0` or `1`. default value is `1`.|
|`parallel`|optional|whether to allow build system running jobs in parallel.<br>value shall be `0` or `1`. default value is `1`.|
||||
|`onstart`|optional|POSIX shell code to be run when this package's formula is loaded.<br>`PWD` is `$PACKAGE_WORKING_DIR`|
|`onready`|optional|POSIX shell code to be run when this package's needed resources all are ready.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`onfinal`|optional|POSIX shell code to be run when this package is successfully installed.<br>`PWD` is `$PACKAGE_INSTALL_DIR`|
||||
|`do12345`|optional|POSIX shell code to be run for native build.<br>It is only meaningful when requesting for cross building.<br>It is running in a separated process.|
|`dopatch`|optional|POSIX shell code to be run to apply patches manually.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`prepare`|optional|POSIX shell code to be run to do some additional preparation.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`install`|optional|POSIX shell code to be run when user run `ppkg install <PKG>`. If this mapping is not present, `ppkg` will run default install code according to `bsystem`.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR` if `binbstd` is `0`, otherwise it is `$PACKAGE_BCACHED_DIR`|
|`dotweak`|optional|POSIX shell code to be run to do some tweaks immediately after installing.<br>`PWD` is `$PACKAGE_INSTALL_DIR`|
||||
|`caveats`|optional|multiple lines of plain text to be displayed after installation.|

|phases|
|-|
|![phases](phases.svg)|

|build system name|build script file name|
|-|-|
|`meson`|`meson.build`|
|`cmake`|`CMakeLists.txt`|
|`gmake`|`GNUMakefile` or `Makefile`|
|`ninja`|`build.ninja`|
|`xmake`|`xmake.lua`|
|`cargo`|`Cargo.toml`|
|`go`|`go.mod`|
|`rake`|`Rakefile`|
|`autogen`|`autogen.sh`|
|`autotools`|`configure.ac`|
|`configure`|`configure`|

**commands that can be used out of the box:**

|command|usage-example|
|-|-|
|`bash`|[Reference](https://www.gnu.org/software/bash/manual/bash.html)|
|`CoreUtils`|[Reference](https://www.gnu.org/software/coreutils/manual/coreutils.html)|
|`xargs`|[Reference](https://www.gnu.org/software/findutils/manual/html_node/find_html/Invoking-xargs.html)|
|`find`|[Reference](https://www.gnu.org/software/findutils/manual/html_mono/find.html)|
|`gawk`|[Reference](https://www.gnu.org/software/gawk/manual/gawk.html)|
|`gsed`|[Reference](https://www.gnu.org/software/sed/manual/sed.html)|
|`grep`|[Reference](https://www.gnu.org/software/grep/manual/grep.html)|
|`tree`|[Reference](https://linux.die.net/man/1/tree)|
|`jq`|[Reference](https://stedolan.github.io/jq/manual/)|
|`yq`|[Reference](https://mikefarah.gitbook.io/yq/)|
|`d2`|[Reference](https://github.com/terrastruct/d2)|
|`bat`|[Reference](https://github.com/sharkdp/bat)|
|`xxd`|[Reference](https://raw.githubusercontent.com/vim/vim/master/runtime/doc/xxd.man)|
|`git`|[Reference](https://git-scm.com/docs/git)|
|`curl`|[Reference](https://curl.se/docs/manpage.html)|
|`bsdtar`|[Reference](https://man.archlinux.org/man/core/libarchive/bsdtar.1.en)|
|`pkg-config`|[Reference](https://people.freedesktop.org/~dbn/pkg-config-guide.html)|
|`patchelf`|[Reference](https://github.com/NixOS/patchelf)|
|`sysinfo`|[Reference](https://github.com/leleliu008/C-examples/tree/master/utils/sysinfo)|
|||
|`echo`|`echo 'your message.'`|
|`info`|`info 'your information.'`|
|`warn`|`warn "no package manager found."`|
|`error`|`error 'error message.'`|
|`abort`|`abort 1 "please specify a package name."`|
|`success`|`success "build success."`|
|`isInteger`|`isInteger $x \|\| abort 1 "should be an integer."`|
|`isCrossBuild`|`isCrossBuild && abort 1 "This package is not supposed to be cross built."`|
|`wfetch`|`wfetch <URL> [--uri=<URL-MIRROR>] [--sha256=<SHA256>] [-o <PATH> [-q]`|
|||
|`configure`|`configure --enable-pic`|
|`mesonw`|`mesonw -Dneon=disabled -Darm-simd=disabled`|
|`cmakew`|`cmakew -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON`|
|`gmakew`|`gmakew`|
|`xmakew`|`xmakew`|
|`cargow`|`cargow`|
|`gow`|`gow`|

**shell variables can be used directly:**

|variable|overview|
|-|-|
|`PPKG_ARG0`|the 1st arguments of `ppkg` that you've supplied.|
|`PPKG_ARG1`|the 2nd arguments of `ppkg` that you've supplied.|
|`PPKG_ARGV`|the all arguments of `ppkg` that you've supplied.|
|`PPKG_PATH`|the full path of `ppkg` that you're running.|
|`PPKG_HOME`|the home directory of `ppkg` that you're running.|
|`PPKG_VERSION`|the version of `ppkg` that you're running.|
|||
|`UPPM`|the executable filepath of [uppm](https://github.com/leleliu008/uppm)|
|||
|`TIMESTAMP_UNIX`|the unix timestamp of this action.|
|||
|`NATIVE_OS_KIND`|current running os kind. value shall be any one of `linux` `darwin` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`NATIVE_OS_TYPE`|current running os type. value shall be any one of `linux` `macos` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`NATIVE_OS_NAME`|current running os name. value might be any one of `Debian GNU/Linux` `Ubuntu` `CentOS` `Fedora` `FreeBSD` `NetBSD` `OpenBSD`, `DragonFlyBSD`, etc|
|`NATIVE_OS_VERS`|current running os version.|
|`NATIVE_OS_ARCH`|current running os arch. value might be any one of `x86_64` `amd64` `arm64` `aarch64`, `ppc64le`, `riscv64`, `s390x`, etc|
|`NATIVE_OS_NCPU`|current running os's cpu core count.|
|`NATIVE_OS_LIBC`|current running os's libc name. value shall be any one of `glibc` and `musl`.|
|`NATIVE_OS_EUID`|current running os's effective user ID.|
|`NATIVE_OS_EGID`|current running os's effective group ID.|
|||
|`TARGET_PLATFORM_NAME`|target platform name that is built for. value shall be any one of `linux` `macos` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`TARGET_PLATFORM_VERS`|target platform version that is built with.|
|`TARGET_PLATFORM_ARCH`|target platform arch that is built for. value might be any one of `x86_64` `amd64` `arm64` `aarch64`, `ppc64le`, `riscv64`, `s390x`, etc|
|||
|`CROSS_COMPILING`|value shall be 0 or 1. indicates whether is cross-compiling.|
|||
|`CC`|the C Compiler.|
|`CFLAGS`|the flags of `CC`.|
|`CXX`|the C++ Compiler.|
|`CXXFLAGS`|the flags of `CXX`.|
|`CPP`|the C/C++ PreProcessor.|
|`CPPFLAGS`|the flags of `CPP`.|
|`AS`|the assembler.|
|`AR`|the archiver.|
|`RANLIB`|the archiver extra tool.|
|`LD`|the linker.|
|`LDFLAGS`|the flags of `LD`.|
|`NM`|a command line tool to list symbols from object files.|
|`STRIP`|a command line tool to discard symbols and other data from object files.|
|||
|`PACKAGE_WORKING_DIR`|the working directory when installing.|
|`PACKAGE_BSCRIPT_DIR`|the directory where the build script (e.g. `Makefile`, `configure`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc) is located in.|
|`PACKAGE_BCACHED_DIR`|the directory where the temporary files are stored in when building.|
|`PACKAGE_INSTALL_DIR`|the directory where the final files will be installed to.|
|||
|`x_INSTALL_DIR`|the installation directory of x package.|
|`x_INCLUDE_DIR`|`$x_INSTALL_DIR/include`|
|`x_LIBRARY_DIR`|`$x_INSTALL_DIR/lib`|

## ppkg formula repository

a typical hierarchical structure of a ppkg formula repository looks like below:

```
PPKGFormulaRepoName
├── formula
│   ├── packageA.yml
│   └── packageB.yml
├── LICENSE
└── README.md
```

## ppkg formula repository local location

`${PPKG_HOME}/repos.d/${PPKGFormulaRepoName}`

## ppkg formula repository local config

a ppkg formula repository's config file is located at `${PPKG_HOME}/repos.d/${PPKGFormulaRepoName}/.ppkg-formula-repo.yml`

a typical ppkg formula repository's config file content looks like below:

```yaml
url: https://github.com/leleliu008/ppkg-formula-repository-official-core
branch: master
pinned: 0
enabled: 1
created: 1673684639
updated: 1673684767
```

If a ppkg formula repository is `pinned`, which means it would not be updated.

If a ppkg formula repository is `disabled`, which means ppkg would not search formulas in this formula repository.

## ppkg formula repository management

run `ppkg formula-repo-add ` command to create a new formula repository locally from an exsting remote git repository.

run `ppkg formula-repo-init` command to create a new formula repository locally without taking any further action.

## ppkg official formula repository

ppkg official formula repository is hosted at <https://github.com/leleliu008/ppkg-formula-repository-official-core>

It would be automatically fetched to your local repository as name `official-core` when you run `ppkg update` command.

**Note:** If you find that a package is not in ppkg official formula repository yet, PR is welcomed.

## prebuild packages built by this software

- <https://github.com/leleliu008/uppm-package-repository-linux-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-linux-aarch64>
- <https://github.com/leleliu008/uppm-package-repository-linux-riscv64>
- <https://github.com/leleliu008/uppm-package-repository-linux-ppc64le>
- <https://github.com/leleliu008/uppm-package-repository-linux-s390x>
