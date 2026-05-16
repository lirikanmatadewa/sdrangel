import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import QtLocation 5.14
import QtPositioning 5.14

Item {
    id: qmlMap
    property int mapZoomLevel: 11
    property string mapProvider: "osm"
    property variant mapPtr
    property variant guiPtr
    property bool smoothing

    function createMap(pluginParameters, gui) {
        guiPtr = gui
        var paramString = ""
        for (var prop in pluginParameters) {
            var parameter = 'PluginParameter { name: "' + prop + '"; value: "' + pluginParameters[prop] + '"}'
            paramString = paramString + parameter
        }
        var pluginString = 'import QtLocation 5.14; Plugin{ name:"' + mapProvider + '"; '  + paramString + '}'
        var plugin = Qt.createQmlObject(pluginString, qmlMap)

        if (mapPtr) {
            mapPtr.destroy()
            mapPtr = null
        }
        mapPtr = actualMapComponent.createObject(page)
        mapPtr.plugin = plugin
        mapPtr.forceActiveFocus()
        return mapPtr
    }

    function getMapTypes() {
        var mapTypes = []
        if (mapPtr) {
            for (var i = 0; i < mapPtr.supportedMapTypes.length; i++) {
                mapTypes[i] = mapPtr.supportedMapTypes[i].name
            }
        }
        return mapTypes
    }

    function setMapType(mapTypeIndex) {
        if (mapPtr && (mapTypeIndex < mapPtr.supportedMapTypes.length)) {
            if (mapPtr.supportedMapTypes[mapTypeIndex] !== undefined) {
                mapPtr.activeMapType = mapPtr.supportedMapTypes[mapTypeIndex]
            }
        }
    }

    Item {
        id: page
        anchors.fill: parent
    }

    Item{
        id: doaOverlay
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 8
        width: 240
        height: 240
        z: 9999
        visible: (typeof mapGui !== "undefined" && mapGui !== null)

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: "#66000000"
            border.width: 1
            border.color: "#66ffffff"
        }

        Text {
            id: doaTitle
            property string currentAngleText: "0"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 10
            anchors.topMargin: 8
            text: "MAX DOA Angle: " + currentAngleText
            color: "white"
            font.pixelSize: 14
            font.bold: true
            z: 2

            function updateAngleText() {
                var angleValue = 0
                if (typeof mapGui !== "undefined" && mapGui !== null) {
                    angleValue = Number(mapGui.doaAngle)
                }
                if (isNaN(angleValue)) {
                    angleValue = 0
                }
                currentAngleText = Math.round(angleValue).toString()
            }

            Component.onCompleted: {
                updateAngleText()
            }
        }

                Canvas {
            id: doaCanvas
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: doaTitle.bottom
            anchors.bottom: parent.bottom
            anchors.margins: 8

            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()

                var w = width
                var h = height
                var cx = w / 2
                var cy = h / 2 + 8
                var r = Math.min(w, h) * 0.42

                // ===== background =====
                var bgGrad = ctx.createRadialGradient(cx, cy, r * 0.08, cx, cy, r)
                bgGrad.addColorStop(0.0, "#0d1522")
                bgGrad.addColorStop(0.55, "#09111b")
                bgGrad.addColorStop(1.0, "#04070c")

                ctx.beginPath()
                ctx.arc(cx, cy, r, 0, 2 * Math.PI)
                ctx.fillStyle = bgGrad
                ctx.fill()

                // outer glow ring
                ctx.beginPath()
                ctx.arc(cx, cy, r + 2, 0, 2 * Math.PI)
                ctx.strokeStyle = "rgba(110,170,255,0.18)"
                ctx.lineWidth = 6
                ctx.stroke()

                // outer ring
                ctx.beginPath()
                ctx.arc(cx, cy, r, 0, 2 * Math.PI)
                ctx.strokeStyle = "#7aa7d9"
                ctx.lineWidth = 1.5
                ctx.stroke()

                // ===== concentric rings =====
                for (var i = 1; i <= 5; i++) {
                    var rr = r * i / 5
                    ctx.beginPath()
                    ctx.arc(cx, cy, rr, 0, 2 * Math.PI)
                    ctx.strokeStyle = (i === 5) ? "rgba(100,160,220,0.28)" : "rgba(90,130,170,0.16)"
                    ctx.lineWidth = 1
                    ctx.stroke()
                }

                // ===== radial lines =====
                for (var deg = 0; deg < 360; deg += 45) {
                    var rr1 = (deg - 90) * Math.PI / 180.0
                    var x = cx + Math.cos(rr1) * r
                    var y = cy + Math.sin(rr1) * r

                    ctx.beginPath()
                    ctx.moveTo(cx, cy)
                    ctx.lineTo(x, y)
                    ctx.strokeStyle = (deg % 90 === 0) ? "rgba(130,180,230,0.28)" : "rgba(130,180,230,0.14)"
                    ctx.lineWidth = (deg % 90 === 0) ? 1.2 : 1
                    ctx.stroke()
                }

                // ===== degree labels =====
                ctx.fillStyle = "rgba(255,255,255,0.92)"
                ctx.font = "bold 12px sans-serif"
                var labels = [0, 45, 90, 135, 180, 225, 270, 315]
                for (var j = 0; j < labels.length; j++) {
                    var d = labels[j]
                    var rad = (d - 90) * Math.PI / 180.0
                    var lx = cx + Math.cos(rad) * (r + 18)
                    var ly = cy + Math.sin(rad) * (r + 18)
                    ctx.fillText(d + "\u00B0", lx - 14, ly + 4)
                }

                // ===== safe DOA read =====
                var doa = 0
                if (typeof mapGui !== "undefined" && mapGui !== null) {
                    doa = Number(mapGui.doaAngle)
                }
                if (isNaN(doa)) {
                    doa = 0
                }

                // ===== orientation config =====
                var spreadDeg = 42
                var angleOffset = -90

                var startDeg = doa - spreadDeg / 2
                var endDeg = doa + spreadDeg / 2

                var startRad = (startDeg + angleOffset) * Math.PI / 180.0
                var endRad = (endDeg + angleOffset) * Math.PI / 180.0
                var doaRad = (doa + angleOffset) * Math.PI / 180.0

                // ===== soft wedge glow =====
                ctx.beginPath()
                ctx.moveTo(cx, cy)
                ctx.arc(cx, cy, r * 0.96, startRad, endRad, false)
                ctx.closePath()
                ctx.fillStyle = "rgba(88, 130, 255, 0.18)"
                ctx.fill()

                // inner wedge
                ctx.beginPath()
                ctx.moveTo(cx, cy)
                ctx.arc(cx, cy, r * 0.88, startRad, endRad, false)
                ctx.closePath()
                var wedgeGrad = ctx.createRadialGradient(cx, cy, r * 0.06, cx, cy, r * 0.88)
                wedgeGrad.addColorStop(0.0, "rgba(130,180,255,0.35)")
                wedgeGrad.addColorStop(0.65, "rgba(90,120,255,0.22)")
                wedgeGrad.addColorStop(1.0, "rgba(70,95,220,0.10)")
                ctx.fillStyle = wedgeGrad
                ctx.fill()

                // wedge edge lines
                ctx.beginPath()
                ctx.moveTo(cx, cy)
                ctx.lineTo(cx + Math.cos(startRad) * r * 0.9, cy + Math.sin(startRad) * r * 0.9)
                ctx.moveTo(cx, cy)
                ctx.lineTo(cx + Math.cos(endRad) * r * 0.9, cy + Math.sin(endRad) * r * 0.9)
                ctx.strokeStyle = "rgba(150,190,255,0.26)"
                ctx.lineWidth = 1.2
                ctx.stroke()

                // ===== needle glow =====
                var tipX = cx + Math.cos(doaRad) * r * 0.86
                var tipY = cy + Math.sin(doaRad) * r * 0.86
                var tailX = cx - Math.cos(doaRad) * r * 0.18
                var tailY = cy - Math.sin(doaRad) * r * 0.18

                ctx.beginPath()
                ctx.moveTo(tailX, tailY)
                ctx.lineTo(tipX, tipY)
                ctx.strokeStyle = "rgba(90,170,255,0.22)"
                ctx.lineWidth = 10
                ctx.lineCap = "round"
                ctx.stroke()

                // ===== modern needle body =====
                var nx = Math.cos(doaRad)
                var ny = Math.sin(doaRad)
                var px = -ny
                var py = nx

                var baseHalf = 5.5
                var midHalf = 3.0
                var tailHalf = 3.8

                var frontBaseX = cx + nx * 16
                var frontBaseY = cy + ny * 16

                var midX = cx + nx * (r * 0.48)
                var midY = cy + ny * (r * 0.48)

                ctx.beginPath()
                ctx.moveTo(cx + px * baseHalf, cy + py * baseHalf)
                ctx.lineTo(frontBaseX + px * midHalf, frontBaseY + py * midHalf)
                ctx.lineTo(tipX, tipY)
                ctx.lineTo(frontBaseX - px * midHalf, frontBaseY - py * midHalf)
                ctx.lineTo(cx - px * baseHalf, cy - py * baseHalf)
                ctx.closePath()

                var needleGrad = ctx.createLinearGradient(cx, cy, tipX, tipY)
                needleGrad.addColorStop(0.0, "#8fe3ff")
                needleGrad.addColorStop(0.45, "#54b8ff")
                needleGrad.addColorStop(1.0, "#2d7cff")
                ctx.fillStyle = needleGrad
                ctx.fill()

                ctx.strokeStyle = "rgba(210,240,255,0.70)"
                ctx.lineWidth = 1
                ctx.stroke()

                // ===== tail =====
                ctx.beginPath()
                ctx.moveTo(cx + px * tailHalf, cy + py * tailHalf)
                ctx.lineTo(tailX, tailY)
                ctx.lineTo(cx - px * tailHalf, cy - py * tailHalf)
                ctx.closePath()
                ctx.fillStyle = "rgba(70,120,180,0.55)"
                ctx.fill()

                // ===== center hub glow =====
                ctx.beginPath()
                ctx.arc(cx, cy, 13, 0, 2 * Math.PI)
                ctx.fillStyle = "rgba(90,170,255,0.16)"
                ctx.fill()

                // center hub outer
                ctx.beginPath()
                ctx.arc(cx, cy, 8.5, 0, 2 * Math.PI)
                var hubGrad = ctx.createRadialGradient(cx, cy, 1, cx, cy, 8.5)
                hubGrad.addColorStop(0.0, "#d8f6ff")
                hubGrad.addColorStop(0.35, "#7ed5ff")
                hubGrad.addColorStop(1.0, "#2d79d9")
                ctx.fillStyle = hubGrad
                ctx.fill()

                // center hub inner
                ctx.beginPath()
                ctx.arc(cx, cy, 3.2, 0, 2 * Math.PI)
                ctx.fillStyle = "#f6fdff"
                ctx.fill()

                // ===== top highlight arc =====
                ctx.beginPath()
                ctx.arc(cx, cy, r * 0.72, -115 * Math.PI / 180, -65 * Math.PI / 180, false)
                ctx.strokeStyle = "rgba(255,255,255,0.10)"
                ctx.lineWidth = 2
                ctx.stroke()
            }

            Component.onCompleted: {
                doaTitle.updateAngleText()
                requestPaint()
            }

            Connections {
                target: (typeof mapGui !== "undefined") ? mapGui : null
                function onDoaAngleChanged() {
                    doaTitle.updateAngleText()
                    doaCanvas.requestPaint()
                }
            }
        }
    }

    Component {
        id: actualMapComponent

        Map {
            id: map
            objectName: "map"
            anchors.fill: parent
            center: QtPositioning.coordinate(51.5, 0.125)
            zoomLevel: 10
            gesture.enabled: true
            gesture.acceptedGestures: MapGestureArea.PinchGesture | MapGestureArea.PanGesture

            MapItemView {
                model: imageModelFiltered
                delegate: imageComponent
            }

            MapItemView {
                model: polygonModelFiltered
                delegate: polygonComponent
            }

            MapItemView {
                model: polygonModelFiltered
                delegate: polygonNameComponent
            }

            MapItemView {
                model: polylineModelFiltered
                delegate: polylineComponent
            }

            MapItemView {
                model: polylineModelFiltered
                delegate: polylineNameComponent
            }

            MapItemView {
                model: mapModelFiltered
                delegate: groundTrack1Component
            }

            MapItemView {
                model: mapModelFiltered
                delegate: groundTrack2Component
            }

            MapItemView {
                model: mapModelFiltered
                delegate: predictedGroundTrack1Component
            }

            MapItemView {
                model: mapModelFiltered
                delegate: predictedGroundTrack2Component
            }

            MapItemView {
                model: mapModelFiltered
                delegate: mapComponent
            }

            onZoomLevelChanged: {
                mapZoomLevel = zoomLevel
                mapModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                imageModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                polygonModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                polylineModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
            }

            onCenterChanged: {
                polylineModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                polygonModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                imageModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                mapModelFiltered.viewChanged(visibleRegion.boundingGeoRectangle().topLeft.longitude, visibleRegion.boundingGeoRectangle().topLeft.latitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude, visibleRegion.boundingGeoRectangle().bottomRight.latitude, zoomLevel)
                mapModel.viewChanged(visibleRegion.boundingGeoRectangle().bottomLeft.longitude, visibleRegion.boundingGeoRectangle().bottomRight.longitude)
            }

            onSupportedMapTypesChanged: {
                guiPtr.supportedMapsChanged()
            }
        }
    }

    function mapRect() {
        if (mapPtr)
            return mapPtr.visibleRegion.boundingGeoRectangle()
        else
            return null
    }

    Component {
        id: imageComponent
        MapQuickItem {
            coordinate: position
            anchorPoint.x: imageId.width / 2
            anchorPoint.y: imageId.height / 2
            zoomLevel: imageZoomLevel
            sourceItem: Image {
                id: imageId
                source: imageData
            }
            autoFadeIn: false
        }
    }

    Component {
        id: polygonComponent
        MapPolygon {
            border.width: 1
            border.color: borderColor
            color: fillColor
            path: polygon
            autoFadeIn: false
        }
    }

    Component {
        id: polygonNameComponent
        MapQuickItem {
            coordinate: position
            anchorPoint.x: polygonText.width / 2
            anchorPoint.y: polygonText.height / 2
            zoomLevel: mapZoomLevel > 11 ? mapZoomLevel : 11
            sourceItem: Grid {
                columns: 1
                Grid {
                    layer.enabled: smoothing
                    layer.smooth: smoothing
                    horizontalItemAlignment: Grid.AlignHCenter
                    Text {
                        id: polygonText
                        text: label
                        textFormat: TextEdit.RichText
                    }
                }
            }
        }
    }

    Component {
        id: polylineComponent
        MapPolyline {
            line.width: 1
            line.color: lineColor
            path: coordinates
            autoFadeIn: false
        }
    }

    Component {
        id: polylineNameComponent
        MapQuickItem {
            coordinate: position
            anchorPoint.x: polylineText.width / 2
            anchorPoint.y: polylineText.height / 2
            zoomLevel: mapZoomLevel > 11 ? mapZoomLevel : 11
            sourceItem: Grid {
                columns: 1
                Grid {
                    layer.enabled: smoothing
                    layer.smooth: smoothing
                    horizontalItemAlignment: Grid.AlignHCenter
                    Text {
                        id: polylineText
                        text: label
                        textFormat: TextEdit.RichText
                    }
                }
            }
        }
    }

    Component {
        id: mapComponent
        MapQuickItem {
            id: mapElement
            anchorPoint.x: image.width / 2
            anchorPoint.y: image.height / 2
            coordinate: position
            zoomLevel: (typeof mapImageMinZoom !== "undefined") ? (mapZoomLevel > mapImageMinZoom ? mapZoomLevel : mapImageMinZoom) : zoomLevel
            autoFadeIn: false

            sourceItem: Grid {
                id: gridItem
                columns: 1
                Grid {
                    horizontalItemAlignment: Grid.AlignHCenter
                    columnSpacing: 5
                    layer.enabled: smoothing
                    layer.smooth: smoothing
                    Image {
                        id: image
                        rotation: mapImageRotation
                        source: mapImage
                        visible: mapImageVisible
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: {
                                if (mouse.button === Qt.LeftButton) {
                                    selected = !selected
                                    if (selected) {
                                        mapModel.moveToFront(mapModelFiltered.mapRowToSource(index))
                                    }
                                } else if (mouse.button === Qt.RightButton) {
                                    menuItems.clear()
                                    menus.clear()
                                    if (frequencies.length > 0) {
                                        var deviceSets = mapModel.getDeviceSets()
                                        for (var i = 0; i < deviceSets.length; i++) {
                                            menus.append({
                                                title: "Set " + deviceSets[i] + " to...",
                                                deviceSet: i
                                            })
                                            for (var j = 0; j < frequencies.length; j++) {
                                                menuItems.append({
                                                    text: frequencyStrings[j],
                                                    frequency: frequencies[j],
                                                    deviceSet: deviceSets[i],
                                                    menuIndex: i
                                                })
                                            }
                                        }
                                    }
                                    var c = mapPtr.toCoordinate(Qt.point(mouse.x, mouse.y))
                                    coordsMenuItem.text = "Coords: " + c.latitude.toFixed(6) + ", " + c.longitude.toFixed(6)
                                    contextMenu.popup()
                                }
                            }
                        }
                    }
                    Rectangle {
                        id: bubble
                        color: bubbleColour
                        border.width: 1
                        width: text.width + 5
                        height: text.height + 5
                        radius: 5
                        visible: mapTextVisible
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton | Qt.RightButton
                            onClicked: {
                                if (mouse.button === Qt.LeftButton) {
                                    selected = !selected
                                    if (selected) {
                                        mapModel.moveToFront(mapModelFiltered.mapRowToSource(index))
                                    }
                                } else if (mouse.button === Qt.RightButton) {
                                    menuItems.clear()
                                    menus.clear()
                                    if (frequencies.length > 0) {
                                        var deviceSets = mapModel.getDeviceSets()
                                        for (var i = 0; i < deviceSets.length; i++) {
                                            menus.append({
                                                title: "Set " + deviceSets[i] + " to...",
                                                deviceSet: i
                                            })
                                            for (var j = 0; j < frequencies.length; j++) {
                                                menuItems.append({
                                                    text: frequencyStrings[j],
                                                    frequency: frequencies[j],
                                                    deviceSet: deviceSets[i],
                                                    menuIndex: i
                                                })
                                            }
                                        }
                                    }
                                    var c = mapPtr.toCoordinate(Qt.point(mouse.x, mouse.y))
                                    coordsMenuItem.text = "Coords: " + c.latitude.toFixed(6) + ", " + c.longitude.toFixed(6)
                                    contextMenu.popup()
                                }
                            }
                            ListModel {
                                id: menus
                            }
                            ListModel {
                                id: menuItems
                            }
                            Menu {
                                id: contextMenu
                                MenuItem {
                                    text: "Set as target"
                                    onTriggered: target = true
                                }
                                MenuItem {
                                    text: "Move to front"
                                    onTriggered: mapModel.moveToFront(mapModelFiltered.mapRowToSource(index))
                                }
                                MenuItem {
                                    text: "Move to back"
                                    onTriggered: mapModel.moveToBack(mapModelFiltered.mapRowToSource(index))
                                }
                                MenuItem {
                                    text: "Track on 3D map"
                                    onTriggered: mapModel.track3D(mapModelFiltered.mapRowToSource(index))
                                }
                                MenuItem {
                                    id: coordsMenuItem
                                    text: ""
                                }
                                Instantiator {
                                    model: menus
                                    delegate: Menu {
                                        id: contextSubMenu
                                        title: model.title
                                    }
                                    onObjectAdded: function(index, object) {
                                        contextMenu.insertMenu(index, object)
                                    }
                                    onObjectRemoved: function(index, object) {
                                        contextMenu.removeMenu(object)
                                    }
                                }
                                Instantiator {
                                    model: menuItems
                                    delegate: MenuItem {
                                        text: model.text
                                        onTriggered: mapModel.setFrequency(model.frequency, model.deviceSet)
                                    }
                                    onObjectAdded: function(index, object) {
                                        var menuItem = menuItems.get(index)
                                        contextMenu.menuAt(menuItem.menuIndex).insertItem(index, object)
                                    }
                                    onObjectRemoved: function(index, object) {
                                        object.menu.removeItem(object)
                                    }
                                }
                            }
                        }
                        Text {
                            id: text
                            anchors.centerIn: parent
                            text: mapText
                            textFormat: TextEdit.RichText
                            onLinkActivated: {
                                console.log("Link", link)
                                mapModel.link(link)
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: predictedGroundTrack1Component
        MapPolyline {
            line.width: 2
            line.color: predictedGroundTrackColor
            path: predictedGroundTrack1
            autoFadeIn: false
        }
    }

    Component {
        id: predictedGroundTrack2Component
        MapPolyline {
            line.width: 2
            line.color: predictedGroundTrackColor
            path: predictedGroundTrack2
            autoFadeIn: false
        }
    }

    Component {
        id: groundTrack1Component
        MapPolyline {
            line.width: 2
            line.color: groundTrackColor
            path: groundTrack1
            autoFadeIn: false
        }
    }

    Component {
        id: groundTrack2Component
        MapPolyline {
            line.width: 2
            line.color: groundTrackColor
            path: groundTrack2
            autoFadeIn: false
        }
    }
}