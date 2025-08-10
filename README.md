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

## Using ppkg via [Docker](https://www.docker.com/)

`docker` container is an isolated clean environment where the running process can not be affected by your host system's environemt variables.
|REPOSITORY|OS|ARCH|
|-|-|-|
|`ghcr.io/leleliu008/ppkg/alpine`|`alpine`|all except `loong64`|
|`ghcr.io/leleliu008/ppkg/alpine/amd64`|`alpine`|`amd64`|
|`ghcr.io/leleliu008/ppkg/alpine/arm64/v8`|`alpine`|`arm64/v8`|
|`ghcr.io/leleliu008/ppkg/alpine/loong64`|`alpine`|`loong64`|
|`ghcr.io/leleliu008/ppkg/alpine/riscv64`|`alpine`|`riscv64`|
|`ghcr.io/leleliu008/ppkg/alpine/ppc64le`|`alpine`|`ppc64le`|
|`ghcr.io/leleliu008/ppkg/alpine/s390x`|`alpine`|`s390x`|
||||
|`ghcr.io/leleliu008/ppkg/ubuntu`|`ubuntu`|all|
|`ghcr.io/leleliu008/ppkg/ubuntu/amd64`|`ubuntu`|`amd64`|
|`ghcr.io/leleliu008/ppkg/ubuntu/arm64/v8`|`ubuntu`|`arm64/v8`|
|`ghcr.io/leleliu008/ppkg/ubuntu/riscv64`|`ubuntu`|`riscv64`|
|`ghcr.io/leleliu008/ppkg/ubuntu/ppc64le`|`ubuntu`|`ppc64le`|
|`ghcr.io/leleliu008/ppkg/ubuntu/s390x`|`ubuntu`|`s390x`|
||||
|`ghcr.io/leleliu008/ppkg/debian/loong64`|`debian`|`loong64`|
||||
|`ghcr.io/leleliu008/ppkg/archlinux/loong64`|`archinux`|`loong64`|
||||
|`ghcr.io/leleliu008/ppkg/openeuler`|`openeuler`|all|
|`ghcr.io/leleliu008/ppkg/openeuler/amd64`|`openeuler`|`amd64`|
|`ghcr.io/leleliu008/ppkg/openeuler/arm64`|`openeuler`|`arm64`|
|`ghcr.io/leleliu008/ppkg/openeuler/loong64`|`openeuler`|`loong64`|

**step1. create a directory to be mounted to the docker container**

```bash
install -d ~/ppkg-home
```

