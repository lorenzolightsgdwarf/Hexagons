#include "imageanayser.h"
#include <QVector4D>
#include <QImage>
#include <QFile>
#include <QTextStream>

#include <float.h>
ImageAnayser::ImageAnayser(QObject *parent):
    QObject(parent)
{
    generate_valid_positions();
}

bool ImageAnayser::run(QVariantList markers, QString origin)
{

    if(0!=reconstruct_board(markers, MARKER_LEN, MARKER_DIST, MARKER_ELEV, origin)){
        qDebug() << "something went wrong in the image conversion";
        return false;
    }

    return true;

}



int ImageAnayser::reconstruct_board(QVariantList markers, float marker_size, float marker_distance, float marker_elevation, QString origin){

    m_poses.clear();
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

    int indexOfOrigin = getOriginMarkerID(_markers, origin);
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

        if(new_squad.size()>1){
            qDebug() << "Marker:"<<id;
            qDebug() <<"Center" << new_squad.at(0);
            qDebug() << "Top Rigth" <<new_squad.at(1);
            qDebug() << "Top Left" << new_squad.at(2);
            qDebug() << "Bottom Left" << new_squad.at(3);
            qDebug() << "Bottom Right" << new_squad.at(4);
            qDebug() << "Distance to Origin" << new_squad.at(0).length();
            qDebug() << "Orientation" << orientation*180/PI;

            m_poses[id]=QVector4D(new_squad.at(0).x(),new_squad.at(0).y(),new_squad.at(0).z(),orientation);
        }
    }

    emit posesChanged();

    return 0;

}


bool ImageAnayser::write_output_file(){

    if(m_poses.size()==0) return false;

#ifdef ANDROID
    QString extstr=QString(getenv("EXTERNAL_STORAGE"))+"/staTIc/";
    QFile output_file(extstr+"board_configuration.data");
#else
    QFile output_file("board_configuration.data");
#endif
      if(!output_file.open(QFile::WriteOnly| QFile::Text)){
          qDebug()<<"Error in opening output file";
          return false;
      }
      QTextStream output_stream(&output_file);
      output_stream<<m_poses.size()<<"\n";
      QMatrix4x4 rotX(1,0,0,0,
                      0,cos(0.143117),-sin(0.143117),0,
                      0,sin(0.143117),cos(0.143117),0,
                      0,0,0,1);
      Q_FOREACH(QString marker_id,m_poses.keys()){
          QString marker_number=marker_id;
          output_stream<<"\n"<<marker_number.remove(0,4).toInt()<<"\n";
          output_stream<<MARKER_LEN<<"\n"<<"\n";

          QVector4D pose=m_poses[marker_id].value<QVector4D>();
          double angleZ=pose.w();
          QMatrix4x4 rotZ(cos(angleZ),-sin(angleZ),0,0,
                          sin(angleZ),cos(angleZ),0,0,
                          0,0,1,0,
                          0,0,0,1);
          QMatrix4x4 compRot=rotZ*rotX;
          output_stream<<compRot(0,0)<<" "<<compRot(0,1)<<" "<<compRot(0,2)<<" "<<pose.x()<<"\n";
          output_stream<<compRot(1,0)<<" "<<compRot(1,1)<<" "<<compRot(1,2)<<" "<<pose.y()<<"\n";
          output_stream<<compRot(2,0)<<" "<<compRot(2,1)<<" "<<compRot(2,2)<<" "<<pose.z()<<"\n";
      }
      output_stream.flush();
      output_file.close();
      return true;
}

void ImageAnayser::change_pose(QString id, qreal x, qreal y, qreal orientation)
{
        QVector4D pose=m_poses[id].value<QVector4D>();
        pose.setX(x);
        pose.setY(y);
        pose.setW(orientation*M_PI/180.0);
        m_poses[id]=pose;
        emit posesChanged();

}


int ImageAnayser::getOriginMarkerID(QList<QVariantMap> markers, QString origin){
    for(int i=0;i<markers.size();i++){
        QVariantMap v=markers.at(i);
        if(v["id"].toString()==origin)
            return i;
    }
    return -1;
}


