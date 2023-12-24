#include <unistd.h>
#include "core/log.h"

int ppkg_help() {
    if (isatty(STDOUT_FILENO)) {
        const char * str = ""
        COLOR_GREEN
        "A portable package manager for Unix-like system.\n\n"
        "ppkg <ACTION> [ARGUMENT...]\n\n"
        "ppkg --help\n"
        "ppkg -h\n"
        COLOR_OFF
        "    show help of this command.\n\n"
        COLOR_GREEN
        "ppkg --version\n"
        "ppkg -V\n"
        COLOR_OFF
        "    show version of this command.\n\n"
        COLOR_GREEN
        "ppkg env\n"
        COLOR_OFF
        "    show basic information about this software.\n\n"
        COLOR_GREEN
        "ppkg buildinfo\n"
        COLOR_OFF
        "    show build information about this software.\n\n"
        COLOR_GREEN
        "ppkg sysinfo\n"
        COLOR_OFF
        "    show basic information about your current running operation system.\n\n"
        COLOR_GREEN
        "ppkg integrate zsh [-v] [--output-dir=<DIR>]\n"
        COLOR_OFF
        "    download a zsh completion script file to a approprivate location.\n\n"
        "    to apply this feature, you may need to run the command 'autoload -U compinit && compinit' in your terminal (your current running shell must be zsh).\n\n"
        COLOR_GREEN
        "ppkg update\n"
        COLOR_OFF
        "    update all available formula repositories.\n\n"
        COLOR_GREEN
        "ppkg search <REGULAR-EXPRESSION> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    search all available packages whose name matches the given regular expression pattern.\n\n"
        COLOR_GREEN
        "ppkg info <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    show information of the given package.\n\n"
        COLOR_GREEN
        "ppkg tree <PACKAGE-NAME> [--dirsfirst | -L N]\n"
        COLOR_OFF
        "    list files of the given installed package in a tree-like format.\n\n"
        COLOR_GREEN
        "ppkg pack <PACKAGE-NAME> -t <7z|zip|tar.gz|tar.xz|tar.bz2>\n"
        COLOR_OFF
        "    pack the given installed package.\n\n"
        COLOR_GREEN
        "ppkg logs <PACKAGE-NAME>\n"
        COLOR_OFF
        "    show logs of the given installed package.\n\n"
        COLOR_GREEN
        "ppkg depends <PACKAGE-NAME> [-t <OUTPUT-TYPE>] [-o <OUTPUT-PATH>] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    show packages that are depended by the given package.\n\n"
        "    <OUTPUT-TYPE> must be one of <dot|box|svg|png>\n\n"
        "    If -o <OUTPUT-PATH> option is given, the result will be written to file, otherwise, the result will be written to stdout.\n\n"
        "   <OUTPUT-PATH> can be either the filepath or directory. If it is an existing directory or ends with slash, then it will be treated as a directory, otherwise, it will be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it will be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-dependencies.<OUTPUT-TYPE>\n\n"
        "    If <OUTPUT-PATH> is treated as a filepath, and if -t <OUTPUT-TYPE> option is not given, if <OUTPUT-PATH> ends with one of .dot|.box|.svg|.png, <OUTPUT-TYPE> will be the <OUTPUT-PATH> suffix, otherwise, <OUTPUT-TYPE> will be box.\n\n"
        "    If -t <OUTPUT-TYPE> and -o <OUTPUT-PATH> options both are not given, <OUTPU-TYPE> will be box and output to stdout.\n\n"
        COLOR_GREEN
        "ppkg fetch   <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    download resources of the given package to the local cache.\n\n"
        COLOR_GREEN
        "ppkg   install <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        COLOR_OFF
        "    install the given packages.\n\n"
        COLOR_GREEN
        "ppkg   upgrade [PACKAGE-NAME]... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        COLOR_OFF
        "    upgrade the given packages or all outdated packages.\n\n"
        COLOR_GREEN
        "ppkg reinstall <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        COLOR_OFF
        "    reinstall the given packages.\n\n"
        COLOR_GREEN
        "ppkg uninstall <PACKAGE-NAME>\n"
        COLOR_OFF
        "    uninstall the given packages.\n\n"
        COLOR_GREEN
        "ppkg ls-available [-v] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    list the available packages.\n\n"
        COLOR_GREEN
        "ppkg ls-installed [-v]\n"
        COLOR_OFF
        "    list the installed packages.\n\n"
        COLOR_GREEN
        "ppkg ls-outdated [-v]\n"
        COLOR_OFF
        "    list the outdated  packages.\n\n"
        COLOR_GREEN
        "ppkg is-available <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    check if the given package is available.\n\n"
        COLOR_GREEN
        "ppkg is-installed <PACKAGE-NAME>\n"
        COLOR_OFF
        "    check if the given package is installed.\n\n"
        COLOR_GREEN
        "ppkg is-outdated  <PACKAGE-NAME>\n"
        COLOR_OFF
        "    check if the given package is outdated.\n\n"
        COLOR_GREEN
        "ppkg formula-view <PACKAGE-NAME> [--no-color] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    view the formula of the given package.\n\n"
        COLOR_GREEN
        "ppkg formula-edit <PACKAGE-NAME> [--editor=EDITOR] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        COLOR_OFF
        "    edit the formula of the given package.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-list\n"
        COLOR_OFF
        "    list all available formula repositories.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-add  <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    create a new empty formula repository then sync with server.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-del  <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    delete the given formula repository.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-init <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    create a new empty formula repository.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-conf <FORMULA-REPO-NAME>      [--url=VALUE | --branch=VALUE --pin/--unpin --enable/--disable]\n"
        COLOR_OFF
        "    change the config of the given formula repository.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-sync <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    sync the given formula repository with server.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-info <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    show information of the given formula repository.\n\n"
        COLOR_GREEN
        "ppkg gen-url-transform-sample\n"
        COLOR_OFF
        "    generate url-transform sample.\n\n"
        COLOR_GREEN
        "ppkg cleanup\n"
        COLOR_OFF
        "    cleanup the unused cache.\n\n\n"
        COLOR_GREEN
        "ppkg util zlib-deflate -L <LEVEL> < input/file/path\n"
        COLOR_OFF
        "    compress data using zlib deflate algorithm.\n\n"
        "    LEVEL >= 1 && LEVEL <= 9\n\n"
        "    The smaller the LEVEL, the faster the speed and the lower the compression ratio.\n\n"
        COLOR_GREEN
        "ppkg util zlib-inflate < input/file/path\n"
        COLOR_OFF
        "    decompress data using zlib inflate algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base16-encode <STR>\n"
        COLOR_OFF
        "    encode <STR> using base16 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base16-encode < input/file/path\n"
        COLOR_OFF
        "    encode data using base16 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base16-decode <BASE16-ENCODED-SUM>\n"
        COLOR_OFF
        "    decode <BASE16-ENCODED-SUM> using base16 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base16-decode < input/file/path\n"
        COLOR_OFF
        "    decode data using base16 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base64-encode <STR>\n"
        COLOR_OFF
        "    encode <STR> using base64 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base64-encode < input/file/path\n"
        COLOR_OFF
        "    encode data using base64 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base64-decode <BASE64-ENCODED-SUM>\n"
        COLOR_OFF
        "    decode <BASE64-ENCODED-SUM> using base64 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util base64-decode < input/file/path\n"
        COLOR_OFF
        "    decode data using base64 algorithm.\n\n"
        COLOR_GREEN
        "ppkg util sha256sum <input/file/path>\n"
        COLOR_OFF
        "    calculate sha256sum of file.\n\n"
        COLOR_GREEN
        "ppkg util sha256sum < input/file/path\n"
        COLOR_OFF
        "    calculate sha256sum of file.\n\n"
        COLOR_GREEN
        "ppkg util which <COMMAND-NAME> [-a]\n"
        COLOR_OFF
        "    find <COMMAND-NAME> in PATH.\n"
        ;

        printf("%s\n", str);
    } else {
        const char * str = ""
        "A portable package manager for Unix-like system.\n\n"
        "ppkg <ACTION> [ARGUMENT...]\n\n"
        "ppkg --help\n"
        "ppkg -h\n"
        "    show help of this command.\n\n"
        "ppkg --version\n"
        "ppkg -V\n"
        "    show version of this command.\n\n"
        "ppkg env\n"
        "    show basic information about this software.\n\n"
        "ppkg buildinfo\n"
        "    show build information about this software.\n\n"
        "ppkg sysinfo\n"
        "    show basic information about your current running operation system.\n\n"
        "ppkg update\n"
        "    update all available formula repositories.\n\n"
        "ppkg search <REGULAR-EXPRESSION> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    search all available packages whose name matches the given regular expression pattern.\n\n"
        "ppkg info <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    show information of the given package.\n\n"
        "ppkg tree <PACKAGE-NAME> [--dirsfirst | -L N]\n"
        "    list files of the given installed package in a tree-like format.\n\n"
        "ppkg pack <PACKAGE-NAME> -t <7z|zip|tar.gz|tar.xz|tar.bz2>\n"
        "    pack the given installed package.\n\n"
        "ppkg logs <PACKAGE-NAME>\n"
        "    show logs of the given installed package.\n\n"
        "ppkg depends <PACKAGE-NAME> [-t <OUTPUT-TYPE>] [-o <OUTPUT-PATH>] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    show packages that are depended by the given package.\n\n"
        "    <OUTPUT-TYPE> must be one of <dot|box|svg|png>\n\n"
        "    If -o <OUTPUT-PATH> option is given, the result will be written to file, otherwise, the result will be written to stdout.\n\n"
        "   <OUTPUT-PATH> can be either the filepath or directory. If it is an existing directory or ends with slash, then it will be treated as a directory, otherwise, it will be treated as a filepath.\n\n"
        "    If <OUTPUT-PATH> is treated as a directory, then it will be expanded to <OUTPUT-PATH>/<PACKAGE-NAME>-dependencies.<OUTPUT-TYPE>\n\n"
        "    If <OUTPUT-PATH> is treated as a filepath, and if -t <OUTPUT-TYPE> option is not given, if <OUTPUT-PATH> ends with one of .dot|.box|.svg|.png, <OUTPUT-TYPE> will be the <OUTPUT-PATH> suffix, otherwise, <OUTPUT-TYPE> will be box.\n\n"
        "    If -t <OUTPUT-TYPE> and -o <OUTPUT-PATH> options both are not given, <OUTPU-TYPE> will be box and output to stdout.\n\n"
        "ppkg fetch   <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    download resources of the given package to the local cache.\n\n"
        "ppkg   install <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    install the given packages.\n\n"
        "ppkg   upgrade [PACKAGE-NAME]... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    upgrade the given packages or all outdated packages.\n\n"
        "ppkg reinstall <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-session-dir]\n"
        "    reinstall the given packages.\n\n"
        "ppkg uninstall <PACKAGE-NAME>\n"
        "    uninstall the given packages.\n\n"
        "ppkg ls-available [-v] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    list the available packages.\n\n"
        "ppkg ls-installed [-v]\n"
        "    list the installed packages.\n\n"
        "ppkg ls-outdated [-v]\n"
        "    list the outdated  packages.\n\n"
        "ppkg is-available <PACKAGE-NAME> [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    check if the given package is available.\n\n"
        "ppkg is-installed <PACKAGE-NAME>\n"
        "    check if the given package is installed.\n\n"
        "ppkg is-outdated  <PACKAGE-NAME>\n"
        "    check if the given package is outdated.\n\n"
        "ppkg formula-view <PACKAGE-NAME> [--no-color] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    view the formula of the given package.\n\n"
        "ppkg formula-edit <PACKAGE-NAME> [--editor=EDITOR] [-p linux|macos|freebsd|openbsd|netbsd|dragonflybsd]\n"
        "    edit the formula of the given package.\n\n"
        "ppkg formula-repo-add  <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    create a new empty formula repository then sync with server.\n\n"
        "ppkg formula-repo-del <FORMULA-REPO-NAME>\n"
        "    delete the given formula repository.\n\n"
        "ppkg formula-repo-init <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    create a new empty formula repository.\n\n"
        "ppkg formula-repo-conf <FORMULA-REPO-NAME>      [--url=VALUE | --branch=VALUE --pin/--unpin --enable/--disable]\n"
        "    change the config of the given formula repository.\n\n"
        "ppkg formula-repo-sync <FORMULA-REPO-NAME>\n"
        "    sync the given formula repository with server.\n\n"
        "ppkg formula-repo-info <FORMULA-REPO-NAME>\n"
        "    show information of the given formula repository.\n\n"
        "ppkg formula-repo-list\n"
        "    list all available formula repositories.\n\n"
        "ppkg gen-url-transform-sample\n"
        "    generate url-transform sample.\n\n"
        "ppkg cleanup\n"
        "    cleanup the unused cache.\n"
        "ppkg util zlib-deflate -L <LEVEL> < input/file/path\n"
        "    compress data using zlib deflate algorithm.\n\n"
        "    LEVEL >= 1 && LEVEL <= 9\n\n"
        "    The smaller the LEVEL, the faster the speed and the lower the compression ratio.\n\n"
        "ppkg util zlib-inflate < input/file/path\n"
        "    decompress data using zlib inflate algorithm.\n\n"
        "ppkg util base16-encode <STR>\n"
        "    encode <STR> using base16 algorithm.\n\n"
        "ppkg util base16-encode < input/file/path\n"
        "    encode data using base16 algorithm.\n\n"
        "ppkg util base16-decode <BASE16-ENCODED-SUM>\n"
        "    decode <BASE16-ENCODED-SUM> using base16 algorithm.\n\n"
        "ppkg util base16-decode < input/file/path\n"
        "    decode data using base16 algorithm.\n\n"
        "ppkg util base64-encode <STR>\n"
        "    encode <STR> using base64 algorithm.\n\n"
        "ppkg util base64-encode < input/file/path\n"
        "    encode data using base64 algorithm.\n\n"
        "ppkg util base64-decode <BASE64-ENCODED-SUM>\n"
        "    decode <BASE64-ENCODED-SUM> using base64 algorithm.\n\n"
        "ppkg util base64-decode < input/file/path\n"
        "    decode data using base64 algorithm.\n\n"
        "ppkg util sha256sum <input/file/path>\n"
        "    calculate sha256sum of file.\n\n"
        "ppkg util sha256sum < input/file/path\n"
        "    calculate sha256sum of file.\n\n"
        "ppkg util which <COMMAND-NAME> [-a]\n"
        "    find <COMMAND-NAME> in PATH.\n"
        ;

        printf("%s\n", str);
    }

    return 0;
}
