import QtQuick 2.6
import QtQuick.Window 2.2
import QtMultimedia 5.6
import QtQuick.Controls 1.4
import ARToolkit 1.0
import ImageAnayser 1.0
import QtQuick.Layouts 1.1
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
            property string current_selection_id
            onPressAndHold: {
                var minDist = 1000
                var max = filter.detectedMarkers.length
                if(max > 0){
                    for(var i = 0; i < max; i++){//if we can assume a minimum distance then replace i++ with i+=4
                        var dx = mouseX - ( video_output.contentRect.x+0.5*scaleX*(filter.detectedMarkers[i]["TLCorner"].x+filter.detectedMarkers[i]["BRCorner"].x))
                        var dy = mouseY - ( video_output.contentRect.y+0.5*scaleY*(filter.detectedMarkers[i]["TLCorner"].y+filter.detectedMarkers[i]["BRCorner"].y))
                        var newMin = Math.sqrt((dx*dx)+(dy*dy))
                        if(newMin < minDist){
                            minDist = newMin
                            clicked_id = filter.detectedMarkers[i]["id"]
                        }
                    }
                }
            }
            onClicked: {
                var minDist = 200
                var valid=false;
                var max = filter.detectedMarkers.length
                if(max > 0){
                    for(var i = 0; i < max; i++){//if we can assume a minimum distance then replace i++ with i+=4
                        var dx = mouseX - ( video_output.contentRect.x+0.5*scaleX*(filter.detectedMarkers[i]["TLCorner"].x+filter.detectedMarkers[i]["BRCorner"].x))
                        var dy = mouseY - ( video_output.contentRect.y+0.5*scaleY*(filter.detectedMarkers[i]["TLCorner"].y+filter.detectedMarkers[i]["BRCorner"].y))
                        var newMin = Math.sqrt((dx*dx)+(dy*dy))
                        if(newMin < minDist){
                            minDist = newMin
                            valid=true
                            current_selection_id = filter.detectedMarkers[i]["id"]
                        }
                    }
                }
                if(!valid)
                    current_selection_id=""
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
            projectionMatrix:Qt.matrix4x4(
                        1.4272479930179818e+03 ,0 ,6.4750000000000000e+02,0,
                        0, 1.4272479930179818e+03 , 4.8550000000000000e+02,0,
                        0,0,1,0,
                        0,0,0,1)
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
                    ctx.font="20pt sans-serif"
                    ctx.textBaseline="top"
                    var text_width=ctx.measureText("(0000.00,0000.00,000)").width;
                    for (var i = 0; i< max; ++i){
                        if(selector.current_selection_id == filter.detectedMarkers[i]["id"])
                            ctx.fillStyle = "#FFFBCC";
                        else
                            ctx.fillStyle = "#000000";
                        var centreX = video_output.contentRect.x+0.5*scaleX*(filter.detectedMarkers[i]["TLCorner"].x+filter.detectedMarkers[i]["BRCorner"].x);
                        var centreY = video_output.contentRect.y+0.5*scaleY*(filter.detectedMarkers[i]["TLCorner"].y+filter.detectedMarkers[i]["BRCorner"].y);

                        ctx.fillRect(centreX-5,
                                     centreY-5,
                                     text_width,100);

                        if(selector.clicked_id == filter.detectedMarkers[i]["id"])
                            ctx.fillStyle = "#ff0000";
                        else
                            ctx.fillStyle = "#00ff00";


                        var tag_id=filter.detectedMarkers[i]["id"];
                        var text=filter.detectedMarkers[i]["id"];
                        ctx.fillText(text,centreX,centreY);
                        if(analyser.poses[tag_id]){
                            text="("+Math.round(analyser.poses[tag_id].x*100)/100+","+Math.round(analyser.poses[tag_id].y*100)/100+","+Math.round(analyser.poses[tag_id].w*180/Math.PI)+")";
                            ctx.fillText(text,centreX,centreY+40);
                        }
                    }
                }
        }


        Button{
            id : captureButton
            text : "Capture Frame"
            property string filename: "save.jpg"
            anchors.bottom: parent.bottom
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 50
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

        Button{
            anchors.leftMargin: 75
            anchors.left: captureButton.right
            anchors.verticalCenter: captureButton.verticalCenter
            text:"Save to File"
            property bool saved: false
            onClicked: {
                saved=analyser.write_output_file();
            }
            Rectangle{
                width: 50
                height: 50
                radius: 25
                anchors.left: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: if(parent.saved)
                           return "green"
                       else
                           return "red"
            }
        }

        Column{
             visible:selector.current_selection_id.length>0
             anchors.margins: 10
             anchors.right: parent.right
             anchors.verticalCenter: parent.verticalCenter
             spacing: text_spin_box_x.height
             Rectangle{
                id:spinbox_x
                width: text_spin_box_x.width+text_spin_box_x.height*4
                height: text_spin_box_x.height*4
                property real value:0;
                Connections{
                    target: selector
                    onCurrent_selection_idChanged: if(selector.current_selection_id.length>0) spinbox_x.value=analyser.poses[selector.current_selection_id].x
                }
                anchors.margins: 10
                Text{
                    anchors.centerIn: parent
                    id:text_spin_box_x
                    text:"X: "+Math.round(spinbox_x.value*100)/100
                    Button{
                       text: "+"
                       onClicked: {
                           spinbox_x.value+=90*Math.cos(Math.PI/6)
                           analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                       }
                       anchors.margins: 5
                       anchors.left: parent.right
                       anchors.bottom: parent.verticalCenter
                       width: parent.height*2
                       height: width

                    }
                    Button{
                       text: "-"
                       anchors.margins: 5
                       onClicked: {
                           spinbox_x.value-=90*Math.cos(Math.PI/6)
                           analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                       }
                       anchors.left: parent.right
                       anchors.top: parent.verticalCenter
                       width: parent.height*2
                       height: width
                    }
                }
             }
             Rectangle{
                id:spinbox_y
                width: text_spin_box_y.width+text_spin_box_y.height*4
                height: text_spin_box_y.height*4
                property real value:0;
                Connections{
                    target: selector
                    onCurrent_selection_idChanged: if(selector.current_selection_id.length>0) spinbox_y.value=analyser.poses[selector.current_selection_id].y
                }
                anchors.margins: 10
                Text{
                    anchors.centerIn: parent
                    id:text_spin_box_y
                    text:"Y: "+Math.round(spinbox_y.value*100)/100
                    Button{
                       text: "+"
                       onClicked: {
                           spinbox_y.value+=90*Math.sin(Math.PI/6)
                           analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                       }
                       anchors.margins: 5
                       anchors.left: parent.right
                       anchors.bottom: parent.verticalCenter
                       width: parent.height*2
                       height: width

                    }
                    Button{
                       text: "-"
                       anchors.margins: 5
                       onClicked: {
                           spinbox_y.value-=90*Math.sin(Math.PI/6)
                           analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                       }

                       anchors.left: parent.right
                       anchors.top: parent.verticalCenter
                       width: parent.height*2
                       height: width
                    }
                }
             }
             Rectangle{
                id:spinbox_or
                width: text_spin_box_or.width+text_spin_box_or.height*4
                height: text_spin_box_or.height*4
                property real value:0;
                anchors.margins: 10
                Connections{
                    target: selector
                    onCurrent_selection_idChanged: if(selector.current_selection_id.length>0) spinbox_or.value=analyser.poses[selector.current_selection_id].w*180/Math.PI
                }
                Text{
                    anchors.centerIn: parent
                    id:text_spin_box_or
                    text:"Angle: "+Math.round(spinbox_or.value*100)/100
                    Button{
                       text: "+"
                       onClicked:if(spinbox_or.value < 180) {
                                     spinbox_or.value+=60
                                     analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                                 }
                       anchors.margins: 5
                       anchors.left: parent.right
                       anchors.bottom: parent.verticalCenter
                       width: parent.height*2
                       height: width

                    }
                    Button{
                       text: "-"
                       anchors.margins: 5
                       onClicked: if(spinbox_or.value > -180) {
                                      spinbox_or.value-=60
                                      analyser.change_pose(selector.current_selection_id,spinbox_x.value,spinbox_y.value,spinbox_or.value)
                                  }
                       anchors.left: parent.right
                       anchors.top: parent.verticalCenter
                       width: parent.height*2
                       height: width
                    }
                }
             }
        }


    }

}
