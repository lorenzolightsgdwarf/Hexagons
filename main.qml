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

    property real scaleX:video_output.contentRect.width/filter.cameraResolution.width
    property real scaleY:video_output.contentRect.height/filter.cameraResolution.height

    property int run_done:-1;

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
                        var dx = mouseX - ( video_output.contentRect.x+scaleX*filter.detectedMarkers[i]["TLCorner"].x)
                        var dy = mouseY - ( video_output.contentRect.y+scaleY*filter.detectedMarkers[i]["TLCorner"].y)
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
            deviceId: QtMultimedia.availableCameras[0].deviceId
            imageProcessing.whiteBalanceMode: CameraImageProcessing.WhiteBalanceAuto
            focus.focusMode: CameraFocus.FocusContinuous
            captureMode: Camera.CaptureViewfinder
            exposure.exposureMode: CameraExposure.ExposureModeVendor
            exposure.meteringMode: CameraExposure.MeteringMatrix
            imageProcessing.colorFilter: CameraImageProcessing.ColorFilterVendor
            imageProcessing.denoisingLevel: 1
            imageProcessing.sharpeningLevel: 1
            viewfinder.resolution: "640x480"
            imageCapture {
                onImageCaptured: {
                    run_done=analyser.run(filter.detectedMarkers, selector.clicked_id)
                }
            }
        }

        VideoOutput {
            id:video_output
            source: camera
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectCrop
            focus : visible // to receive focus and capture key events when visible
            filters: [filter]

        }
        ARToolkit{
            id:filter
            matrixCode: ARToolkit.MATRIX_CODE_4x4_BCH_13_9_3
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
                    var max = filter.detectedMarkers.length
                    for (var i = 0; i< max; ++i){
                        if(selector.clicked_id == filter.detectedMarkers[i]["id"])
                            ctx.fillStyle = "#ff0000";
                        else
                            ctx.fillStyle = "#00ff00";

                        var centreX = video_output.contentRect.x+scaleX*filter.detectedMarkers[i]["TLCorner"].x-3;
                        var centreY = video_output.contentRect.y+scaleY*filter.detectedMarkers[i]["TLCorner"].y-3;
                        ctx.fillRect(centreX, centreY, 10, 10)
                    }
                }
        }


        Button{
            id : captureButton
            text : "Capture Frame"
            property string filename: "save.jpg"
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                camera.imageCapture.captureToLocation(filename)
            }
            Rectangle{
                width: 50
                height: 50
                radius: 25
                anchors.left: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: if(run_done<0)
                           return "yellow"
                       else if(run_done==0)
                           return "red"
                       else return "green"
            }
        }
    }

}
