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

# find "${SOURCE_DIR}" -iname '*.qml' -exec grep 'import ' {} \; | sort -u
# import "elements"
# import EmojiModel 0.1
# import im.kaidan.kaidan 1.0
# import MediaUtils 0.1
# import org.kde.kirigami 2.0 as Kirigami
# import org.kde.kirigami 2.2 as Kirigami
# import org.kde.kirigami 2.3 as Kirigami
# import org.kde.kirigami 2.4 as Kirigami
# import org.kde.kirigami 2.5 as Kirigami
# import QtGraphicalEffects 1.0
# import Qt.labs.folderlistmodel 2.1
# import QtLocation 5.9 as Location
# import QtMultimedia 5.10 as Multimedia
# import QtMultimedia 5.9
# import QtMultimedia 5.9 as Multimedia
# import QtPositioning 5.9 as Positioning
# import QtQml 2.2
# import QtQuick 2.0
# import QtQuick 2.3
# import QtQuick 2.6
# import QtQuick 2.7
# import QtQuick 2.9
# import QtQuick.Controls 2.0 as Controls
# import QtQuick.Controls 2.1 as Controls
# import QtQuick.Controls 2.2
# import QtQuick.Controls 2.2 as Controls
# import QtQuick.Controls 2.3 as Controls
# import QtQuick.Controls 2.4 as Controls
# import QtQuick.Controls.Material 2.0
# import QtQuick.Dialogs 1.2
# import QtQuick.Layouts 1.1
# import QtQuick.Layouts 1.2
# import QtQuick.Layouts 1.3
# import "settings"
# import StatusBar 0.1

declare -A QT # 5.10
QT["QtGraphicalEffects"]="1.0"
QT["Qt.labs.folderlistmodel"]="2.1"
QT["QtLocation"]="5.3"
QT["QtMultimedia"]="5.8"
QT["QtPositioning"]="5.2"
QT["QtQml"]="2.2"
QT["QtQuick"]="2.7"
QT["QtQuick.Controls"]="2.3"
QT["QtQuick.Controls.Material"]="2.3"
QT["QtQuick.Dialogs"]="1.3"
QT["QtQuick.Layouts"]="1.3"

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
