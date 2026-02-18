/*
 * @file Humo Kiosk 2026 - Ultra-Readable Outdoor Popup
 * Optimized for: Extreme Sunlight, 1280x1024 Resolution, 2026 Branding
 */

import QtQuick 2.15
import Application.Types 1.0

Rectangle {
    id: rootItem
    width: 900
    height: 480 // Increased height to support "Monstrous" text sizes
    color: "#0A0A0B"
    radius: 45
    border.width: 4
    border.color: "#3A3A3C"

    // --- MAIN CONTENT: 170px Icon + 56px Massive Text ---
    Row {
        id: mainContent
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 45
            topMargin: 55
        }
        spacing: 40

        Item {
            id: iconWrapper
            width: 170
            height: 170
            anchors.verticalCenter: textColumn.verticalCenter

            Image {
                id: imageMain
                anchors.fill: parent
                sourceSize.width: 170
                sourceSize.height: 170
                smooth: true
                mipmap: true
            }

            AnimatedImage {
                id: imageWait
                anchors.centerIn: parent
                width: 100
                height: 100
                source: "qrc:/MessageBox/wait.gif"
                visible: false
            }
        }

        Column {
            id: textColumn
            width: parent.width - iconWrapper.width - parent.spacing

            Text {
                id: textMessage
                width: parent.width
                color: "#FFFFFF"
                // --- RESTORED MASSIVE SIZE ---
                font {
                    pixelSize: 52 // High-visibility outdoor standard
                    weight: Font.Black
                    family: "Segoe UI, Arial"
                }
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignLeft
                style: Text.Outline // Adds contrast against glare
                styleColor: "#40000000"
            }
        }
    }

    // --- EXTENDED TEXT (Flickable) ---
    Flickable {
        id: flick
        anchors {
            top: mainContent.bottom
            left: parent.left
            right: parent.right
            bottom: buttonBar.top
            margins: 45
            topMargin: 15
            bottomMargin: 15
        }
        clip: true
        contentHeight: textMessageExt.paintedHeight
        visible: textMessageExt.text !== ""

        TextEdit {
            id: textMessageExt
            width: parent.width
            color: "#8E8E93"
            font.pixelSize: 24 // Increased for readability
            wrapMode: TextEdit.Wrap
            readOnly: true
        }
    }

    // --- BUTTON BAR: Massive Touch Targets ---
    Row {
        id: buttonBar
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: 45
        }
        spacing: 30

        Rectangle {
            id: btnCancel
            width: 260
            height: 95
            color: "#1C1C1E"
            radius: 24
            visible: false
            border.color: "#3A3A3C"

            Text {
                id: btnText
                anchors.centerIn: parent
                color: "#8E8E93"
                font {
                    pixelSize: 28
                    weight: Font.Bold
                }
                text: "#cancel"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: Application.graphics.hidePopup()
            }
        }

        Rectangle {
            id: btnOK
            width: 260
            height: 95
            radius: 24
            visible: false
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "#FF7D40"
                }
                GradientStop {
                    position: 1.0
                    color: "#FA5300"
                }
            }

            Text {
                anchors.centerIn: parent
                text: "OK"
                color: "#FFFFFF"
                font {
                    pixelSize: 32
                    weight: Font.Black
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: Application.graphics.hidePopup({
                    button: MessageBox.OK
                })
            }
        }
    }

    // --- LOGIC MAPPING ---
    function resetHandler(aParameters) {
        textMessage.text = aParameters["text_message"] || "";
        textMessageExt.text = aParameters["text_message_ext"] || "";

        var iconMap = {
            [MessageBox.Critical]: "qrc:/images/critical_industrial.svg",
            [MessageBox.Warning]: "qrc:/images/warning_industrial.svg",
            [MessageBox.Question]: "qrc:/images/question_industrial.svg",
            [MessageBox.Info]: "qrc:/images/info_industrial.svg"
        };

        imageMain.source = iconMap[aParameters["icon"]] || "";
        imageWait.visible = (aParameters["icon"] === MessageBox.Wait);
        imageMain.visible = !imageWait.visible;

        btnOK.visible = (aParameters["button"] === MessageBox.OK || aParameters["icon"] === MessageBox.Question);
        btnCancel.visible = (aParameters["button"] === MessageBox.Cancel || aParameters["icon"] === MessageBox.Question);
    }
}
