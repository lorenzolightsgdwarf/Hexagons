#include "imageanayser.h"
#include <QVector2D>
#include <QImage>
#include <stdio.h>


ImageAnayser::ImageAnayser(QObject *parent):
    QObject(parent)
{

}

void ImageAnayser::run(QVariantList markers, QString origin)
{

    if(0!=reconstruct_board(markers, MARKER_LEN, MARKER_DIST, MARKER_ELEV, origin)){
        qDebug() << "something went wrong in the image conversion";
    }

}



int ImageAnayser::reconstruct_board(QVariantList markers, float marker_size, float marker_distance, float marker_elevation, QString origin){

    QList<QVariantMap> _markers;
    Q_FOREACH(QVariant v,markers){
        _markers.append(v.value<QVariantMap>());
    }
    if(_markers.size()<2)
        return -1;

    QVector3D matC(0, 0, 0);
    QVector3D matTR(marker_size/2, -marker_size/2, 0);
    QVector3D matTL(-marker_size/2, -marker_size/2, 0);
    QVector3D matBR(marker_size/2, marker_size/2, 0);
    QVector3D matBL(-marker_size/2, marker_size/2, 0);

    QMatrix3x3 rotMat;
    QVector3D translation;
    QQuaternion rot;

    int indexOfOrigin = getBottomLeftMarker(_markers, origin);
    if(indexOfOrigin<0){
        qDebug()<<"Can't find origin id:"<<origin;
        return-2;
    }



    //calculate the transformation matrix of the origin tag
    rot= _markers.at(indexOfOrigin)["rotation"].value<QQuaternion>();
    translation= _markers.at(indexOfOrigin)["translation"].value<QVector3D>();
    rotMat=rot.toRotationMatrix();
    QMatrix4x4 originM;
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            originM(i,j)=rotMat(i,j);
    originM(0,3)=translation.x();originM(1,3)=translation.y();originM(2,3)=translation.z();
    originM(3,0)=0;originM(3,1)=0;originM(3,2)=0;originM(3,3)=1;

    QMatrix4x4 originInv = originM.inverted();

    for(int i = 0; i < _markers.size(); i++){

        QVariantMap marker=_markers.at(i);

        QString id=marker["id"].toString();

        //calculate the transformation matrix of the tag i
        rot= marker["rotation"].value<QQuaternion>();
        translation= marker["translation"].value<QVector3D>();
        rotMat=rot.toRotationMatrix();
        QMatrix4x4 matrixOf_i;
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                matrixOf_i(i,j)=rotMat(i,j);
        matrixOf_i(0,3)=translation.x();matrixOf_i(1,3)=translation.y();matrixOf_i(2,3)=translation.z();
        matrixOf_i(3,0)=0;matrixOf_i(3,1)=0;matrixOf_i(3,2)=0;matrixOf_i(3,3)=1;

        //calculate the transformation matrix to get from the origin tag to the tag i
        QMatrix4x4 multipM = originInv * matrixOf_i;

        //coordinates of the corners and the center of the tag i
        QVector3D cVec = multipM * matC ;
        QVector3D trVec = multipM * matTR ;
        QVector3D tlVec = multipM * matTL ;
        QVector3D blVec = multipM * matBL ;
        QVector3D brVec = multipM * matBR ;

        QVector<QVector3D> squad;
        squad.append(cVec);
        squad.append(trVec);
        squad.append(tlVec);
        squad.append(blVec);
        squad.append(brVec);

        QVector<QVector3D> new_squad;
        double orientation;
        new_squad = approximations(squad, marker_distance, marker_size, marker_elevation, orientation);

        qDebug() << "Marker:"<<id;
        qDebug() <<"Center" << new_squad.at(0);
        qDebug() << "Top Rigth" <<new_squad.at(1);
        qDebug() << "Top Left" << new_squad.at(2);
        qDebug() << "Bottom Left" << new_squad.at(3);
        qDebug() << "Bottom Right" << new_squad.at(4);
        qDebug() << "Distance to Origin" << new_squad.at(0).length();
        qDebug() << "Orientation" << orientation*180/PI;
    }


    return 0;

}




int ImageAnayser::getBottomLeftMarker(QList<QVariantMap> markers, QString origin){
    for(int i=0;i<markers.size();i++){
        QVariantMap v=markers.at(i);
        if(v["id"].toString()==origin)
            return i;
    }
    return -1;
}


QVector<QVector3D> ImageAnayser::approximations(QVector<QVector3D> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation){

    double y_chunk = ((double) marker_distance) * cos(PI/3.0);

    double y_mod = fmod(((double) exact[0].y()), y_chunk);
    if(y_mod < 0.0){
        y_mod += y_chunk;
    }


    double to_add = 0;
    if(y_mod > y_chunk/2.0) to_add = y_chunk;

    double y_new = ( exact[0][1]) - (y_mod) + (to_add);

    double x_chunk = ((double)marker_distance) * cos(PI/6.0);
    double x_dist = exact[0].x();
    double x_mod = fmod(( x_dist), (x_chunk));
    if(x_mod < 0){
        x_mod += x_chunk;
    }

    if(x_mod > ( x_chunk/2.0)){
        to_add = x_chunk;
    } else {
        to_add = 0.0;
    }

    double x_new = x_dist - ( x_mod) + (to_add);


    QVector3D new_center(x_new, y_new, 0.0);

    if(exact.size() == 5){
        double alpha = orientationOfMarker(exact[0], exact[1], exact[2], marker_size);
        double halfLen = ((double)marker_size)/2.0;

        double new_front_x = x_new - halfLen*sin(alpha);
        double new_front_y = y_new - halfLen*cos(alpha);

        QVector3D new_topRight (new_front_x + halfLen*cos(alpha), new_front_y - halfLen*sin(alpha), 0);

        QVector3D new_topLeft  (new_front_x - halfLen*cos(alpha), new_front_y + halfLen*sin(alpha), 0);

        double new_back_x = x_new + halfLen*sin(alpha);
        double new_back_y = y_new + halfLen*cos(alpha);

        QVector3D new_botRight (new_back_x + halfLen*cos(alpha), new_back_y - halfLen*sin(alpha), 0);
        QVector3D new_botLeft  (new_back_x - halfLen*cos(alpha), new_back_y + halfLen*sin(alpha), 0);

        QVector<QVector3D> squad;
        squad.append(new_center);
        squad.append(new_topRight);
        squad.append(new_topLeft);
        squad.append(new_botLeft);
        squad.append(new_botRight);
        orientation = alpha;
        return squad;
    } else{
        QVector<QVector3D> squad(1);
        squad.append(new_center);
        orientation = 0;
        return squad;
    }


}

double ImageAnayser::orientationOfMarker (QVector3D center, QVector3D topRight, QVector3D topLeft, float marker_size){
    double xc = center.x();
    double yc = center.y();
    double xl = topLeft.x();
    double yl = topLeft.y();
    double xr = topRight.x();
    double yr = topRight.y();

    double xt = (xl + xr)/2.0;
    double yt = (yl + yr)/2.0;
    double l = ((double)marker_size)/2.0;
    double range = 0.0;



    double dy = yc - yt; //positive when facing like the origin tag
    double dx = xc - xt; //positive when tilting in the mathematical orientation

    range = l * cos(PI/6.0);

    if(dy > range){
        return 0;
    } else if(dy < -range){
        return PI;
    } else {
        double factor = (dy > 0.0) ? 1.0 : 2.0;
        double sign = (dx > 0.0)? -1.0 : 1.0;
        return sign * factor * (PI/3.0);
    }


}