QVector<QVector3D> ImageAnayser::approximations(QVector<QVector3D> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation){


    bool uncertainty_on_x=false;
    bool uncertainty_on_y=false;


    double y_chunk = ((double) marker_distance) * sin(PI/6.0);//Pay attention to the tile orientation!!!

    double y_mod = fmod(((double) exact[0].y()), y_chunk);

    double to_add = 0;

    if(fabs(y_mod) > y_chunk/3.0 && fabs(y_mod) < 2*y_chunk/3.0 )
           uncertainty_on_y=true;

    if(fabs(y_mod) > y_chunk/3.0){
        if(y_mod>0)
            to_add = y_chunk;
        else
            to_add = -y_chunk;
    }

    float y_new = ( exact[0].y()) - (y_mod) + (to_add);


    double x_chunk = ((double)marker_distance) * cos(PI/6.0);
    double x_dist = exact[0].x();
    double x_mod = fmod(( x_dist), (x_chunk));
//    if(x_mod < 0){
//        x_mod += x_chunk;
//    }

    to_add=0.0;

    if(fabs(x_mod) > ( x_chunk/3.0) && fabs(x_mod) < ( 2*x_chunk/3.0))
           uncertainty_on_x=true;

    if(fabs(x_mod) > ( x_chunk/3.0)){
        if(x_mod>0)
            to_add = x_chunk;
        else
            to_add = -x_chunk;
    }
    float x_new = x_dist - ( x_mod) + (to_add);


    if(uncertainty_on_x && uncertainty_on_y){
        QVector2D best_alternative(DBL_MAX,DBL_MAX);
        Q_FOREACH(QVector2D valid_position,m_valid_positions){
            if(exact[0].toVector2D().distanceToPoint(valid_position)< exact[0].toVector2D().distanceToPoint(best_alternative))
                best_alternative=exact[0].toVector2D();
        }
        x_new=best_alternative.x();
        y_new=best_alternative.y();
    }
    else if(uncertainty_on_x){
        qreal best_x= DBL_MAX;
        Q_FOREACH(QVector2D valid_position,m_valid_positions){
            if(qFuzzyCompare(valid_position.y(),y_new)){
                 if(fabs(exact[0].x()-valid_position.x()) < fabs(exact[0].x()-best_x))
                     best_x=valid_position.x();
            }
        }
        x_new=best_x;
    }
    else if(uncertainty_on_y){
        qreal best_y= DBL_MAX;
        Q_FOREACH(QVector2D valid_position,m_valid_positions){
            if(qFuzzyCompare(valid_position.x(),x_new)){
                 if(fabs(exact[0].y()-valid_position.y()) < fabs(exact[0].y()-best_y))
                     best_y=valid_position.y();
            }
        }
        y_new=best_y;
    }
    else{
        bool is_valid=false;
        Q_FOREACH(QVector2D valid_position,m_valid_positions)
            if(qFuzzyCompare(valid_position,QVector2D(x_new,y_new))){
                is_valid=true;
                break;
            }
        if(!is_valid)
            return QVector<QVector3D>();

    }

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

void ImageAnayser::generate_valid_positions()
{
    float max_circle=500;
    QList<QVector2D> directions;
    directions<<QVector2D(cos(M_PI/6)*MARKER_DIST,sin(M_PI/6)*MARKER_DIST)<<QVector2D(0,MARKER_DIST)
              <<QVector2D(-cos(M_PI/6)*MARKER_DIST,sin(M_PI/6)*MARKER_DIST)<<QVector2D(-cos(M_PI/6)*MARKER_DIST,-sin(M_PI/6)*MARKER_DIST)
              <<QVector2D(0,-MARKER_DIST)<<QVector2D(cos(M_PI/6)*MARKER_DIST,-sin(M_PI/6)*MARKER_DIST);

    QList<QVector2D> to_visit;

    to_visit<<QVector2D(0,0);

    QVector2D current;

    while(!to_visit.isEmpty()){
          current=to_visit.first();
          to_visit.pop_front();

          if(current.length()>max_circle)
              continue;

          bool skip=false;

          Q_FOREACH(QVector2D prev_center, m_valid_positions )
              if(prev_center.distanceToPoint(current)<1){
                  skip=true;
                  break;
              }

          if(skip)
              continue;

          m_valid_positions.append(current);

          Q_FOREACH(QVector2D dir, directions)
              to_visit.append(current+dir);

    }
}
