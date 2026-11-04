pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Transcribe

ColumnLayout {
    id: view
    required property Backend backend
    property bool following: true
    property bool applying: false
    property bool positioning: false
    property bool loaded: false
    property int revision: 0
    property bool displayingTail: false

    function positionAfterLayout(): void {
        positioning = true
        const generation = ++revision
        Qt.callLater(function() {
            if (generation !== view.revision) return
            viewport.contentY = view.following ? Math.max(0, viewport.contentHeight - viewport.height) : 0
            view.positioning = false
        })
    }
    function applyLatest(): void {
        const snapshot = backend.transcript
        if (loaded && editor.text === snapshot) {
            displayingTail = backend.transcriptNotice.length > 0
            return
        }
        const initial = !loaded
        applying = true
        positioning = true
        if (snapshot.startsWith(editor.text)) editor.insert(editor.length, snapshot.slice(editor.text.length))
        else editor.text = snapshot
        displayingTail = backend.transcriptNotice.length > 0
        loaded = snapshot.length > 0
        applying = false
        if (following || initial) positionAfterLayout()
        else positioning = false
    }
    function synchronize(): void {
        if (!loaded || (following && editor.selectedText.length === 0)) applyLatest()
    }
    function reset(): void {
        applying = true
        following = backend.busy && backend.resultDirectory === backend.taskResultDirectory
        loaded = false
        editor.deselect()
        editor.text = ""
        displayingTail = false
        applying = false
        applyLatest()
    }
    function resume(): void {
        applying = true
        editor.deselect()
        following = true
        applying = false
        applyLatest()
        positionAfterLayout()
    }
    function viewportMoved(): void {
        if (applying || positioning || editor.selectedText.length > 0) return
        const atEnd = viewport.contentY >= Math.max(0, viewport.contentHeight - viewport.height) - 2
        if (atEnd && !following) resume()
        else if (!atEnd) following = false
    }
    Connections {
        target: view.backend
        function onTranscriptChanged(): void { view.synchronize() }
        function onTranscriptReset(): void { view.reset() }
    }
    Component.onCompleted: reset()
    Label {
        text: qsTr("Показан конец текста; полный TXT доступен отдельной кнопкой.")
        visible: view.displayingTail
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
        Layout.fillWidth: true
    }
    ScrollView {
        id: scroll
        objectName: "transcriptScroll"
        Layout.fillWidth: true
        Layout.preferredHeight: 260
        Flickable {
            id: viewport
            width: scroll.availableWidth
            height: scroll.availableHeight
            contentWidth: width
            contentHeight: editor.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            onContentYChanged: view.viewportMoved()
            onContentHeightChanged: if (view.following && !view.applying) view.positionAfterLayout()
            onHeightChanged: if (view.following && !view.applying) view.positionAfterLayout()
            UiTextArea {
                id: editor
                objectName: "transcriptText"
                width: viewport.width
                height: implicitHeight
                readOnly: true
                selectByMouse: true
                persistentSelection: true
                wrapMode: TextEdit.Wrap
                textFormat: TextEdit.PlainText
                placeholderText: qsTr("Текст появится во время распознавания. Полный результат сохраняется в TXT.")
                Accessible.name: qsTr("Частичный текст расшифровки")
                onSelectedTextChanged: {
                    if (view.applying) return
                    if (selectedText.length > 0) view.following = false
                    else view.viewportMoved()
                }
            }
        }
    }
    UiButton {
        objectName: "followTranscript"
        text: qsTr("К новому тексту")
        visible: !view.following && editor.text !== view.backend.transcript
        onClicked: view.resume()
    }
}
