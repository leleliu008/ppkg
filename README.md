# zpkg
a lightweight, portable package manager for UNIX.

## Install zpkg via HomeBrew

```bash
brew tap leleliu008/fpliu
brew install zpkg
```

## Install zpkg via running shell script
```bash
sh -c "$(curl -fsSL https://raw.githubusercontent.com/leleliu008/zpkg/master/install.sh)"
```
**CHINESE USER IF NO VPN**, please try following instructions:
```bash
curl -LO https://raw.githubusercontents.com/leleliu008/zpkg/master/install.sh
chmod +x install.sh
./install.sh --china
```

## Install zpkg step by step
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/zpkg/master/bin/zpkg
chmod a+x zpkg
mv zpkg /usr/local/bin/

# following instructions is optional, and these instructions only worked in zsh
zpkg integrate zsh
autoload -U compinit && compinit
```

## zpkg command usage
*   show help of this command
        
        zpkg -h
        zpkg --help
        
*   show version of this command
        
        zpkg -V
        zpkg --version
        
*   show current machine os and environment variables

        zpkg env

*   integrate `zsh-completion` script

        zpkg integrate zsh
        zpkg integrate zsh -x
        zpkg integrate zsh --china
        zpkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `zpkg`. when you've typed `zpkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`


*   update the formula repositories

        zpkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [zpkg-formula-repository](https://github.com/leleliu008/zpkg-formula-repository)

*   search packages can be installed
        
        zpkg search curl
        zpkg search lib
        
*   show infomation of the given package or all available packages
        
        zpkg info curl
        zpkg info curl version
        zpkg info curl summary
        zpkg info curl webpage
        zpkg info curl src.git
        zpkg info curl installed-dir
        zpkg info curl installed-metadata
        zpkg info curl installed-datetime-unix
        zpkg info curl installed-datetime-formatted
        zpkg info curl installed-pkg-version
        zpkg info curl --json
        zpkg info curl --json | jq .
        zpkg info @all
        zpkg info @all --json
        zpkg info @all --json | jq .
        

    For more keys, please see [README.md](https://github.com/leleliu008/zpkg-formula-repository/blob/master/README.md#the-function-must-be-invoked-on-top-of-the-formula)

*   install packages
        
        zpkg install curl
        zpkg install curl bzip2
        zpkg install curl bzip2 --jobs=4
        zpkg install curl bzip2 --jobs=4 -v
        zpkg install curl bzip2 --jobs=4 -v -x
        zpkg install curl bzip2 --jobs=4 -v -x --dry-run
        zpkg install curl bzip2 --jobs=4 -v -x --dry-run --keep-working-dir
        
*   reinstall packages
        
        zpkg reinstall curl
        zpkg reinstall curl bzip2 -v
        
*   uninstall packages

        zpkg uninstall curl
        zpkg uninstall curl bzip2
        
*   upgrade the outdated packages

        zpkg upgrade
        zpkg upgrade curl
        zpkg upgrade curl bzip2 -v
        
*   upgrade this software

        zpkg upgrade @self
        zpkg upgrade @self -x
        zpkg upgrade @self --china
        zpkg upgrade @self --china -x
        

*   list the avaliable formula repos

        zpkg formula-repo list

*   add a new formula repo

        zpkg formula-repo add my_repo https://github.com/leleliu008/zpkg-formula-repository.git

*   delete a existing formula repo

        zpkg formula-repo del my_repo

*   view the formula of the given package
        
        zpkg formula view curl
        
*   edit the formula of the given package
        
        zpkg formula edit curl
        
*   create the formula of the given package
        
        zpkg formula create curl
        
*   delete the formula of the given package
        
        zpkg formula delete curl
        
*   rename the formula of the given package to new name
        
        zpkg formula rename curl curl7
        
*   list the available packages
        
        zpkg ls-available
        
*   list the installed packages
        
        zpkg ls-installed
        
*   list the outdated packages
        
        zpkg ls-outdated
        
*   is the given package available ?
        
        zpkg is-available curl
        zpkg is-available curl ge 7.50.0
        zpkg is-available curl gt 7.50.0
        zpkg is-available curl le 7.50.0
        zpkg is-available curl lt 7.50.0
        zpkg is-available curl eq 7.50.0
        zpkg is-available curl ne 7.50.0
        
*   is the given package installed ?
        
        zpkg is-installed curl
        
*   is the given package outdated ?
        
        zpkg is-outdated curl
        
*   list files of the given installed package in a tree-like format.
        
        zpkg tree curl
        zpkg tree curl -L 3
        
*   download formula resources of the given package to the cache
        
        zpkg fetch curl
        
*   show logs of the given installed package
        
        zpkg logs curl
        
*   pack the given installed package
        
        zpkg pack curl
        
*   show or open the homepage of the given package or this project
        
        zpkg homepage
        zpkg homepage --open
        zpkg homepage --open curl
        zpkg homepage curl --open
        
*   show the installation direcotory of the given package or this software
        
        zpkg prefix
        zpkg prefix curl
        
*   show the depended packages of the given package
        
        zpkg depends curl
        
*   cleanup the unused cache
        
        zpkg cleanup
        
