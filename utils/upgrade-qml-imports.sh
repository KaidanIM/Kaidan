#!/bin/bash

SCRIPT_DIR=$(dirname "${0}")

if [ "${SCRIPT_DIR:0:1}" != "/" ]; then
    SCRIPT_DIR=${PWD}/${SCRIPT_DIR}
fi

SOURCE_DIR="${SCRIPT_DIR}/../src"

# Force early exit on issue
set -e
# Force debug traces of executed commands
#set -o xtrace

# To see current imports:
# find "${SOURCE_DIR}" -iname '*.qml' -exec grep 'import ' {} \; | sort -u

declare -A QT
QT["QtGraphicalEffects"]="1.12"
QT["Qt.labs.folderlistmodel"]="2.1"
QT["QtLocation"]="5.12"
QT["QtMultimedia"]="5.12"
QT["QtPositioning"]="5.12"
QT["QtQml"]="2.12"
QT["QtQuick"]="2.12"
QT["QtQuick.Controls"]="2.12"
QT["QtQuick.Controls.Material"]="2.12"
QT["QtQuick.Dialogs"]="1.12"
QT["QtQuick.Layouts"]="1.12"

declare -A KIRIGAMI
KIRIGAMI["org.kde.kirigami"]="2.8"

declare -A KAIDAN
KAIDAN["EmojiModel"]="0.1"
KAIDAN["MediaUtils"]="0.1"
KAIDAN["StatusBar"]="0.1"
KAIDAN["im.kaidan.kaidan"]="1.0"

SED_REPLACES=

for key in "${!QT[@]}"
do
    SED_REPLACES="${SED_REPLACES} -e 's/[[:space:]]*import[[:space:]]+${key//./\\.}[[:space:]]+[0-9]+\.[0-9]+/import ${key} ${QT[${key}]}/'"
done

for key in "${!KIRIGAMI[@]}"
do
    SED_REPLACES="${SED_REPLACES} -e 's/[[:space:]]*import[[:space:]]+${key//./\\.}[[:space:]]+[0-9]+\.[0-9]+/import ${key} ${KIRIGAMI[${key}]}/'"
done

for key in "${!KAIDAN[@]}"
do
    SED_REPLACES="${SED_REPLACES} -e 's/[[:space:]]*import[[:space:]]+${key//./\\.}[[:space:]]+[0-9]+\.[0-9]+/import ${key} ${KAIDAN[${key}]}/'"
done

SED_CMD="sed -Ei'' ${SED_REPLACES}"

while IFS= read -r qml_file; do
    echo "Updating ${qml_file}..."
    eval ${SED_CMD} "${qml_file}"
done < <( find "${SOURCE_DIR}" -type f -iname '*.qml' )
