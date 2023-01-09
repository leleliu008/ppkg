# ppkg
portable package manager for Unix-like system.

# two implementations
I provide two implementations of this software:
- implemented in `POSIX Shell`, the source code is on `master` branch.
- implemented in `C`, the source code is on `dev` branch.

In theroy, they should have same behaver except for have bugs.

## Install ppkg
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/bin/ppkg
chmod a+x ppkg
mv ppkg /usr/local/bin/
ppkg setup
```

## ~/.ppkg
all relevant dirs and files are located in `~/.ppkg` directory.

## ppkg command usage
*   **show help of this command**
        
        ppkg -h
        ppkg --help
        
*   **show version of this command**

        ppkg -V
        ppkg --version
        
*   **show current machine os info**

        ppkg env
        
*   **integrate `zsh-completion` script**

        ppkg integrate zsh
        ppkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
        ppkg integrate zsh -v
        
    I have provide a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`

*   **update formula repositories**

        ppkg update
        
*   **search packages**
        
        ppkg search curl
        ppkg search lib
        
*   **show infomation of the given package**
        
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

        ppkg depends curl --format=dot
        ppkg depends curl --format=box
        ppkg depends curl --format=png
        ppkg depends curl --format=svg

        ppkg depends curl --format=dot > xx.dot
        ppkg depends curl --format=box > xx.txt
        ppkg depends curl --format=png > xx.png
        ppkg depends curl --format=svg > xx.svg
        
*   **download formula resources of the given package to the local cache**
        
        ppkg fetch curl
        ppkg fetch @all

        ppkg fetch curl -v
        ppkg fetch @all -v

*   **install packages**
        
        ppkg install curl
        ppkg install curl bzip2 -v
        
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
        
*   **list the avaliable formula repositories**

        ppkg formula-repo-list

*   **add a new formula repository**

        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo
        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo master
        ppkg formula-repo-add my_repo https://github.com/leleliu008/ppkg-formula-repository-my_repo main
        
    **Note:**
    - this software supports multiple formula repositories.
    - offical formula repository is https://github.com/leleliu008/ppkg-formula-repository-offical-core

*   **delete a existing formula repository**

        ppkg formula-repo-del my_repo

*   **list the available packages**
        
        ppkg ls-available
        
*   **list the installed packages**
        
        ppkg ls-installed
        
*   **list the outdated packages**
        
        ppkg ls-outdated
        
*   **is the given package available ?**
        
        ppkg is-available curl
        
*   **is the given package installed ?**
        
        ppkg is-installed curl
        
*   **is the given package outdated ?**
        
        ppkg is-outdated  curl
        
*   **list files of the given installed package in a tree-like format**
        
        ppkg tree curl
        ppkg tree curl -L 3
        
*   **show logs of the given installed package**
        
        ppkg logs curl
        
*   **pack the given installed package**
        
        ppkg pack curl
        ppkg pack curl --type=tar.xz
        ppkg pack curl --type=tar.gz
        ppkg pack curl --type=tar.lz
        ppkg pack curl --type=tar.bz2
        ppkg pack curl --type=zip
        
*   **cleanup the unused cached files**
        
        ppkg cleanup
        

## influential environment variables

*   **HOME**

    this environment variable must be set.

    this environment variable already have been set on most systems, if not set or set a empty string, you will receive an error message.

*   **PATH**

    some features rely on this environment variable.

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

    following is a example of `/path/of/url-transform` command implementation:

    ```bash
    #!/bin/sh

    case $1 in
        *githubusercontent.com/*)
            printf 'https://ghproxy.com/%s\n' "$1"
            ;;
        https://github.com/*)
            printf 'https://ghproxy.com/%s\n' "$1"
            ;;
        '') printf '%s\n' 'url-transform <URL>, <URL> is not given.' >&2 ;;
        *)  printf '%s\n' "$1"
    esac
    ```

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

*   **PPKG_XTRACE**

    for debugging purposes.

    this environment variable only affects POSIX-Shell-based implementation.

    enables `set -x`:

    ```bash
    export PPKG_XTRACE=1
    ```

*   **CMake relevant environment variables**

    [Reference](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html)

*   **Cargo relevant environment variables**

    [Reference](https://doc.rust-lang.org/cargo/reference/environment-variables.html)

*   **Golang relevant environment variables**

    [Reference](https://golang.org/doc/install/source#environment)

    example:

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```
