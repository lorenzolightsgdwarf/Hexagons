import QtQuick 2.6
import QtQuick.Window 2.2
import QtMultimedia 5.6
import QtQuick.Controls 1.4
import ARToolkit 1.0
import ImageAnayser 1.0
Window {
    visible: true
    id : root
    width : 640
    height : 480


    MainForm {
       anchors.fill: parent

        MouseArea {
            id: selector
            anchors.fill: parent
            property string clicked_id
            onClicked: {
                var minDist = 1000
                var max = filter.detectedMarkers.length
                if(max > 0){
                    for(var i = 0; i < max; i++){//if we can assume a minimum distance then replace i++ with i+=4
                        var dx = mouseX - (filter.detectedMarkers[i]["TLCorner"].x)
                        var dy = mouseY - (filter.detectedMarkers[i]["TLCorner"].y)
                        var newMin = Math.sqrt((dx*dx)+(dy*dy))
                        if(newMin < minDist){
                            minDist = newMin
                            clicked_id = filter.detectedMarkers[i]["id"]
                        }
                    }
                    console.log("clicked is: ", clicked_id)
                }

            }

        }

        Camera {
            id: camera

            deviceId: QtMultimedia.availableCameras[1].deviceId
            viewfinder.resolution: "640x480"
            imageCapture {
                onImageCaptured: {
                    photoPreview.source = preview
                    analyser.run(filter.detectedMarkers, selector.clicked_id)
                }
            }
        }

        VideoOutput {
            source: camera
            anchors.fill: parent
            focus : visible // to receive focus and capture key events when visible
            filters: [filter]

        }
        ARToolkit{
            id:filter
            onDetectedMarkersChanged: {
                if(detectedMarkers.length>0){
                    canvas.requestPaint()
                }
            }
        }

        ImageAnayser{
            id: analyser
        }

        Canvas {
                id: canvas
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                onPaint: {
                    var ctx = canvas.getContext('2d')

                    ctx.reset();

                    ctx.fillStyle = "#ff0000";
                    var max = filter.detectedMarkers.length
                    for (var i = 0; i< max; ++i){
                        var centreX = filter.detectedMarkers[i]["TLCorner"].x-3;
                        var centreY = filter.detectedMarkers[i]["TLCorner"].y-3;
                        ctx.fillRect(centreX, centreY, 6, 6)
                    }
                }
        }

        Image {
            id: photoPreview
            anchors.fill: parent
        }

        Button{
            id : captureButton
            text : "Capture Frame"
            property string filename: "save.jpg"

            x : 320 - width/2
            y : 480 - height

            onClicked: {
                camera.imageCapture.captureToLocation(filename)
            }
        }
    }

}
