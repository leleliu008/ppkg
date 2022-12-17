#ifndef GIT_H
#define GIT_H

    int do_git_clone(const char * url, const char * repositoryDIR);
    int do_git_pull(const char * repositoryDIR, const char * remoteName, const char * refspec);

#endif
