import QtQuick 2.0
import Enginio 1.0
import "qrc:///config.js" as AppConfig

import QtQuick.Dialogs 1.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

/*
 * Enginio image gallery example.
 *
 * Main window contains list of enginioModel on the backend and button for uploading
 * new enginioModel. Image list contains image thumbnail (generated by Enginio
 * backend) and some image metadata. Clicking list items downloads image file
 * and displays it in dialog window. Clicking the red x deletes enginioModel from
 * backend.
 *
 * In the backend enginioModel are represented as objects of type "objects.image". These
 * objects contain a property "file" which is a reference to the actual binary file.
 */

Rectangle {
    width: 500
    height: 700

    // Enginio client specifies the backend to be used
    //! [client]
    Enginio {
        id: client
        backendId: AppConfig.backendData.id
        backendSecret: AppConfig.backendData.secret
        onError: console.log("Enginio error: " + reply.errorCode + ": " + reply.errorString)
    }
    //! [client]

    //! [model]
    EnginioModel {
        id: enginioModel
        enginio: client
        query: { // query for all objects of type "objects.image" and include references to files
            "objectType": "objects.image",
            "include": {"file": {}},
        }
    }
    //! [model]

    // Delegate for displaying individual rows of the model
    Component {
        id: imageListDelegate

        Rectangle {
            height: 120
            width: parent.width
            color: index % 2 ? "#eeeeee" : "transparent"
            //! [image-fetch]
            Image {
                id: image
                x: 10
                y: 10
                height: parent.height - 20
                width: 80
                smooth: true
                fillMode: Image.PreserveAspectFit
                Component.onCompleted: {
                    var data = { "id": file.id,
                                 "variant": "thumbnail"}
                    var reply = client.downloadFile(data)
                    reply.finished.connect(function() {
                        image.source = reply.data.expiringUrl
                    })
                }
            }
            //! [image-fetch]
            ProgressBar {
                height: 10
                width: image.width - 20
                anchors.centerIn: image

                maximumValue: 1.0
                minimumValue: 0
                value: image.progress

                Layout.fillWidth: true
                visible: image.status != Image.Ready
            }

            Column {
                x: 100
                y: 10
                Text {
                    height: 33
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.5
                    text: name ? name : ""
                }
                Text {
                    height: 33
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.5
                    text: sizeStringFromFile(file)
                }
                Text {
                    height: 33
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: height * 0.5
                    text: timeStringFromFile(file)
                }
            }

            // Clicking list item opens full size image in separate dialog
            MouseArea {
                id: hitbox
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    imageDialog.fileId = file.id;
                    imageDialog.visible = true
                }
                onContainsMouseChanged: deleteIcon.opacity = containsMouse ? 1.0 : 0.0
            }

            // Delete button
            Image {
                id: deleteIcon
                width: 32
                height: 32
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                opacity: 0.0
                source: "qrc:icons/delete_icon.png"
                Behavior on opacity { NumberAnimation { duration: 200 } }
                MouseArea {
                    anchors.fill: parent
                    onClicked: enginioModel.remove(index)
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        ListView {
            id: imageListView
            model: enginioModel // get the data from EnginioModel
            delegate: imageListDelegate
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
        }
        RowLayout {
            width: parent.width
            Button {
                text: "Upload new image..."
                onClicked: fileDialog.visible = true;
            }
            ProgressBar {
                id: progressBar
                visible: false
                maximumValue: 1.0

                Layout.fillWidth: true
            }
        }
    }

    // Dialog for showing full size image
    Rectangle {
        id: imageDialog
        anchors.fill: parent
        property string fileId
        color: "#646464"
        visible: false
        opacity: visible ? 1.0 : 0.0

        onFileIdChanged: {
            image.source = ""
            // Download the full image, not the thumbnail
            var data = { "id": fileId }
            var reply = client.downloadFile(data)
            reply.finished.connect(function() {
                image.source = reply.data.expiringUrl
            })
        }

        Label {
            id: label
            text: "Loading ..."
            color: "white"
            anchors.centerIn: parent
            visible: image.status != Image.Ready
        }
        ProgressBar {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: label.bottom
            anchors.topMargin: 10
            width: parent.width / 3
            value: image.progress
            visible: image.status != Image.Ready
        }
        Image {
            id: image
            anchors.fill: parent
            anchors.margins: 10
            smooth: true
            cache: false
            fillMode: Image.PreserveAspectFit
        }
        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: parent.visible = false
        }
    }

    // File dialog for selecting image file from local file system
    FileDialog {
        id: fileDialog
        title: "Select image file to upload"
        nameFilters: [ "Image files (*.png *.jpg *.jpeg)", "All files (*)" ]

        onSelectionAccepted: {
            var pathParts = fileUrl.toString().split("/");
            var fileName = pathParts[pathParts.length - 1];
            var fileObject = {
                objectType: "objects.image",
                name: fileName,
                localPath: fileUrl.toString()
            }
            var reply = client.create(fileObject);
            reply.finished.connect(function() {
                var uploadData = {
                    file: { fileName: fileName },
                    targetFileProperty: {
                        objectType: "objects.image",
                        id: reply.data.id,
                        propertyName: "file"
                    },
                };
                var uploadReply = client.uploadFile(uploadData, fileUrl)
                progressBar.visible = true
                uploadReply.progress.connect(function(progress, total) {
                    progressBar.value = progress/total
                })
                uploadReply.finished.connect(function() {
                    var tmp = enginioModel.query; enginioModel.query = {}; enginioModel.query = tmp;
                    progressBar.visible = false
                })
            })
        }
    }

    function sizeStringFromFile(fileData) {
        var str = [];
        if (fileData && fileData.fileSize) {
            str.push("Size: ");
            str.push(fileData.fileSize);
            str.push(" bytes");
        }
        return str.join("");
    }

    function doubleDigitNumber(number) {
        if (number < 10)
            return "0" + number;
        return number;
    }

    function timeStringFromFile(fileData) {
        var str = [];
        if (fileData && fileData.createdAt) {
            var date = new Date(fileData.createdAt);
            if (date) {
                str.push("Uploaded: ");
                str.push(date.toDateString());
                str.push(" ");
                str.push(doubleDigitNumber(date.getHours()));
                str.push(":");
                str.push(doubleDigitNumber(date.getMinutes()));
            }
        }
        return str.join("");
    }
}
