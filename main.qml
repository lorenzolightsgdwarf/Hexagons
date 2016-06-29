import QtQuick 2.6
import QtQuick.Window 2.2
import QtMultimedia 5.6
import QtQuick.Controls 1.4
import FilterFactory 1.0
import ImageAnayser 1.0
Window {
    visible: true
    id : root
    width : 640
    height : 360


    MainForm {
        width: 640
        height: 360

        MouseArea {
            id: selector
            anchors.fill: parent
            property int clicked_id
            onClicked: {
                var minDist = 1000
                var max = filter.corners.length
                if(max > 0){
                    for(var i = 0; i < max; i++){//if we can assume a minimum distance then replace i++ with i+=4

                        var dx = mouseX - (filter.corners[i].x * 640 / 1280)
                        var dy = mouseY - (filter.corners[i].y * 360 / 720)
                        var newMin = Math.sqrt((dx*dx)+(dy*dy))
                        if(newMin < minDist){
                            minDist = newMin
                            clicked_id = filter.corners[i].z
                        }
                    }
                    console.log("clicked is: ", clicked_id)
                }

            }

        }

        Camera {
            id: camera

            //deviceId: QtMultimedia.availableCameras[1].deviceId

            imageCapture {
                onImageCaptured: {
                    photoPreview.source = preview

                    analyser.run(captureButton.filename, selector.clicked_id)
                }
            }
        }

        VideoOutput {
            source: camera
            anchors.fill: parent
            focus : visible // to receive focus and capture key events when visible
            filters: [filter]

        }
        FilterFactory{
            id:filter
            onCornersChanged: {if(corners.length>0)canvas.requestPaint()}
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
                    var max = filter.corners.length
                    for (var i = 0; i< max; ++i){

                        var centreX = ((filter.corners[i].x*640) / 1280) - 3;
                        var centreY = ((filter.corners[i].y*360) / 720) - 3;
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
            property string filename: "/home/leo/build-Hexagons-Desktop_Qt_5_6_0_GCC_64bit-Debug/save.jpg"

            x : 320 - width/2
            y : 360 - height

            onClicked: {
                camera.imageCapture.captureToLocation(filename)
            }
        }
//        Button{
//            id: resumeButton
//            text : "Resume"

//            onClicked: //how do I resume?;
//        }
    }

}
