#include <unistd.h>
#include "core/log.h"

int ppkg_help() {
    if (isatty(STDOUT_FILENO)) {
        const char * str = ""
        COLOR_GREEN
        "ppkg (Portable Package manager for UNIX)\n\n"
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
        "    show current machine os info.\n\n"
        COLOR_GREEN
        "ppkg update\n"
        COLOR_OFF
        "    update the formula repositories.\n\n"
        COLOR_GREEN
        "ppkg search  <keyword>\n"
        COLOR_OFF
        "    search packages whose name match the given keyword.\n\n"
        COLOR_GREEN
        "ppkg info    <PACKAGE-NAME|@all>\n"
        COLOR_OFF
        "    show information of the given package.\n\n"
        COLOR_GREEN
        "ppkg tree <PACKAGE-NAME> [--dirsfirst | -L N]\n"
        COLOR_OFF
        "    list files of the given installed package in a tree-like format.\n\n"
        COLOR_GREEN
        "ppkg depends <PACKAGE-NAME>\n"
        COLOR_OFF
        "    show packages that are depended by the given package.\n\n"
        COLOR_GREEN
        "ppkg fetch   <PACKAGE-NAME|@all>\n"
        COLOR_OFF
        "    download the prebuild binary archive file of the given package to the local cache.\n\n"
        COLOR_GREEN
        "ppkg   install <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        COLOR_OFF
        "    install the given packages.\n\n"
        COLOR_GREEN
        "ppkg   upgrade [PACKAGE-NAME]... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        COLOR_OFF
        "    upgrade the given packages or all outdated packages.\n\n"
        COLOR_GREEN
        "ppkg reinstall <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        COLOR_OFF
        "    reinstall the given packages.\n\n"
        COLOR_GREEN
        "ppkg uninstall <PACKAGE-NAME>\n"
        COLOR_OFF
        "    uninstall the given packages.\n\n"
        COLOR_GREEN
        "ppkg ls-available\n"
        COLOR_OFF
        "    list the available packages.\n\n"
        COLOR_GREEN
        "ppkg ls-installed\n"
        COLOR_OFF
        "    list the installed packages.\n\n"
        COLOR_GREEN
        "ppkg ls-outdated\n"
        COLOR_OFF
        "    list the outdated  packages.\n\n"
        COLOR_GREEN
        "ppkg is-available <PACKAGE-NAME>\n"
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
        "ppkg cleanup\n"
        COLOR_OFF
        "    cleanup the unused cache.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-list\n"
        COLOR_OFF
        "    list formula repositories.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-add <FORMULA-REPO-NAME> <FORMULA-REPO-URL> <BRANCH>\n"
        COLOR_OFF
        "    add a new formula-repo.\n\n"
        COLOR_GREEN
        "ppkg formula-repo-del <FORMULA-REPO-NAME>\n"
        COLOR_OFF
        "    delete a existing formula-repo.\n\n"
        COLOR_GREEN
        "ppkg pack <PACKAGE-NAME> --type=<7z|zip|tar.gz|tar.xz|tar.bz2>\n"
        COLOR_OFF
        "    pack the given installed package.\n\n"
        COLOR_GREEN
        "ppkg logs <PACKAGE-NAME>\n"
        COLOR_OFF
        "    show logs of the given installed package.\n";

        printf("%s\n", str);
    } else {
        const char * str = ""
        "ppkg (Universal Prebuild Package Manager)\n\n"
        "ppkg <ACTION> [ARGUMENT...]\n\n"
        "ppkg --help\n"
        "ppkg -h\n"
        "    show help of this command.\n\n"
        "ppkg --version\n"
        "ppkg -V\n"
        "    show version of this command.\n\n"
        "ppkg env\n"
        "    show current machine os info.\n\n"
        "ppkg update\n"
        "    update the formula repositories.\n\n"
        "ppkg search  <keyword>\n"
        "    search packages whose name match the given keyword.\n\n"
        "ppkg info    <PACKAGE-NAME|@all>\n"
        "    show information of the given package.\n\n"
        "ppkg tree <PACKAGE-NAME> [--dirsfirst | -L N]\n"
        "    list files of the given installed package in a tree-like format.\n\n"
        "ppkg depends <PACKAGE-NAME>\n"
        "    show packages that are depended by the given package.\n\n"
        "ppkg fetch   <PACKAGE-NAME|@all>\n"
        "    download the prebuild binary archive file of the given package to the local cache.\n\n"
        "ppkg   install <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        "    install the given packages.\n\n"
        "ppkg   upgrade [PACKAGE-NAME]... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        "    upgrade the given packages or all outdated packages.\n\n"
        "ppkg reinstall <PACKAGE-NAME>... [--jobs=N -q -v --dry-run --keep-installing-dir]\n"
        "    reinstall the given packages.\n\n"
        "ppkg uninstall <PACKAGE-NAME>\n"
        "    uninstall the given packages.\n\n"
        "ppkg ls-available\n"
        "    list the available packages.\n\n"
        "ppkg ls-installed\n"
        "    list the installed packages.\n\n"
        "ppkg ls-outdated\n"
        "    list the outdated  packages.\n\n"
        "ppkg is-available <PACKAGE-NAME>\n"
        "    check if the given package is available.\n\n"
        "ppkg is-installed <PACKAGE-NAME>\n"
        "    check if the given package is installed.\n\n"
        "ppkg is-outdated  <PACKAGE-NAME>\n"
        "    check if the given package is outdated.\n\n"
        "ppkg cleanup\n"
        "    cleanup the unused cache.\n\n"
        "ppkg formula-repo-list\n"
        "    list formula repositories.\n\n"
        "ppkg formula-repo-add <FORMULA-REPO-NAME> <FORMULA-REPO-URL> <BRANCH>\n"
        "    add a new formula-repo.\n\n"
        "ppkg formula-repo-del <FORMULA-REPO-NAME>\n"
        "    delete a existing formula-repo.\n\n"
        "ppkg pack <PACKAGE-NAME> --type=<7z|zip|tar.gz|tar.xz|tar.bz2>\n"
        "    pack the given installed package.\n\n"
        "ppkg logs <PACKAGE-NAME>\n"
        "    show logs of the given installed package.\n";

        printf("%s\n", str);
    }

    return 0;
}
