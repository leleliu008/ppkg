#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/log.h"
#include "ppkg.h"

int ppkg_main(int argc, char* argv[]) {
    if (argc == 1) {
        ppkg_help();
        return PPKG_OK;
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        ppkg_help();
        return PPKG_OK;
    }

    if ((strcmp(argv[1], "-V") == 0) || (strcmp(argv[1], "--version") == 0)) {
        printf("%s\n", PPKG_VERSION);
        return PPKG_OK;
    }

    bool verbose = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            break;
        }
    }

    if (strcmp(argv[1], "env") == 0) {
        int resultCode = ppkg_env(verbose);

        if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "update") == 0) {
        int resultCode = ppkg_formula_repo_list_update();

        if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "search") == 0) {
        int resultCode = ppkg_search(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is not given.\n", argv[0]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is empty string.\n", argv[0]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "info") == 0) {
        int resultCode = ppkg_info(argv[2], argv[3]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_ARG_IS_UNKNOWN) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], unrecognized KEY: %s\n", argv[0], argv[3]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "tree") == 0) {
        int resultCode = ppkg_tree(argv[2], argc - 3, &argv[3]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "depends") == 0) {
        PPKGDependsOutputFormat outputFormat = PPKGDependsOutputFormat_BOX;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "--format=dot") == 0) {
                outputFormat = PPKGDependsOutputFormat_DOT;
            } else if (strcmp(argv[i], "--format=box") == 0) {
                outputFormat = PPKGDependsOutputFormat_BOX;
            } else if (strcmp(argv[i], "--format=png") == 0) {
                outputFormat = PPKGDependsOutputFormat_PNG;
            } else if (strcmp(argv[i], "--format=svg") == 0) {
                outputFormat = PPKGDependsOutputFormat_SVG;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return PPKG_ARG_IS_INVALID;
            }
        }

        int resultCode = ppkg_depends(argv[2], outputFormat);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "fetch") == 0) {
        int resultCode = ppkg_fetch(argv[2], verbose);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "install") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                int resultCode = ppkg_check_if_the_given_package_is_available(argv[i]);

                if (resultCode == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (resultCode == PPKG_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (resultCode != PPKG_OK) {
                    return resultCode;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int resultCode = ppkg_install(packageName, verbose);

            if (resultCode == PPKG_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (resultCode == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (resultCode != PPKG_OK) {
                return resultCode;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "uninstall") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                int resultCode = ppkg_check_if_the_given_package_is_installed(argv[i]);

                if (resultCode == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (resultCode == PPKG_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                    LOG_ERROR3("package [", argv[i], "] is not installed.");
                }

                if (resultCode != PPKG_OK) {
                    return resultCode;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int resultCode = ppkg_uninstall(packageName, verbose);

            if (resultCode == PPKG_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (resultCode == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (resultCode != PPKG_OK) {
                return resultCode;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "reinstall") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                int resultCode = ppkg_check_if_the_given_package_is_available(argv[i]);

                if (resultCode == PPKG_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (resultCode != PPKG_OK) {
                    return resultCode;
                }

                resultCode = ppkg_check_if_the_given_package_is_installed(argv[i]);

                if (resultCode == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (resultCode == PPKG_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (resultCode != PPKG_OK) {
                    return resultCode;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int resultCode = ppkg_reinstall(packageName, verbose);

            if (resultCode == PPKG_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (resultCode == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (resultCode != PPKG_OK) {
                return resultCode;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "upgrade") == 0) {
        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                int resultCode = ppkg_check_if_the_given_package_is_outdated(argv[i]);

                if (resultCode == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (resultCode == PPKG_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                    LOG_ERROR3("package [", argv[i], "] is not installed.");
                } else if (resultCode == PPKG_PACKAGE_IS_NOT_OUTDATED) {
                    LOG_ERROR3("package [", argv[i], "] is not outdated.");
                }

                if (resultCode != PPKG_OK) {
                    return resultCode;
                }
            }
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int resultCode = ppkg_upgrade(packageName, verbose);

            if (resultCode == PPKG_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (resultCode == PPKG_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (resultCode == PPKG_PACKAGE_IS_NOT_OUTDATED) {
                fprintf(stderr, "package [%s] is not outdated.\n", packageName);
            } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (resultCode == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (resultCode != PPKG_OK) {
                return resultCode;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "ls-available") == 0) {
        int resultCode = ppkg_list_the_available_packages();

        if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "ls-installed") == 0) {
        int resultCode = ppkg_list_the_installed_packages();

        if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "ls-outdated") == 0) {
        int resultCode = ppkg_list_the_outdated__packages();

        if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "is-available") == 0) {
        int resultCode = ppkg_check_if_the_given_package_is_available(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "is-installed") == 0) {
        int resultCode = ppkg_check_if_the_given_package_is_installed(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "is-outdated") == 0) {
        int resultCode = ppkg_check_if_the_given_package_is_outdated(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "formula-repo-list") == 0) {
        return ppkg_formula_repo_list_printf();
    }

    if (strcmp(argv[1], "formula-repo-add") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> <FORMULA-REPO-BRANCH>\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> <FORMULA-REPO-BRANCH>\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        return ppkg_formula_repo_add(argv[2], argv[3], argv[4]);
    }

    if (strcmp(argv[1], "formula-repo-del") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return PPKG_ARG_IS_NULL;
        }

        return ppkg_formula_repo_del(argv[2]);
    }

    if (strcmp(argv[1], "integrate") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s integrate <zsh|bash|fish>\n", argv[0]);
            return PPKG_ARG_IS_NULL;
        } else if (strcmp(argv[2], "zsh") == 0) {
            return ppkg_integrate_zsh_completion (NULL, verbose);
        } else if (strcmp(argv[2], "bash") == 0) {
            return ppkg_integrate_bash_completion(NULL, verbose);
        } else if (strcmp(argv[2], "fish") == 0) {
            return ppkg_integrate_fish_completion(NULL, verbose);
        } else {
            LOG_ERROR2("unrecognized argument: ", argv[2]);
            return PPKG_ARG_IS_INVALID;
        }
    }

    if (strcmp(argv[1], "upgrade-self") == 0) {
        return ppkg_upgrade_self(verbose);
    }

    if (strcmp(argv[1], "cleanup") == 0) {
        return ppkg_cleanup(verbose);
    }

    if (strcmp(argv[1], "logs") == 0) {
        int resultCode = ppkg_logs(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "pack") == 0) {
        int resultCode = ppkg_pack(argv[2], ArchiveType_tar_xz, verbose);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (resultCode == PPKG_PACKAGE_IS_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (resultCode == PPKG_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (resultCode == PPKG_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "util-base64-encode-of-string") == 0) {
        int resultCode = ppkg_util_base64_encode_of_string(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <STR> , <STR> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <STR> , <STR> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }

    if (strcmp(argv[1], "util-base64-decode-to-string") == 0) {
        int resultCode = ppkg_util_base64_decode_to_string(argv[2]);

        if (resultCode == PPKG_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <STR> , <STR> is not given.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <STR> , <STR> is empty string.\n", argv[0], argv[1]);
        } else if (resultCode == PPKG_ARG_IS_INVALID) {
            fprintf(stderr, "%s is invalid base64-encoded string.\n", argv[2]);
        } else if (resultCode == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return resultCode;
    }


    LOG_ERROR2("unrecognized action: ", argv[1]);
    return PPKG_ARG_IS_UNKNOWN;
}

int main(int argc, char* argv[]) {
    return ppkg_main(argc, argv);
}
