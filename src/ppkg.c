#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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

    if (strcmp(argv[1], "setup") == 0) {
        return ppkg_setup(verbose);
    }

    if (strcmp(argv[1], "sysinfo") == 0) {
        return ppkg_sysinfo();
    }

    if (strcmp(argv[1], "buildinfo") == 0) {
        return ppkg_buildinfo();
    }

    if (strcmp(argv[1], "env") == 0) {
        int ret = ppkg_env(verbose);

        if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "update") == 0) {
        int ret = ppkg_formula_repo_list_update();

        if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "search") == 0) {
        int ret = ppkg_search(argv[2]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is not given.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is empty string.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "info") == 0) {
        int ret = ppkg_info(argv[2], argv[3]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_ARG_IS_UNKNOWN) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], unrecognized KEY: %s\n", argv[0], argv[3]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "tree") == 0) {
        int ret = ppkg_tree(argv[2], argc - 3, &argv[3]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "depends") == 0) {
        int    outputType     = 0;

        char * outputPath     = NULL;
        char * outputFilePath = NULL;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-t") == 0) {
                const char * type = argv[i + 1];

                if (type == NULL) {
                    fprintf(stderr, "-t <OUTPUT-TYPE>, <OUTPUT-TYPE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                }

                if (type[0] == '\0') {
                    fprintf(stderr, "-o <OUTPUT-TYPE>, <OUTPUT-TYPE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_EMPTY;
                }

                if (strcmp(type, "dot") == 0) {
                    outputType = 1;
                    i++;
                } else if (strcmp(type, "box") == 0) {
                    outputType = 2;
                    i++;
                } else if (strcmp(type, "svg") == 0) {
                    outputType = 3;
                    i++;
                } else if (strcmp(type, "png") == 0) {
                    outputType = 4;
                    i++;
                } else {
                    LOG_ERROR2("unrecognized type: ", type);
                    return PPKG_ERROR_ARG_IS_INVALID;
                }
             } else if (strcmp(argv[i], "-o") == 0) {
                outputPath = argv[i + 1];

                if (outputPath == NULL) {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                }

                if (outputPath[0] == '\0') {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_EMPTY;
                }

                i++;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return PPKG_ERROR_ARG_IS_INVALID;
            }
        }

        if (outputPath != NULL) {
            struct stat st;

            if (stat(outputPath, &st) == 0 && S_ISDIR(st.st_mode)) {
                size_t outputFilePathLength = strlen(outputPath) + strlen(argv[2]) + 20U;

                outputFilePath = (char*) malloc(outputFilePathLength);

                if (outputFilePath == NULL) {
                    return PPKG_ERROR_MEMORY_ALLOCATE;
                }

                const char * outputFileNameSuffix;

                switch (outputType) {
                    case 0: outputFileNameSuffix = "box"; break;
                    case 1: outputFileNameSuffix = "dot"; break;
                    case 2: outputFileNameSuffix = "box"; break;
                    case 3: outputFileNameSuffix = "svg"; break;
                    case 4: outputFileNameSuffix = "png"; break;
                }

                snprintf(outputFilePath, outputFilePathLength, "%s/%s-dependencies.%s", outputPath, argv[2], outputFileNameSuffix);
            } else {
                size_t outputPathLength = strlen(outputPath);

                if (outputPath[outputPathLength - 1] == '/') {
                    size_t outputFilePathLength = strlen(outputPath) + strlen(argv[2]) + 20U;

                    outputFilePath = (char*) malloc(outputFilePathLength);

                    if (outputFilePath == NULL) {
                        return PPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    const char * outputFileNameSuffix;

                    switch (outputType) {
                        case 0: outputFileNameSuffix = "box"; break;
                        case 1: outputFileNameSuffix = "dot"; break;
                        case 2: outputFileNameSuffix = "box"; break;
                        case 3: outputFileNameSuffix = "svg"; break;
                        case 4: outputFileNameSuffix = "png"; break;
                    }

                    snprintf(outputFilePath, outputFilePathLength, "%s%s-dependencies.%s", outputPath, argv[2], outputFileNameSuffix);
                } else {
                    outputFilePath = strdup(outputPath);

                    if (outputFilePath == NULL) {
                        return PPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    // if -t option is not given, let's check if is explicitly specified in filename's suffix
                    if (outputType == 0) {
                        if (outputPathLength > 4) {
                            const char * suffix = outputFilePath + outputPathLength - 4;

                                   if (strcmp(suffix, ".dot") == 0) {
                                outputType = 1;
                            } else if (strcmp(suffix, ".box") == 0) {
                                outputType = 2;
                            } else if (strcmp(suffix, ".svg") == 0) {
                                outputType = 3;
                            } else if (strcmp(suffix, ".png") == 0) {
                                outputType = 4;
                            }
                        }
                    }
                }
            }
        }

        PPKGDependsOutputType outputTypeEnum;

        switch (outputType) {
            case 0: outputTypeEnum = PPKGDependsOutputType_BOX; break;
            case 1: outputTypeEnum = PPKGDependsOutputType_DOT; break;
            case 2: outputTypeEnum = PPKGDependsOutputType_BOX; break;
            case 3: outputTypeEnum = PPKGDependsOutputType_SVG; break;
            case 4: outputTypeEnum = PPKGDependsOutputType_PNG; break;
        }


        int ret = ppkg_depends(argv[2], outputTypeEnum, outputFilePath);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> is unspecified.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> should be a non-empty string.\n", argv[0]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        free(outputFilePath);

        return ret;
    }


    if (strcmp(argv[1], "fetch") == 0) {
        int ret = ppkg_fetch(argv[2], verbose);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "install") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        PPKGInstallOptions options = {0};

        options.logLevel = PPKGLogLevel_normal;

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-q") == 0) {
                options.logLevel = PPKGLogLevel_silent;
            } else if (strcmp(argv[i], "-v") == 0) {
                options.logLevel = PPKGLogLevel_verbose;
            } else if (strcmp(argv[i], "-vv") == 0) {
                options.logLevel = PPKGLogLevel_very_verbose;
            } else if (strcmp(argv[i], "--dry-run") == 0) {
                options.dryrun = true;
            } else if (strcmp(argv[i], "--keep-installing-dir") == 0) {
                options.keepInstallingDir = true;
            } else if (strcmp(argv[i], "--export-compile-commands-json") == 0) {
                options.exportCompileCommandsJson = true;
            } else if (strcmp(argv[i], "--enable-ccache") == 0) {
                options.enableCcache = true;
            } else if (strcmp(argv[i], "--enable-bear") == 0) {
                options.enableBear = true;
            } else if (strcmp(argv[i], "--build-type=debug") == 0) {
                options.buildType = PPKGBuildType_debug;
            } else if (strcmp(argv[i], "--build-type=release") == 0) {
                options.buildType = PPKGBuildType_release;
            } else if (strcmp(argv[i], "--link-type=static-only") == 0) {
                options.linkType = PPKGLinkType_static_only;
            } else if (strcmp(argv[i], "--link-type=shared-only") == 0) {
                options.linkType = PPKGLinkType_shared_only;
            } else if (strcmp(argv[i], "--link-type=static-prefered") == 0) {
                options.linkType = PPKGLinkType_static_prefered;
            } else if (strcmp(argv[i], "--link-type=shared-prefered") == 0) {
                options.linkType = PPKGLinkType_shared_prefered;
            } else if (strncmp(argv[i], "--jobs=", 7) == 0) {
                char * jobsStr = &argv[i][7];

                if (jobsStr[0] == '\0') {
                    LOG_ERROR1("--jobs=<N> , <N> should be a non-empty string");
                    return PPKG_ERROR;
                } else {
                    int i = 0;

                    for (;;) {
                        char c = jobsStr[i];

                        if (c == '\0') {
                            break;
                        }

                        if ((c >= '0') && (c <= '9')) {
                            i++;
                        } else {
                            LOG_ERROR1("--jobs=<N> , <N> should be a integer.");
                            return PPKG_ERROR;
                        }
                    }
                }

                options.parallelJobsCount = atoi(jobsStr);
            } else {
                int ret = ppkg_check_if_the_given_package_is_available(argv[i]);

                if (ret == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int ret = ppkg_install(packageName, options);

            if (ret == PPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (ret == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (ret != PPKG_OK) {
                return ret;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "uninstall") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                int ret = ppkg_check_if_the_given_package_is_installed(argv[i]);

                if (ret == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                    LOG_ERROR3("package [", argv[i], "] is not installed.");
                }

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int ret = ppkg_uninstall(packageName, verbose);

            if (ret == PPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (ret == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (ret != PPKG_OK) {
                return ret;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "reinstall") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        PPKGInstallOptions options = {0};

        options.logLevel = PPKGLogLevel_normal;

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-q") == 0) {
                options.logLevel = PPKGLogLevel_silent;
            } else if (strcmp(argv[i], "-v") == 0) {
                options.logLevel = PPKGLogLevel_verbose;
            } else if (strcmp(argv[i], "-vv") == 0) {
                options.logLevel = PPKGLogLevel_very_verbose;
            } else {
                int ret = ppkg_check_if_the_given_package_is_available(argv[i]);

                if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (ret != PPKG_OK) {
                    return ret;
                }

                ret = ppkg_check_if_the_given_package_is_installed(argv[i]);

                if (ret == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                }

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }

        if (packageNameIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int ret = ppkg_reinstall(packageName, options);

            if (ret == PPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (ret == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (ret != PPKG_OK) {
                return ret;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "upgrade") == 0) {
        PPKGInstallOptions options = {0};

        options.logLevel = PPKGLogLevel_normal;

        int packageNameIndexArray[argc];
        int packageNameIndexArraySize = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-q") == 0) {
                options.logLevel = PPKGLogLevel_silent;
            } else if (strcmp(argv[i], "-v") == 0) {
                options.logLevel = PPKGLogLevel_verbose;
            } else if (strcmp(argv[i], "-vv") == 0) {
                options.logLevel = PPKGLogLevel_very_verbose;
            } else {
                int ret = ppkg_check_if_the_given_package_is_outdated(argv[i]);

                if (ret == PPKG_OK) {
                    packageNameIndexArray[packageNameIndexArraySize] = i;
                    packageNameIndexArraySize++;
                } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                    LOG_ERROR4("package name [", argv[i], "] is not match pattern ", PPKG_PACKAGE_NAME_PATTERN);
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                    LOG_ERROR3("package [", argv[i], "] is not available.");
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                    LOG_ERROR3("package [", argv[i], "] is not installed.");
                } else if (ret == PPKG_ERROR_PACKAGE_NOT_OUTDATED) {
                    LOG_ERROR3("package [", argv[i], "] is not outdated.");
                }

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }

        for (int i = 0; i < packageNameIndexArraySize; i++) {
            char * packageName = argv[packageNameIndexArray[i]];

            int ret = ppkg_upgrade(packageName, options);

            if (ret == PPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
            } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package [%s] is not available.\n", packageName);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package [%s] is not installed.\n", packageName);
            } else if (ret == PPKG_ERROR_PACKAGE_NOT_OUTDATED) {
                fprintf(stderr, "package [%s] is not outdated.\n", packageName);
            } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
                fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
            } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
                fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
            } else if (ret == PPKG_ERROR) {
                fprintf(stderr, "occurs error.\n");
            }

            if (ret != PPKG_OK) {
                return ret;
            }
        }

        return PPKG_OK;
    }

    if (strcmp(argv[1], "ls-available") == 0) {
        int ret = ppkg_show_the_available_packages();

        if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "ls-installed") == 0) {
        int ret = ppkg_list_the_installed_packages();

        if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "ls-outdated") == 0) {
        int ret = ppkg_list_the_outdated__packages();

        if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "is-available") == 0) {
        int ret = ppkg_check_if_the_given_package_is_available(argv[2]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "is-installed") == 0) {
        int ret = ppkg_check_if_the_given_package_is_installed(argv[2]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "is-outdated") == 0) {
        int ret = ppkg_check_if_the_given_package_is_outdated(argv[2]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "formula-repo-list") == 0) {
        return ppkg_formula_repo_list_printf();
    }

    if (strcmp(argv[1], "formula-repo-add") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = 0;
        int enabled = 1;

        const char * branch = NULL;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return PPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return ppkg_formula_repo_add(argv[2], argv[3], branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-init") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = 1;
        int enabled = 1;

        char * branch = NULL;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return PPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return ppkg_formula_repo_create(argv[2], argv[3], branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-del") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        return ppkg_formula_repo_remove(argv[2]);
    }

    if (strcmp(argv[1], "formula-repo-sync") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        return ppkg_formula_repo_sync_(argv[2]);
    }

    if (strcmp(argv[1], "formula-repo-conf") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = -1;
        int enabled = -1;

        const char * branch = NULL;
        const char * url = NULL;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--url=", 6) == 0) {
                if (argv[i][6] == '\0') {
                    fprintf(stderr, "--url=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                } else {
                    url = &argv[i][6];
                }
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return PPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return PPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return ppkg_formula_repo_config(argv[2], url, branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-info") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        return ppkg_formula_repo_info_(argv[2]);
    }

    if (strcmp(argv[1], "integrate") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s integrate <zsh|bash|fish>\n", argv[0]);
            return PPKG_ERROR_ARG_IS_NULL;
        } else if (strcmp(argv[2], "zsh") == 0) {
            return ppkg_integrate_zsh_completion (NULL, verbose);
        } else if (strcmp(argv[2], "bash") == 0) {
            return ppkg_integrate_bash_completion(NULL, verbose);
        } else if (strcmp(argv[2], "fish") == 0) {
            return ppkg_integrate_fish_completion(NULL, verbose);
        } else {
            LOG_ERROR2("unrecognized argument: ", argv[2]);
            return PPKG_ERROR_ARG_IS_INVALID;
        }
    }

    if (strcmp(argv[1], "upgrade-self") == 0) {
        return ppkg_upgrade_self(verbose);
    }

    if (strcmp(argv[1], "cleanup") == 0) {
        return ppkg_cleanup(verbose);
    }

    if (strcmp(argv[1], "gen-url-transform-sample") == 0) {
        return ppkg_generate_url_transform_sample();
    }

    if (strcmp(argv[1], "logs") == 0) {
        int ret = ppkg_logs(argv[2]);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "pack") == 0) {
        int ret = ppkg_pack(argv[2], ArchiveType_tar_xz, verbose);

        if (ret == PPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == PPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [--type=tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], PPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == PPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == PPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == PPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == PPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "util") == 0) {
        return ppkg_util(argc, argv);
    }

    LOG_ERROR2("unrecognized action: ", argv[1]);
    return PPKG_ERROR_ARG_IS_UNKNOWN;
}

int main(int argc, char* argv[]) {
    return ppkg_main(argc, argv);
}
