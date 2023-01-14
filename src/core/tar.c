#include <string.h>
#include <locale.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tar.h"

int tar_list(const char * inputFilePath, int flags) {
	if (inputFilePath != NULL && strcmp(inputFilePath, "-") == 0) {
		inputFilePath = NULL;
    }

    struct archive *ar = archive_read_new();
    struct archive *aw = archive_write_disk_new();

    archive_read_support_format_all(ar);
    archive_read_support_filter_all(ar);

    archive_write_disk_set_options(aw, flags);

    struct archive_entry *entry = NULL;

    int resultCode = 0;

	if ((resultCode = archive_read_open_filename(ar, inputFilePath, 10240))) {
        goto clean;
    }

	for (;;) {
		resultCode = archive_read_next_header(ar, &entry);

		if (resultCode == ARCHIVE_EOF) {
			break;
        }

		if (resultCode == ARCHIVE_OK) {
		    printf("%s\n", archive_entry_pathname(entry));
        } else {
            goto clean;
        }
	}

clean:
    if (resultCode != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
    }

	archive_read_close(ar);
	archive_read_free(ar);

	archive_write_close(aw);
  	archive_write_free(aw);

    return resultCode;
}

int tar_extract(const char * outputDir, const char * inputFilePath, int flags, bool verbose, size_t stripComponentsNumber) {
    if (inputFilePath != NULL && strcmp(inputFilePath, "-") == 0) {
		inputFilePath = NULL;
    }

    // https://github.com/libarchive/libarchive/issues/459
    setlocale(LC_ALL, "");

	struct archive *ar = archive_read_new();
	struct archive *aw = archive_write_disk_new();

    archive_read_support_format_all(ar);
    archive_read_support_filter_all(ar);

	archive_write_disk_set_options(aw, flags);

	struct archive_entry *entry = NULL;

	int resultCode = 0;

	if ((resultCode = archive_read_open_filename(ar, inputFilePath, 10240))) {
        goto clean;
    }

	for (;;) {
		resultCode = archive_read_next_header(ar, &entry);

		if (resultCode == ARCHIVE_EOF) {
            resultCode =  ARCHIVE_OK;
			break;
        }

		if (resultCode != ARCHIVE_OK) {
            goto clean;
        }

        ////////////////////////////////////////////////////////////////////////////////////

        const char * entry_pathname = archive_entry_pathname(entry);

        if (stripComponentsNumber > 0) {
            size_t entry_pathname_length = strlen(entry_pathname);

            for (size_t i = 0; i < entry_pathname_length; i++) {
                if (entry_pathname[i] == '/') {
                    entry_pathname = entry_pathname + i + 1;
                    break;
                }
            }

            if (strcmp(entry_pathname, "") == 0) {
                continue;
            }
        }

		if (verbose) {
			printf("x %s\n", entry_pathname);
        }

        if ((outputDir != NULL) && (strcmp(outputDir, "") != 0)) {
            size_t outputFilePathLength = strlen(outputDir) + strlen(entry_pathname) + 2;
            char   outputFilePath[outputFilePathLength];
            memset(outputFilePath, 0, outputFilePathLength);
            snprintf(outputFilePath, outputFilePathLength, "%s/%s", outputDir, entry_pathname);

            archive_entry_set_pathname(entry, outputFilePath);
        } else {
            archive_entry_set_pathname(entry, entry_pathname);
        }

        ////////////////////////////////////////////////////////////////////////////////////

        const char * hardlinkname = archive_entry_hardlink(entry);

        if (hardlinkname != NULL) {
            if (stripComponentsNumber > 0) {
                size_t hardlinkname_length = strlen(hardlinkname);
                for (size_t i = 0; i < hardlinkname_length; i++) {
                    if (hardlinkname[i] == '/') {
                        hardlinkname = hardlinkname + i + 1;
                        break;
                    }
                }

                if ((outputDir != NULL) && (strcmp(outputDir, "") != 0)) {
                    size_t outputFilePathLength = strlen(outputDir) + strlen(hardlinkname) + 2;
                    char   outputFilePath[outputFilePathLength];
                    memset(outputFilePath, 0, outputFilePathLength);
                    snprintf(outputFilePath, outputFilePathLength, "%s/%s", outputDir, hardlinkname);

                    archive_entry_set_hardlink_utf8(entry, outputFilePath);
                } else {
                    archive_entry_set_hardlink_utf8(entry, hardlinkname);
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////

        resultCode = archive_write_header(aw, entry);

        if (resultCode != ARCHIVE_OK) {
            goto clean;
        }

        const void * dataBuff;
        size_t       dataSize;
#if ARCHIVE_VERSION_NUMBER >= 3000000
        int64_t offset;
#else
        off_t   offset;
#endif

        for (;;) {
            resultCode = archive_read_data_block(ar, &dataBuff, &dataSize, &offset);

            if (resultCode == ARCHIVE_EOF) {
                break;
            }

            if (resultCode != ARCHIVE_OK) {
                goto clean;
            }

            resultCode = archive_write_data_block(aw, dataBuff, dataSize, offset);

            if (resultCode != ARCHIVE_OK) {
                goto clean;
            }
        }

        resultCode = archive_write_finish_entry(aw);

        if (resultCode != ARCHIVE_OK) {
            goto clean;
        }
	}

clean:
    if (resultCode != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
    }

	archive_read_close(ar);
	archive_read_free(ar);
	
	archive_write_close(aw);
  	archive_write_free(aw);

    return resultCode;
}

typedef struct {
    char ** array;
    size_t  size;
    size_t  capcity;
} StringArrayList;

int list_files(const char * dirPath, bool verbose, StringArrayList * stringArrayList) {
    if ((dirPath == NULL) || (strcmp(dirPath, "") == 0)) {
        return -1;
    }

    DIR * dir;
    struct dirent * dir_entry;

    dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return -2;
    }

    int r = 0;

    while ((dir_entry = readdir(dir))) {
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = strlen(dirPath) + strlen(dir_entry->d_name) + 2;
        char   filePath[filePathLength];
        memset(filePath, 0, filePathLength);
        snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        //if (verbose) printf("%s\n", filePath);

        struct stat st;

        r = stat(filePath, &st);

        if (r == 0) {
            if (S_ISDIR(st.st_mode)) {
                r = list_files(filePath, verbose, stringArrayList);

                if (r != 0) {
                    break;
                }
            } else {
                if (stringArrayList->size == stringArrayList->capcity) {
                    stringArrayList->capcity += 10;
                    stringArrayList->array = (char**)realloc(stringArrayList->array, (stringArrayList->capcity) * sizeof(char*));

                    if (stringArrayList->array == NULL) {
                        r = -3;
                        break;
                    }
                }
                stringArrayList->array[stringArrayList->size] = strdup(filePath);
                stringArrayList->size += 1;
            }
        } else {
            break;
        }
    }

    closedir(dir);

    return r;
}

int tar_create(const char * inputDir, const char * outputFilePath, ArchiveType type, bool verbose) {
    StringArrayList stringArrayList = {0};

    int resultCode = list_files(inputDir, verbose, &stringArrayList);

    if (resultCode != 0) {
        return resultCode;
    }

    if (stringArrayList.size == 0) {
        return 1;
    }

    if (outputFilePath != NULL && strcmp(outputFilePath, "-") == 0) {
		outputFilePath = NULL;
    }

    // https://github.com/libarchive/libarchive/issues/459
    setlocale(LC_ALL, "");

	struct archive *aw = archive_write_new();

    switch (type) {
        case ArchiveType_tar_gz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_gzip(aw);
            break;
        case ArchiveType_tar_lz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_lzip(aw);
            break;
        case ArchiveType_tar_xz:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_xz(aw);
            break;
        case ArchiveType_tar_bz2:
            archive_write_set_format_ustar(aw);
            archive_write_add_filter_bzip2(aw);
            break;
        case ArchiveType_zip:
            archive_write_set_format_zip(aw);
            break;
    }

    archive_write_open_filename(aw, outputFilePath);

    struct archive       *ar    = NULL;
	struct archive_entry *entry = NULL;

    int fd;

    for (size_t i = 0; i < stringArrayList.size; i++) {
        ar = archive_read_disk_new();

        resultCode = archive_read_disk_open(ar, stringArrayList.array[i]);

        if (resultCode != ARCHIVE_OK) {
            goto clean;
        }

        for (;;) {
            entry = archive_entry_new();

            resultCode = archive_read_next_header2(ar, entry);

            if (resultCode == ARCHIVE_EOF) {
                resultCode =  ARCHIVE_OK;
                archive_entry_free(entry);
                break;
            }

            if (resultCode != ARCHIVE_OK) {
                archive_entry_free(entry);
                goto clean;
            }

            archive_read_disk_descend(ar);

            if (verbose) {
                fprintf(stderr, "a %s\n", archive_entry_pathname(entry));
            }

            resultCode = archive_write_header(aw, entry);

            if (resultCode == ARCHIVE_RETRY || resultCode == ARCHIVE_WARN) {
                resultCode =  ARCHIVE_OK;
            }

            if (resultCode != ARCHIVE_OK) {
                archive_entry_free(entry);
                goto clean;
            }

            fd = open(archive_entry_sourcepath(entry), O_RDONLY);

            char buff[16384];
            size_t len;

            len = read(fd, buff, sizeof(buff));

            while (len > 0) {
                archive_write_data(aw, buff, len);
                len = read(fd, buff, sizeof(buff));
            }

            close(fd);

            archive_entry_free(entry);
        }

        archive_read_close(ar);
        archive_read_free(ar);
    }

clean:
    if (resultCode != ARCHIVE_OK) {
        fprintf(stdout, "%s\n", archive_error_string(ar));
        archive_read_close(ar);
        archive_read_free(ar);
    }

	archive_write_close(aw);
  	archive_write_free(aw);

    if (stringArrayList.size > 0) {
        for (size_t i = 0; i < stringArrayList.size; i++) {
            free(stringArrayList.array[i]);
            stringArrayList.array[i] = NULL;
        }
        free(stringArrayList.array);
        stringArrayList.array = NULL;
    }

    return resultCode;
}
