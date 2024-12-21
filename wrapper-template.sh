#!/bin/sh
set -e
CWD="\$(dirname "\$0")"
cd "\$CWD"
CWD="\$PWD"
cd - > /dev/null
exec "\$CWD/$RELATIVE_PATH/$DYNAMIC_LOADER_FILENAME" --library-path "\$CWD/$RELATIVE_PATH" "\$CWD/${FILEPATH##*/}.exe" "\$@"
