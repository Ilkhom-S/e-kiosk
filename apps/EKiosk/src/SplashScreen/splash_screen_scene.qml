/*
 * @file Humo Kiosk 2026 - Master Splash Screen
 * Optimized for: Outdoor Sunlight, Windows 7 (1280x1024), Qt 5/6 Compatible
 * Theme: Midnight Rich UI (High-Contrast Orange/Black)
 * Note: No external plugin dependencies (No QtGraphicalEffects)
 */

import QtQuick 2.6
import Core.Types 1.0

Rectangle {
    id: root
    width: Core.graphics.width
    height: Core.graphics.height
    // Deep Midnight Black absorbs glare and provides a "Rich" premium feel
    color: "#0A0A0B"

    // --- MAIN UI LAYOUT ---
    Column {
        width: 1000
        anchors.centerIn: parent
        spacing: 70

        // 1. Branding: White Variant Humo Logo (QRC)
        Image {
            id: mainLogo
            width: 520
            sourceSize.width: 520 // Fixes SVG sharpness for Windows 7 / Qt 5
            fillMode: Image.PreserveAspectFit
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/GraphicsItems/images/humo-logo-light.svg"
            smooth: true
            asynchronous: true
        }

        // 2. Status Indicators: Single Row with Guaranteed Breathing Room
        Row {
            id: statusRow
            anchors.horizontalCenter: parent.horizontalCenter

            // LOGIC: Maintain at least 15px gap, but allow up to 35px if space permits
            spacing: Math.max(15, Math.min(35, (1000 - (statusModel.count * iconWidth)) / Math.max(1, statusModel.count - 1)))

            // LOGIC: Icon shrinks from 190px down to ~85px to accommodate all 10 + spacing
            readonly property int iconWidth: {
                if (statusModel.count <= 1)
                    return 190;
                let availableWidth = 1000 - (spacing * (statusModel.count - 1));
                return Math.min(190, availableWidth / statusModel.count);
            }

            Repeater {
                model: statusModel
                delegate: Rectangle {
                    width: statusRow.iconWidth
                    height: width // Keep it square
                    color: "#1C1C1E"
                    radius: width * 0.22 // Scale radius with size
                    border.color: "#FA5300"
                    border.width: width > 120 ? 5 : 3

                    // Add a tiny inner margin so the SVG doesn't touch the border
                    Item {
                        anchors.fill: parent
                        anchors.margins: parent.width * 0.15
                        Image {
                            anchors.fill: parent
                            sourceSize.width: width
                            sourceSize.height: height
                            fillMode: Image.PreserveAspectFit
                            source: model.image ? "qrc:/GraphicsItems/images/" + model.image : ""
                            smooth: true
                        }
                    }
                }
            }
        }

        // 3. Information Labels (Pure White for Sunlight Contrast)
        Column {
            spacing: 30
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                text: qsTr("#terminal_not_available")
                color: "#FFFFFF"
                font {
                    pixelSize: 56
                    weight: Font.Black // Maximum weight for sunlight legibility
                    family: "Segoe UI, Arial"
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 4. Warning Bar: Auto-Scaling Dimensions (Qt 5/6 Compatible)
            Item {
                id: warningContainer
                // Width scales with text but stays within 1000px
                width: Math.min(warningText.implicitWidth + 80, 1000)
                // Height scales with text plus 30px padding
                height: warningText.implicitHeight + 30
                anchors.horizontalCenter: parent.horizontalCenter
                visible: Boolean(global.parameters) && global.parameters.hasOwnProperty("firmware_upload")

                // Hardware-accelerated Shadow Layer
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -3
                    color: "#000000"
                    opacity: 0.35
                    radius: 14
                }

                // Main Orange Bar
                Rectangle {
                    anchors.fill: parent
                    color: "#FA5300"
                    radius: 10

                    Text {
                        id: warningText
                        anchors.centerIn: parent
                        width: parent.width - 40 // Horizontal padding
                        text: "⚠️ " + qsTr("#dont_power_off_terminal")
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap

                        font {
                            pixelSize: 38 // Slightly reduced for better fit
                            weight: Font.Black
                            family: "Segoe UI, Arial"
                        }

                        // Sunlight Contrast Outline
                        style: Text.Outline
                        styleColor: "#40000000"
                    }
                }
            }

            // Support Info (Subtle Gray but High-Weight)
            Column {
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    text: qsTr("#terminal").arg(Core.environment.terminal.AP)
                    color: "#8E8E93"
                    font {
                        pixelSize: 26
                        weight: Font.DemiBold
                    }
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: qsTr("#support").arg(Core.environment.dealer.phone)
                    color: "#FA5300" // Brand color for actionable info
                    font {
                        pixelSize: 28
                        weight: Font.Black
                    }
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // Расположение зон: 1   2
    //                     5
    //                   3   4

    // --- HIDDEN ADMIN ZONES (1-5) ---
    Component {
        id: adminTouchAreaComponent
        MouseArea {
            property int areaNumber: 0
            width: root.width / 3
            height: root.height / 3
            onClicked: selectArea(areaNumber)
            Rectangle {
                anchors.fill: parent
                color: "#FFFFFF"
                opacity: parent.pressed ? 0.08 : 0
            }
        }
    }

    Loader {
        sourceComponent: adminTouchAreaComponent
        x: 0
        y: 0
        onLoaded: item.areaNumber = 1
    }
    Loader {
        sourceComponent: adminTouchAreaComponent
        x: root.width - item.width
        y: 0
        onLoaded: item.areaNumber = 2
    }
    Loader {
        sourceComponent: adminTouchAreaComponent
        x: 0
        y: root.height - item.height
        onLoaded: item.areaNumber = 3
    }
    Loader {
        sourceComponent: adminTouchAreaComponent
        x: root.width - item.width
        y: root.height - item.height
        onLoaded: item.areaNumber = 4
    }
    Loader {
        sourceComponent: adminTouchAreaComponent
        x: (root.width - item.width) / 2
        y: (root.height - item.height) / 2
        onLoaded: item.areaNumber = 5
    }

    // --- LOGIC & DATA ---
    ListModel {
        id: statusModel
    }

    QtObject {
        id: global
        property var icons
        property string clickSequence: ""
        property var parameters: ({})
    }

    Component.onCompleted: {
        global.icons = {
            "blocked": "ic_blocked.svg",
            "config_failure": "ic_config.svg",
            "validator_failure": "ic_validator_error.svg",
            "printer_failure": "ic_printer_error.svg",
            "update_in_progress": "ic_update.svg",
            "network_failure": "ic_network_error.svg",
            "crypt_failure": "ic_crypt.svg",
            "cardreader_failure": "ic_card_error.svg",
            "token_failure": "ic_token_error.svg",
            "account_balance_failure": "ic_balance_error.svg"
        };
    }

    function selectArea(num) {
        screenActivityTimer.start();
        global.clickSequence += num.toString();
        Core.postEvent(EventType.UpdateScenario, {
            signal: "screen_password_updated",
            password: global.clickSequence
        });
        Core.log.normal("Clicked sequence: %1.".arg(global.clickSequence));
    }

    function notifyHandler(aEvent, aParameters) {
        statusModel.clear();
        global.parameters = aParameters;
        for (var key in aParameters) {
            if (aParameters[key] && global.icons[key]) {
                statusModel.append({
                    "image": global.icons[key]
                });
            }
        }
    }

    Timer {
        id: screenActivityTimer
        interval: 5000
        onTriggered: onScreenActivityTimeout()
    }
}
