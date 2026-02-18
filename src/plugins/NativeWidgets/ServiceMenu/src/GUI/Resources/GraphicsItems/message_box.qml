import QtQuick 2.15
import Core.Types 1.0

Item {
    id: rootItem
    width: 900
    height: 480 // Increased to fit massive text comfortably

    Rectangle {
        id: card
        anchors.fill: parent
        color: "#0A0A0B"
        radius: 45
        border.width: 4
        border.color: "#3A3A3C"

        // --- THE CHARM (Hairline Glint) ---
        Rectangle {
            anchors.fill: parent
            anchors.margins: 2
            radius: 43
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(255, 255, 255, 0.15)
        }

        // --- MAIN CONTENT ---
        Row {
            id: mainContent
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: 45
                topMargin: 50
            }
            spacing: 40

            Item {
                id: iconContainer
                width: 170
                height: 170
                anchors.verticalCenter: textColumn.verticalCenter

                Image {
                    id: imageMain
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                    visible: !imageWait.visible
                }

                Item {
                    id: imageWait
                    anchors.fill: parent
                    visible: false
                    Rectangle {
                        anchors.fill: parent
                        color: "#1C1C1E"
                        radius: 40
                        border.color: "#FA5300"
                        border.width: 2
                        AnimatedImage {
                            anchors.centerIn: parent
                            width: 90
                            height: 90
                            source: "qrc:/Images/MessageBox/wait.gif"
                        }
                    }
                }
            }

            Column {
                id: textColumn
                width: mainContent.width - iconContainer.width - mainContent.spacing
                spacing: 16

                Text {
                    id: textMessage
                    width: parent.width
                    color: "#FFFFFF"
                    // --- MONSTROUS OUTDOOR SIZE ---
                    font {
                        pixelSize: 52
                        weight: Font.Black
                        family: "Segoe UI, Arial"
                    }
                    wrapMode: Text.WordWrap
                    // High-Contrast Sunlight Outline
                    style: Text.Outline
                    styleColor: "#40000000"
                }

                Flickable {
                    id: flick
                    width: parent.width
                    height: 100
                    contentHeight: textMessageExt.paintedHeight
                    clip: true
                    visible: textMessageExt.text !== ""

                    TextEdit {
                        id: textMessageExt
                        width: flick.width
                        wrapMode: TextEdit.Wrap
                        color: "#8E8E93"
                        font.pixelSize: 24
                        readOnly: true
                    }
                }
            }
        }

        // --- BUTTON BAR (Massive Touch Targets) ---
        Row {
            id: buttonBar
            anchors {
                bottom: parent.bottom
                right: parent.right
                margins: 40
            }
            spacing: 25

            Rectangle {
                id: btnCancel
                width: 260
                height: 95
                color: "#1C1C1E"
                radius: 25
                visible: false
                border.color: "#3A3A3C"
                border.width: 1

                Text {
                    id: btnCancelText
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
                    onClicked: Core.graphics.hidePopup()
                }
            }

            Rectangle {
                id: btnOK
                width: 260
                height: 95
                radius: 25
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
                    onClicked: {
                        Core.graphics.hidePopup({
                            button: MessageBox.OK
                        });
                        Core.postEvent(EventType.UpdateScenario, {
                            signal: "popup_notify"
                        });
                    }
                }
            }
        }
    }

    QtObject {
        id: global
        property bool closeWindow: false
    }

    function resetHandler(aParameters) {
        btnOK.visible = false;
        btnCancel.visible = false;
        imageWait.visible = false;
        textMessage.text = aParameters["text_message"] || "";
        textMessageExt.text = aParameters["text_message_ext"] || "";

        // Map to 2026 Industrial SVG Suite
        var iconMap = {
            [MessageBox.Critical]: "qrc:/GraphicsItems/images/critical_industrial.svg",
            [MessageBox.Warning]: "qrc:/GraphicsItems/images/warning_industrial.svg",
            [MessageBox.Question]: "qrc:/GraphicsItems/images/question_industrial.svg",
            [MessageBox.Info]: "qrc:/GraphicsItems/images/info_industrial.svg"
        };

        imageMain.source = iconMap[aParameters["icon"]] || "";
        imageWait.visible = (aParameters["icon"] === MessageBox.Wait);
        imageMain.visible = !imageWait.visible;

        if (aParameters["button"] === MessageBox.OK || aParameters["icon"] === MessageBox.Question)
            btnOK.visible = true;
        if (aParameters["button"] === MessageBox.Cancel || aParameters["icon"] === MessageBox.Question)
            btnCancel.visible = true;
    }
}
