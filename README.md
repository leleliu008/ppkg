# ppkg
portable package manager for Unix-like system.

## Install ppkg
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ppkg/master/bin/ppkg
chmod a+x ppkg
mv ppkg /usr/local/bin/
ppkg setup
```

## Uninstall ppkg
```bash
rm /usr/local/bin/ppkg
rm -rf /opt/ppkg
```

## ppkg command usage
*   show help of this command
        
        ppkg -h
        ppkg --help
        
*   show version of this command
        
        ppkg -V
        ppkg --version
        
*   show current machine os and environment variables

        ppkg env

*   integrate `zsh-completion` script

        ppkg integrate zsh
        ppkg integrate zsh -x
        ppkg integrate zsh --china
        ppkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `ppkg`. when you've typed `ppkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`


*   update the formula repositories

        ppkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [ppkg-formula-repository](https://github.com/leleliu008/ppkg-formula-repository)

*   search packages can be installed
        
        ppkg search curl
        ppkg search lib
        
*   show infomation of the given package or all available packages
        
        ppkg info curl
        ppkg info curl version
        ppkg info curl summary
        ppkg info curl webpage
        ppkg info curl src.git
        ppkg info curl installed-dir
        ppkg info curl installed-metadata
        ppkg info curl installed-datetime-unix
        ppkg info curl installed-datetime-formatted
        ppkg info curl installed-pkg-version
        ppkg info curl --json
        ppkg info curl --json | jq .
        ppkg info @all
        ppkg info @all --json
        ppkg info @all --json | jq .
        

    For more keys, please see [README.md](https://github.com/leleliu008/ppkg-formula-repository/blob/master/README.md#the-function-must-be-invoked-on-top-of-the-formula)

*   install packages
        
        ppkg install curl
        ppkg install curl bzip2
        ppkg install curl bzip2 --jobs=4
        ppkg install curl bzip2 --jobs=4 -v
        ppkg install curl bzip2 --jobs=4 -v -x
        ppkg install curl bzip2 --jobs=4 -v -x --dry-run
        ppkg install curl bzip2 --jobs=4 -v -x --dry-run --keep-working-dir
        
*   reinstall packages
        
        ppkg reinstall curl
        ppkg reinstall curl bzip2 -v
        
*   uninstall packages

        ppkg uninstall curl
        ppkg uninstall curl bzip2
        
*   upgrade the outdated packages

        ppkg upgrade
        ppkg upgrade curl
        ppkg upgrade curl bzip2 -v
        
*   upgrade this software

        ppkg upgrade @self
        ppkg upgrade @self -x
        ppkg upgrade @self --china
        ppkg upgrade @self --china -x
        

*   list the avaliable formula repos

        ppkg formula-repo list

*   add a new formula repo

        ppkg formula-repo add my_repo https://github.com/leleliu008/ppkg-formula-repository.git

*   delete a existing formula repo

        ppkg formula-repo del my_repo

*   view the formula of the given package
        
        ppkg formula view curl
        
*   edit the formula of the given package
        
        ppkg formula edit curl
        
*   create the formula of the given package
        
        ppkg formula create curl
        
*   delete the formula of the given package
        
        ppkg formula delete curl
        
*   rename the formula of the given package to new name
        
        ppkg formula rename curl curl7
        
*   list the available packages
        
        ppkg ls-available
        
*   list the installed packages
        
        ppkg ls-installed
        
*   list the outdated packages
        
        ppkg ls-outdated
        
*   is the given package available ?
        
        ppkg is-available curl
        ppkg is-available curl ge 7.50.0
        ppkg is-available curl gt 7.50.0
        ppkg is-available curl le 7.50.0
        ppkg is-available curl lt 7.50.0
        ppkg is-available curl eq 7.50.0
        ppkg is-available curl ne 7.50.0
        
*   is the given package installed ?
        
        ppkg is-installed curl
        
*   is the given package outdated ?
        
        ppkg is-outdated curl
        
*   list files of the given installed package in a tree-like format.
        
        ppkg tree curl
        ppkg tree curl -L 3
        
*   download formula resources of the given package to the cache
        
        ppkg fetch curl
        
*   show logs of the given installed package
        
        ppkg logs curl
        
*   pack the given installed package
        
        ppkg pack curl
        
*   show or open the homepage of the given package or this project
        
        ppkg homepage
        ppkg homepage --open
        ppkg homepage --open curl
        ppkg homepage curl --open
        
*   show the installation direcotory of the given package or this software
        
        ppkg prefix
        ppkg prefix curl
        
*   show the depended packages of the given package
        
        ppkg depends curl
        
*   cleanup the unused cache
        
        ppkg cleanup
        