**step2. setup [binfmt_misc](https://docs.kernel.org/admin-guide/binfmt-misc.html) if needed**

```bash
# for loongarch64
docker run --privileged --rm ghcr.io/loong64/binfmt --install all

# for others
docker run --privileged --rm multiarch/qemu-user-static --reset -p yes
```

You can also use your system package manager to do this instead of the docker on `Ubuntu`:

```bash
sudo apt-get -y update
sudo apt-get -y install qemu-user-static binfmt-support
update-binfmts --enable
```

**step3. create the ppkg docker container**

```bash
docker create -it --name ppkg -v ~/ppkg-home:/root/.ppkg ghcr.io/leleliu008/ppkg/alpine
```

**step4. start the ppkg docker container**

```bash
docker start ppkg
```

**step5. install essential tools**

```bash
docker exec -it ppkg ppkg setup
```

**step6. update formula repositories**

```bash
docker exec -it ppkg ppkg update
```

If all goes well, then next you can start to install packages whatever you want, for example, let's install `curl` package:

```bash
docker exec -it ppkg ppkg install curl --static
```

**Note:** you can use `podman` instead of `docker`

## Using ppkg via chroot+ubuntu

`chroot`, an isolated clean environment, is much like `docker` container, where the running process can not be affected by your host system's environment variables.

```bash
curl -LO https://cdimage.ubuntu.com/ubuntu-base/releases/24.04/release/ubuntu-base-24.04.2-base-amd64.tar.gz
install -d ubuntu-rootfs
tar xf ubuntu-base-24.04.2-base-amd64.tar.gz -C ubuntu-rootfs

cp -p /etc/resolv.conf ubuntu-rootfs/etc/

curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
chmod a+x ppkg
mv ppkg ubuntu-rootfs/bin/

sudo mount -o bind  /dev ubuntu-rootfs/dev
sudo mount -t proc  none ubuntu-rootfs/proc
sudo mount -t sysfs none ubuntu-rootfs/sys
sudo mount -t tmpfs none ubuntu-rootfs/tmp

sudo chroot ubuntu-rootfs ppkg setup -y
sudo chroot ubuntu-rootfs ppkg update
sudo chroot ubuntu-rootfs ppkg install curl
```

## Using ppkg via chroot+alpine

`chroot`, an isolated clean environment, is much like `docker` container, where the running process can not be affected by your host system's environment variables.

```bash
curl -LO https://dl-cdn.alpinelinux.org/alpine/v3.22/releases/x86_64/alpine-minirootfs-3.22.0-x86_64.tar.gz
install -d alpine-rootfs
tar xf alpine-minirootfs-3.22.0-x86_64.tar.gz -C alpine-rootfs

cp -p /etc/resolv.conf alpine-rootfs/etc/

curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
chmod a+x ppkg
mv ppkg alpine-rootfs/bin/

sudo mount -o bind  /dev alpine-rootfs/dev
sudo mount -t proc  none alpine-rootfs/proc
sudo mount -t sysfs none alpine-rootfs/sys

sudo chroot alpine-rootfs ppkg setup -y
sudo chroot alpine-rootfs ppkg update
sudo chroot alpine-rootfs ppkg install curl --static
```

you can use [alpine-chroot-install](https://github.com/alpinelinux/alpine-chroot-install/blob/master/alpine-chroot-install) instead of above

```bash
curl -LO https://raw.githubusercontent.com/alpinelinux/alpine-chroot-install/master/alpine-chroot-install
chmod +x alpine-chroot-install

sudo ./alpine-chroot-install -d ~/alpine-chroot -a x86_64

curl -L -o ~/alpine-chroot/ppkg https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg
chmod a+x  ~/alpine-chroot/ppkg

~/alpine-chroot/enter-chroot /ppkg setup
~/alpine-chroot/enter-chroot /ppkg update
~/alpine-chroot/enter-chroot /ppkg install curl --static
```

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

    This command needs `curl || wget` has already been installed.

    This command is actually to do two things:

  - install [uppm](https://github.com/leleliu008/uppm) to `~/.ppkg/core`
  - install other essential tools (listed below) used by this shell script via [uppm](https://github.com/leleliu008/uppm)

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

    This software provides a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, the rest of the arguments will be automatically completed for you.

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

- **show packages depended by the given package**

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

- **bundle the given installed package into a single archive file**

    ```bash
    ppkg bundle curl .tar.gz
    ppkg bundle curl .tar.xz
    ppkg bundle curl .tar.lz
    ppkg bundle curl .tar.bz2
    ppkg bundle curl .zip
    ppkg bundle curl xx.zip
    ppkg bundle curl a/.zip
    ppkg bundle curl a/xx.zip
    ```

- **export the given installed package as another package format (e.g. .deb, .rpm, .apk, .pkg.tar.xz, etc)**

    ```bash
    ppkg export curl .deb
    ppkg export curl curl-8.1.2-linux-x86_64.deb
    ppkg export curl a/curl-8.1.2-linux-x86_64.deb
    ppkg export curl a/.deb

    ppkg export curl .rpm
    ppkg export curl curl-8.1.2-linux-x86_64.rpm
    ppkg export curl a/curl-8.1.2-linux-x86_64.rpm
    ppkg export curl a/.rpm

    ppkg export curl .apk
    ppkg export curl curl-8.1.2-linux-x86_64.apk
    ppkg export curl a/curl-8.1.2-linux-x86_64.apk
    ppkg export curl a/.apk

    ppkg export curl .pkg.tar.xz
    ppkg export curl curl-8.1.2-linux-x86_64.pkg.tar.xz
    ppkg export curl a/curl-8.1.2-linux-x86_64.pkg.tar.xz
    ppkg export curl a/*.pkg.tar.xz
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

- **PPKG_FORMULA_SEARCH_DIRS**

    colon-seperated list of directories to search formulas.

- **GITHUB_ACTIONS**

    The value is `true` to indicates that `ppkg` is running on GitHub Actions.

    In this mode, `ppkg` will use system's package manager to install some packages e.g. `curl`, `g++`, `libc-dev`, `linux-headers`, `rust`, `cargo`, etc.

**Note:** some commonly used environment variables are overridden by this software, these are `CC`, `CXX`, `CPP`, `AS`, `AR`, `LD`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_PATH`, `ACLOCAL_PATH`

## ppkg formula scheme

a ppkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a ppkg package's meta-information including one sentence description, package version, installation instructions, etc.

a ppkg formula's filename suffix must be `.yml`

a ppkg formula'a filename prefix would be treated as the package name.

a ppkg formula'a filename prefix must match the regular expression partten `^[A-Za-z0-9+-._@]{1,50}$`

a ppkg formula's file content only has one level mapping and shall have the following `KEY`s:

|KEY|required?|TYPE|overview|
|-|-|-|-|
|`pkgtype`|optional|`ENUM`|the type of this package.<br>value shall be any one of `exe`, `lib`, `exe+lib`.<br>If this mapping is not present, `ppkg` will determine the package type by package name, if the package name starts/ends with `lib` or ends with `-dev`, it would be recognized as type `lib`, otherwise, it would be recognized as type `exe`|
|`linkage`|optional|`ENUM`|This mapping is only for `exe` type package to specify the executable's link method.<br>value shall be any one of `static`, `static/pie`, `shared`, `shared/most`.<br>`static` indicates this package only support creating fully statically linked executables.<br>`shared` indicates this package only support creating dynamically linked executables and `ppkg` will try to link as many static libraries as possible.<br>`shared/most` indicates this package only support creating dynamically linked executables and `ppkg` will try to link as many shared libraries as possible. <br>`shared` as default if this mapping is not present.|
|`summary`|required|`TEXT`|one sentence description of this package.|
|`license`|optional|`LIST`|A space-separated list of [SPDX license short identifiers](https://spdx.github.io/spdx-spec/v2.3/SPDX-license-list/#a1-licenses-with-short-identifiers)|
|`version`|optional|`TEXT`|the version of this package.<br>If this mapping is not present, it will be calculated from `src-url`, if `src-url` is also not present, it will be calculated from running time as format `date +%Y.%m.%d`|
||||
|`web-url`|optional|`URL`|the home webpage of this package.<br>If this mapping is not present, `git-url` must be present.|
||||
|`git-url`|optional|`URL`|the source code git repository url.<br>If `src-url` is not present, this mapping must be present.|
|`git-ref`|optional|`TEXT`|reference: <https://git-scm.com/book/en/v2/Git-Internals-Git-References> <br>example values: `HEAD` `refs/heads/master` `refs/heads/main` `refs/tags/v1`, default value is `HEAD`|
|`git-sha`|optional|`SHA1SUM`|the full git commit id, 40-byte hexadecimal string, if `git-ref` and `git-sha` both are present, `git-sha` takes precedence over `git-ref`|
|`git-nth`|optional|`INT`|tell `ppkg` that how many depth commits would you like to fetch. default is `1`, this would save your time and storage. If you have to fetch all commits, set this to `0`|
||||
|`src-url`|optional|`URI`|the source code download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/src` while this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/src`<br>also support format like `dir://DIR`|
|`src-uri`|optional|`URL`|the mirror of `src-url`.|
|`src-sha`|optional|`SHA256SUM`|the `sha256sum` of source code.<br>`src-sha` and `src-url` must appear together.|
||||
|`fix-url`|optional|`URL`|the patch file download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/fix` while this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/fix`.|
|`fix-uri`|optional|`URL`|the mirror of `fix-url`.|
|`fix-sha`|optional|`SHA256SUM`|the `sha256sum` of patch file.<br>`fix-sha` and `fix-url` must appear together.|
|`fix-opt`|optional|`LIST`|A space-separated list of arguments to be passed to `patch` command. default value is `-p1`.|
||||
|`patches`|optional|`LIST`|A LF-delimited list of formatted TEXTs. each TEXT has format: `<fix-sha>\|<fix-url>[\|fix-uri][\|fix-opt]`|
||||
|`res-url`|optional|`URL`|other resource download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/res` while this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/res`.|
|`res-uri`|optional|`URL`|the mirror of `res-url`.|
|`res-sha`|optional|`SHA256SUM`|the `sha256sum` of resource file.<br>`res-sha` and `res-url` must appear together.|
||||
|`reslist`|optional|`LIST`|A LF-delimited list of formatted TEXTs. each TEXT has format: `<res-sha>\|<res-url>[\|res-uri][\|unpack-dir][\|N]`. `unpack-dir` is relative to `$PACKAGE_WORKING_DIR/res`, default value is empty. `N` is `--strip-components=N`|
||||
|`dep-pkg`|optional|`LIST`|A space-separated list of   `ppkg packages` depended by this package when installing and/or runtime, which will be installed via [ppkg](https://github.com/leleliu008/ppkg).|
|`dep-pkg-musl`|optional|`LIST`|A space-separated list of   `ppkg packages` depended by this package when installing and/or runtime for target `musl`, which will be installed via [ppkg](https://github.com/leleliu008/ppkg).<br> packages that are missing in `musl-libc` e.g. `libfts` `libargp` `libobstack` `libexecinfo`|
|`dep-res`|optional|`LIST`|A space-separated list of   `well-known resources` needed by this package when installing.<br>The only possible value is `sys/queue.h` at the moment.|
|`dep-lib`|optional|`LIST`|A space-separated list of `pkg-config` packages needed by this package when installing.<br>each of them will be calculated via `pkg-config --libs-only-l ` then passed to the linker.|
|`dep-upp`|optional|`LIST`|A space-separated list of   `uppm packages` depended by this package when installing and/or runtime, which will be installed via [uppm](https://github.com/leleliu008/uppm).|
|`dep-plm`|optional|`LIST`|A space-separated list of    `perl modules` depended by this package when installing and/or runtime, which will be installed via [cpan](https://metacpan.org/dist/CPAN/view/scripts/cpan).|
|`dep-pip`|optional|`LIST`|A space-separated list of `python packages` depended by this package when installing and/or runtime, which will be installed via [pip3](https://github.com/pypa/pip).|
|`dep-gem`|optional|`LIST`|A space-separated list of    `ruby modules` depended by this package when installing and/or runtime, which will be installed via [gem](https://github.com/rubygems/rubygems).|
|`dep-npm`|optional|`LIST`|A space-separated list of    `nodejs packages` depended by this package when installing and/or runtime, which will be installed via [npm](https://github.com/npm/cli).|
||||
|`ccflags`|optional|`LIST`|A space-separated list of arguments to be passed to the C compiler.|
|`xxflags`|optional|`LIST`|A space-separated list of arguments to be passed to the C++ compiler.|
|`oxflags`|optional|`LIST`|A space-separated list of arguments to be passed to the Objc compiler.|
|`ppflags`|optional|`LIST`|A space-separated list of arguments to be passed to the PreProcessor.|
|`ldflags`|optional|`LIST`|A space-separated list of arguments to be passed to the linker.<br>`ppkg` supports a custom option `-p<PKG-CONFIG-PACKAGE-NAME>`. It will be substituted by the result of `pkg-config --libs-only-l <PKG-CONFIG-PACKAGE-NAME>`|
||||
|`bsystem`|optional|`LIST`|A space-separated list of build system names (e.g. `autogen` `autotools` `configure` `cmake` `cmake+gmake` `cmake+ninja` `meson` `xmake` `gmake` `ninja` `cargo` `cabal` `go` `rake`)|
|`bscript`|optional|`PATH`|the directory where the build script is located, relative to `PACKAGE_WORKING_DIR`. build script such as `configure`, `Makefile`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc.|
|`binbstd`|optional|`BOOL`|whether to build in the directory where the build script is located, otherwise build in other directory.<br>value shall be `0` or `1`. default value is `0`.|
|`ltoable`|optional|`BOOL`|whether support [LTO](https://gcc.gnu.org/wiki/LinkTimeOptimization).<br>value shall be `0` or `1`. default value is `1`.|
|`movable`|optional|`BOOL`|whether can be moved/copied to other locations.<br>value shall be `0` or `1`. default value is `1`.|
|`parallel`|optional|`BOOL`|whether to allow build system running jobs in parallel.<br>value shall be `0` or `1`. default value is `1`.|
||||
|`dofetch`|optional|`CODE`|POSIX shell code to be run to take over the fetching process.<br>It would be run in a separate process.<br>`PWD` is `$PACKAGE_WORKING_DIR`|
|`do12345`|optional|`CODE`|POSIX shell code to be run for native build.<br>It is running in a separated process.|
|`dopatch`|optional|`CODE`|POSIX shell code to be run to apply patches manually.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`prepare`|optional|`CODE`|POSIX shell code to be run to do some additional preparation before installing.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`install`|optional|`CODE`|POSIX shell code to be run when user run `ppkg install <PKG>`.<br>If this mapping is not present, `ppkg` will run default install code according to `bsystem`.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR` if `binbstd` is `0`, otherwise it is `$PACKAGE_BCACHED_DIR`|
|`dotweak`|optional|`CODE`|POSIX shell code to be run to do some tweaks immediately after installing.<br>`PWD` is `$PACKAGE_INSTALL_DIR`|
||||
|`bindenv`|optional|`LIST`|A LF-delimited list of formatted TEXTs. each TEXT has format: `<ENV>=<VALUE>`. `%s` in `<VALUE>` represents the install directory.<br>`ppkg` will bind these environment variables to executables while you are running `ppkg bundle`.|
||||
|`wrapper`|optional|`LIST`|A LF-delimited list of formatted TEXTs. each TEXT has format:  `<SRC>\|<DST>`. e.g. `bear.c\|bin/` means that `ppkg` will fetch `bear.c` from https://raw.githubusercontent.com/leleliu008/ppkg-formula-repository-official-core/refs/heads/master/wrappers/bear.c then install it to `$PACKAGE_INSTALL_DIR/bin/` directory.<br>`ppkg` will use these C source files to build the corresponding wrappers rather than a generic one while you are running `ppkg bundle`.|
||||
|`caveats`|optional|`TEXT`|plain text to be displayed after installing.|

|phases|
|-|
|<img src="phases.svg" width="200px" >|

**commands to be used out of the box:**

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
|`NATIVE_PLATFORM_KIND`|current running os kind. value shall be any one of `linux` `darwin` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`NATIVE_PLATFORM_TYPE`|current running os type. value shall be any one of `linux` `macos` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`NATIVE_PLATFORM_NAME`|current running os name. value might be any one of `Debian GNU/Linux` `Ubuntu` `CentOS` `Fedora` `FreeBSD` `NetBSD` `OpenBSD`, `DragonFlyBSD`, etc|
|`NATIVE_PLATFORM_VERS`|current running os version.|
|`NATIVE_PLATFORM_ARCH`|current running os arch. value might be any one of `x86_64` `amd64` `arm64` `aarch64`, `ppc64le`, `riscv64`, `s390x`, etc|
|`NATIVE_PLATFORM_NCPU`|current running os's cpu core count.|
|`NATIVE_PLATFORM_LIBC`|current running os's libc name. value shall be any one of `glibc` and `musl`.|
|`NATIVE_PLATFORM_EUID`|current running os's effective user ID.|
|`NATIVE_PLATFORM_EGID`|current running os's effective group ID.|
|||
|`TARGET_PLATFORM_NAME`|target platform name that is built for. value shall be any one of `linux` `macos` `freebsd` `netbsd` `openbsd` `dragonflybsd`|
|`TARGET_PLATFORM_VERS`|target platform version that is built with.|
|`TARGET_PLATFORM_ARCH`|target platform arch that is built for. value might be any one of `x86_64` `amd64` `arm64` `aarch64`, `ppc64le`, `riscv64`, `s390x`, etc|
|||
|`CROSS_COMPILING`|value shall be 0 or 1. indicates whether is cross-compiling.|
|||
|`CC`|the C compiler.|
|`CFLAGS`|arguments to be passed to the C compiler.|
|`CXX`|the C++ compiler.|
|`CXXFLAGS`|arguments to be passed to the C++ compiler.|
|`CPP`|the C/C++ PreProcessor.|
|`CPPFLAGS`|arguments to be passed to the C/C++ PreProcessor.|
|`AS`|the assembler.|
|`AR`|the archiver.|
|`RANLIB`|the archiver extra tool.|
|`LD`|the linker.|
|`LDFLAGS`|arguments to be passed to the linker.|
|`NM`|a command line tool to list symbols from object files.|
|`STRIP`|a command line tool to discard symbols and other data from object files.|
|||
|`PACKAGE_WORKING_DIR`|the working directory while installing.|
|`PACKAGE_BSCRIPT_DIR`|the directory where the build script (e.g. `Makefile`, `configure`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc) is located.|
|`PACKAGE_BCACHED_DIR`|the directory where the temporary files are stored while building.|
|`PACKAGE_INSTALL_DIR`|the directory where the final files will be installed.|
|||
|`x_INSTALL_DIR`|the installation directory of x package.|
|`x_INCLUDE_DIR`|`$x_INSTALL_DIR/include`|
|`x_LIBRARY_DIR`|`$x_INSTALL_DIR/lib`|

## build system name and corresponding build script file name

|build system name|build script file name|
|-|-|
|`meson`|`meson.build`|
|`cmake`|`CMakeLists.txt`|
|`gmake`|`GNUMakefile` or `Makefile`|
|`ninja`|`build.ninja`|
|`xmake`|`xmake.lua`|
|`cargo`|`Cargo.toml`|
|`cabal`|`cabal.project` `cabal.project.freeze` `cabal.project.local`|
|`go`|`go.mod`|
|`rake`|`Rakefile`|
|`autogen`|`autogen.sh`|
|`autotools`|`configure.ac`|
|`configure`|`configure`|

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

It would be automatically fetched to your local repository as name `official-core` while you are running `ppkg update` command.

**Note:** If you find that a package is not in ppkg official formula repository yet, PR is welcomed.

## prebuild packages built by this software

- <https://github.com/leleliu008/uppm-package-repository-linux-loongarch64>
- <https://github.com/leleliu008/uppm-package-repository-linux-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-linux-aarch64>
- <https://github.com/leleliu008/uppm-package-repository-linux-riscv64>
- <https://github.com/leleliu008/uppm-package-repository-linux-ppc64le>
- <https://github.com/leleliu008/uppm-package-repository-linux-s390x>
- <https://github.com/leleliu008/uppm-package-repository-linux-armv7l>
