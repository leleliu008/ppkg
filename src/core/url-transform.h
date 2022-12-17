#ifndef URL_TRANSFORM_H
#define URL_TRANSFORM_H

#define URL_TRANSFORM_OK                0
#define URL_TRANSFORM_ERROR             101
#define URL_TRANSFORM_RUN_EMPTY_RESULT  102
#define URL_TRANSFORM_ENV_IS_NOT_SET    103
#define URL_TRANSFORM_ENV_VALUE_IS_EMPTY      104
#define URL_TRANSFORM_ENV_VALUE_PATH_NOT_EXIST 105

int url_transform(const char * inUrl, char * * outUrlPointer);

#endif
