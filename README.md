# ppkg
portable package manager for Unix-like system.

<br>

**Note**: This project is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

## two implementations
This project provides two implementations:
1. implemented in `POSIX Shell`, the source code is on `master` branch.
2. implemented in `C`, the source code is on `dev` branch.

In theroy, these two implementations should have the same behaver except for have bugs.

## Install POSIX-Shell-based ppkg
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/bin/ppkg
chmod a+x ppkg
mv ppkg /usr/local/bin/
ppkg setup
```

## ~/.ppkg
all relevant dirs and files are located in `~/.ppkg` directory.

**Note**: Please do NOT place your own files in `~/.ppkg` directory, as `ppkg` will change files in `~/.ppkg` directory without notice.

## ppkg command usage
*   **show help of this command**
        
        ppkg -h
        ppkg --help
        
*   **show version of this command**

        ppkg -V
        ppkg --version
        
*   **show your system's information**

        ppkg sysinfo

*   **show your system's information and other information**

        ppkg env
        
*   **integrate `zsh-completion` script**

        ppkg integrate zsh
        ppkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
        ppkg integrate zsh -v
        
    This project provides a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

*   **update all available formula repositories**

        ppkg update
        
*   **search all available packages whose name matches the given regular express partten**
        
        ppkg search curl
        ppkg search lib
        
*   **show information of the given package**
        
        ppkg info curl
        ppkg info curl summary
        ppkg info curl version
        ppkg info curl web-url
        ppkg info curl git-url
        ppkg info curl git-sha
        ppkg info curl git-ref
        ppkg info curl src-url
        ppkg info curl src-sha

        ppkg info curl formula-path

        ppkg info curl formula-json
        ppkg info curl formula-yaml

        ppkg info curl formula-json | jq .
        ppkg info curl formula-yaml | yq .

        ppkg info curl receipt-path

        ppkg info curl receipt-json
        ppkg info curl receipt-yaml

        ppkg info curl receipt-json | jq .
        ppkg info curl receipt-yaml | yq .

        ppkg info curl installed-dir
        ppkg info curl installed-files
        ppkg info curl installed-timestamp-unix
        ppkg info curl installed-timestamp-iso-8601
        ppkg info curl installed-timestamp-rfc-3339
        ppkg info curl installed-timestamp-iso-8601-utc
        ppkg info curl installed-timestamp-rfc-3339-utc
        ppkg info curl installed-version

        ppkg info @all
        
*   **show packages that are depended by the given package**
        
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
        
*   **download resources of the given package to the local cache**
        
        ppkg fetch curl
        ppkg fetch @all

        ppkg fetch curl -v
        ppkg fetch @all -v

*   **install packages**
        
        ppkg install curl
        ppkg install curl bzip2 -v
        
    **Note:** C and C++ compiler should be installed by yourself using your system's default package manager before running this comand.

*   **reinstall packages**
        
        ppkg reinstall curl
        ppkg reinstall curl bzip2 -v
        
*   **uninstall packages**

        ppkg uninstall curl
        ppkg uninstall curl bzip2 -v
        
*   **upgrade the outdated packages**

        ppkg upgrade
        ppkg upgrade curl
        ppkg upgrade curl bzip2 -v
        
*   **upgrade this software**

        ppkg upgrade-self
        ppkg upgrade-self -v
        
*   **view the formula of the given package**

        ppkg formula-view curl
        ppkg formula-view curl --no-color

*   **edit the formula of the given package**

        ppkg formula-edit curl
        ppkg formula-edit curl --editor=/usr/local/bin/vim

    **Note**: ppkg do NOT save your changes, which means that your changes may be lost after the formula repository is updated!

*   **list all avaliable formula repositories**

        ppkg formula-repo-list

*   **create a new empty formula repository**

        uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo
        uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=master
        uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=main --pin
        uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --unpin --disable
        uppm formula-repo-init my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --enable

*   **create a new empty formula repository then sync with server**

        uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo
        uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=master
        uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --branch=main --pin
        uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --unpin --disable
        uppm formula-repo-add my_repo https://github.com/leleliu008/uppm-formula-repository-my_repo --enable
        
*   **delete the given formula repository**

        uppm formula-repo-del my_repo

*   **sync the given formula repository with server**

        uppm formula-repo-sync my_repo

*   **show information of the given formula repository**

        uppm formula-repo-info my_repo

*   **change the config of the given formula repository**

        uppm formula-repo-conf my_repo --url=https://github.com/leleliu008/uppm-formula-repository-my_repo
        uppm formula-repo-conf my_repo --branch=main
        uppm formula-repo-conf my_repo --pin
        uppm formula-repo-conf my_repo --unpin
        uppm formula-repo-conf my_repo --enable
        uppm formula-repo-conf my_repo --disable

*   **list all available packages**
        
        ppkg ls-available
        
*   **list all installed packages**
        
        ppkg ls-installed
        
*   **list all outdated packages**
        
        ppkg ls-outdated
        
*   **check if the given package is available**
        
        ppkg is-available curl
        
*   **check if the given package is installed**
        
        ppkg is-installed curl
        
*   **check if the given package is outdated**
        
        ppkg is-outdated  curl
        
*   **list installed files of the given installed package in a tree-like format**
        
        ppkg tree curl
        ppkg tree curl -L 3
        
*   **show logs of the given installed package**
        
        ppkg logs curl
        
*   **pack the given installed package**
        
        ppkg pack curl
        ppkg pack curl -t tar.xz
        ppkg pack curl -t tar.gz
        ppkg pack curl -t tar.lz
        ppkg pack curl -t tar.bz2
        ppkg pack curl -t zip

        ppkg pack curl -t zip -o a/
        ppkg pack curl -o a/xx.zip
        
*   **delete the unused cached files**
        
        ppkg cleanup
        
*   **generate url-transform sample**

        ppkg gen-url-transform-sample


## environment variables

*   **HOME**

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

*   **PATH**

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

*   **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

*   **PPKG_URL_TRANSFORM**

    ```bash
    export PPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    you can generate a url-transform sample via `ppkg gen-url-transform-sample`

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.


*   **PPKG_XTRACE**

    for debugging purposes.

    this environment variable only affects POSIX-Shell-based implementation.

    enables `set -x`:

    ```bash
    export PPKG_XTRACE=1
    ```

*   **other relevant environment variables**

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

a ppkg formula'a filename prefix must match regular expression partten `^[A-Za-z0-9+-._]{1,50}$`

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

```
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

ppkg offical formula repository's url: https://github.com/leleliu008/ppkg-formula-repository-offical-core

ppkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `ppkg update` command.

**Note:** If you find that a package is not in ppkg offical formula repository yet, PR is welcomed.

