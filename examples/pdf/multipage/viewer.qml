/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Pdf 5.15
import QtQuick.Shapes 1.14
import QtQuick.Window 2.14
import Qt.labs.platform 1.1 as Platform

ApplicationWindow {
    id: root
    width: 800
    height: 1024
    color: "lightgrey"
    title: document.title
    visible: true
    property alias source: document.source // for main.cpp

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 6
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Open
                    icon.source: "resources/document-open.svg"
                    onTriggered: fileDialog.open()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomIn
                    enabled: view.renderScale < 10
                    icon.source: "resources/zoom-in.svg"
                    onTriggered: view.renderScale *= Math.sqrt(2)
                }
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.ZoomOut
                    enabled: view.renderScale > 0.1
                    icon.source: "resources/zoom-out.svg"
                    onTriggered: view.renderScale /= Math.sqrt(2)
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "resources/zoom-fit-width.svg"
                    onTriggered: view.scaleToWidth(root.contentItem.width, root.contentItem.height)
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "resources/zoom-fit-best.svg"
                    onTriggered: view.scaleToPage(root.contentItem.width, root.contentItem.height)
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+0"
                    icon.source: "resources/zoom-original.svg"
                    onTriggered: view.resetScale()
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+L"
                    icon.source: "resources/rotate-left.svg"
                    onTriggered: view.pageRotation -= 90
                }
            }
            ToolButton {
                action: Action {
                    shortcut: "Ctrl+R"
                    icon.source: "resources/rotate-right.svg"
                    onTriggered: view.pageRotation += 90
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "resources/go-previous-view-page.svg"
                    enabled: view.backEnbled
                    onTriggered: view.back()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "go back"
            }
            SpinBox {
                id: currentPageSB
                from: 1
                to: document.pageCount
                editable: true
                onValueModified: view.goToPage(value - 1)
                Shortcut {
                    sequence: StandardKey.MoveToPreviousPage
                    onActivated: currentPageSB.value--
                }
                Shortcut {
                    sequence: StandardKey.MoveToNextPage
                    onActivated: currentPageSB.value++
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "resources/go-next-view-page.svg"
                    enabled: view.forwardEnabled
                    onTriggered: view.forward()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "go forward"
            }
            ToolButton {
                action: Action {
                    shortcut: StandardKey.Copy
                    icon.source: "resources/edit-copy.svg"
                    enabled: view.selectedText !== ""
                    onTriggered: view.copySelectionToClipboard()
                }
            }
            Shortcut {
                sequence: StandardKey.Find
                onActivated: searchField.forceActiveFocus()
            }
            Shortcut {
                sequence: StandardKey.Quit
                onActivated: Qt.quit()
            }
        }
    }

    Platform.FileDialog {
        id: fileDialog
        title: "Open a PDF file"
        nameFilters: [ "PDF files (*.pdf)" ]
        onAccepted: document.source = file
    }

    Dialog {
        id: passwordDialog
        title: "Password"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        closePolicy: Popup.CloseOnEscape
        anchors.centerIn: parent
        width: 300

        TextField {
            id: passwordField
            placeholderText: qsTr("Please provide the password")
            echoMode: TextInput.Password
            width: parent.width
            onAccepted: passwordDialog.accept()
        }
        onAccepted: document.password = passwordField.text
    }

    Dialog {
        id: errorDialog
        title: "Error loading " + document.source
        standardButtons: Dialog.Ok
        modal: true
        closePolicy: Popup.CloseOnEscape
        anchors.centerIn: parent
        width: 300

        Label {
            id: errorField
            text: document.error
        }
    }

    PdfDocument {
        id: document
        onStatusChanged: {
            if (status === PdfDocument.Error) errorDialog.open()
            view.document = (status === PdfDocument.Ready ? document : undefined)
        }
        onPasswordRequired: {
            passwordDialog.open()
            passwordField.forceActiveFocus()
        }
    }

    PdfMultiPageView {
        id: view
        anchors.fill: parent
        anchors.leftMargin: searchDrawer.position * searchDrawer.width
        document: root.document
        searchString: searchField.text
        onCurrentPageChanged: currentPageSB.value = view.currentPage + 1
    }

    Drawer {
        id: searchDrawer
        edge: Qt.LeftEdge
        modal: false
        width: 300
        y: root.header.height
        height: view.height
        dim: false
        ListView {
            id: searchResultsList
            anchors.fill: parent
            anchors.margins: 2
            model: view.searchModel
            ScrollBar.vertical: ScrollBar { }
            delegate: ItemDelegate {
                width: parent ? parent.width : 0
                text: "page " + (page + 1) + ": " + context
                highlighted: ListView.isCurrentItem
                onClicked: {
                    searchResultsList.currentIndex = index
                    view.goToLocation(page, location, 0)
                    view.searchModel.currentResult = indexOnPage
                }
            }
        }
    }

    footer: ToolBar {
        height: footerRow.implicitHeight
        RowLayout {
            id: footerRow
            anchors.fill: parent
            ToolButton {
                action: Action {
                    icon.source: "resources/go-up-search.svg"
                    shortcut: StandardKey.FindPrevious
                    onTriggered: view.searchBack()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "find previous"
            }
            TextField {
                id: searchField
                placeholderText: "search"
                Layout.minimumWidth: 200
                Layout.fillWidth: true
                onAccepted: searchDrawer.open()
                Image {
                    visible: searchField.text !== ""
                    source: "resources/edit-clear.svg"
                    anchors {
                        right: parent.right
                        top: parent.top
                        bottom: parent.bottom
                        margins: 3
                        rightMargin: 5
                    }
                    TapHandler {
                        onTapped: searchField.clear()
                    }
                }
            }
            ToolButton {
                action: Action {
                    icon.source: "resources/go-down-search.svg"
                    shortcut: StandardKey.FindNext
                    onTriggered: view.searchForward()
                }
                ToolTip.visible: enabled && hovered
                ToolTip.delay: 2000
                ToolTip.text: "find next"
            }
            Label {
                id: statusLabel
                Layout.fillWidth: true
                property size implicitPointSize: document.pagePointSize(view.currentPage)
                text: "page " + (currentPageSB.value) + " of " + document.pageCount +
                      " scale " + view.renderScale.toFixed(2) +
                      " original size " + implicitPointSize.width.toFixed(1) + "x" + implicitPointSize.height.toFixed(1) + " pt"
                visible: document.pageCount > 0
            }
        }
    }
}
