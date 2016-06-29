#include "imageanayser.h"
#include <QVector2D>
#include <QImage>
#include <stdio.h>


ImageAnayser::ImageAnayser(QObject *parent):
    QObject(parent)
{

    m_focalLength = 700.0;
    camMatrix3by3  = (cv::Mat_<float>(3,3) <<
                      m_focalLength, 0, 640/2.f,
                      0, m_focalLength, 384/2.f,
                      0, 0, 1);
    distCoeffs  = (cv::Mat_<float>(1,5) <<
                   4.1105624753282284e-02, 7.9058604507765057e-02,
                         -2.7890683219554159e-03, 2.8142764725885486e-04,
                         -4.0278534170454272e-01);
    dictionaryId = cv::aruco::DICT_6X6_1000;

    dictionary  = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));
    detectorParams  = aruco::DetectorParameters::create();
        detectorParams->doCornerRefinement = true; // do corner refinement in markers

}

void ImageAnayser::run(QString img, int origin)
{
    int fail;

    string filename = img.toStdString();

    Mat image = imread(filename, CV_LOAD_IMAGE_COLOR);

    Size size(640, 360);
    Mat dst;//dst image


    resize(image,dst,size);//resize image

    if(0!= (fail = reconstruct_board(dst, MARKER_LEN, MARKER_DIST, MARKER_ELEV, origin))){
        qDebug() << "something went wrong in the image conversion";
    }

    remove(filename.c_str());

}



int ImageAnayser::reconstruct_board(Mat image, float marker_size, float marker_distance, float marker_elevation, int origin){

    vector< int > ids;
    vector< vector< Point2f > > corners, rejected;
    vector< Vec3d > rvecs, tvecs;
    Matx33d rmat;

    Mat matC = (cv::Mat_<float>(4,1) <<
                        0, 0, 0, 1);
    Mat matTR = (cv::Mat_<float>(4,1) <<
                        marker_size/2, -marker_size/2, 0, 1);
    Mat matTL = (cv::Mat_<float>(4,1) <<

                        -marker_size/2, -marker_size/2, 0, 1);
    Mat matBR = (cv::Mat_<float>(4,1) <<
                        marker_size/2, marker_size/2, 0, 1);
    Mat matBL = (cv::Mat_<float>(4,1) <<
                        -marker_size/2, marker_size/2, 0, 1);


    aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

    if(ids.size() > 0){
        aruco::estimatePoseSingleMarkers(corners, marker_size, camMatrix3by3, distCoeffs, rvecs, tvecs);}

    if(ids.size() > 1){

            string ymlFileName = "test.yml";
            cv::FileStorage fs(ymlFileName, cv::FileStorage::WRITE);
                fs<<"dictionary"<<dictionaryId;
                fs<<"ids"<<ids;

            int indexOfOrigin = getBottomLeftMarker(ids, origin);
            //int indexOfOther = getBottomLeftMarker(ids, 18);


            //calculate the rotation matrix of the origin tag
            cv::Rodrigues(rvecs[indexOfOrigin], rmat);
            Vec3d originP = tvecs[indexOfOrigin];

            //calculate the transformation matrix of the origin tag
            Mat originM = calcTransfMat(originP, rmat);
            Mat originInv = originM.inv();

            size_t numberOfMarkers = ids.size();

            for(size_t i = 0; i < numberOfMarkers; i++){

                    fs<<"Point id"<< ids.at(i);

                    //calculate the rotation matrix of the tag i
                    cv::Rodrigues(rvecs[i], rmat);

                    //calculate the transformation matrix of the tag i
                    Mat matrixOf_i = calcTransfMat(tvecs[i], rmat);

                    //calculate the transformation matrix to get from the origin tag to the tag i
                    Mat multipM = originInv * matrixOf_i;


                    Mat cMat = multipM * matC ;
                    Mat trMat = multipM * matTR ;
                    Mat tlMat = multipM * matTL ;
                    Mat blMat = multipM * matBL ;
                    Mat brMat = multipM * matBR ;

                    //coordinates of the corners and the center of the tag i
                    Vec3d cVec  =  cMat.rowRange(0,3);
                    Vec3d trVec = trMat.rowRange(0,3);
                    Vec3d tlVec = tlMat.rowRange(0,3);
                    Vec3d blVec = blMat.rowRange(0,3);
                    Vec3d brVec = brMat.rowRange(0,3);

                    QVector<Vec3d> squad(5);
                    squad = {cVec, trVec, tlVec, blVec, brVec};

                    QVector<Vec3d> new_squad(5);
                    double orientation;
                    new_squad = approximations(squad, marker_distance, marker_size, marker_elevation, orientation);

                    fs <<"Center" << new_squad[0];
                    fs << "Top Rigth" << new_squad[1];
                    fs << "Top Left" << new_squad[2];
                    fs << "Bottom Left" << new_squad[3];
                    fs << "Bottom Right" << new_squad[4];
                    fs << "Distance to Origin" << norm(new_squad[0]);
                    fs << "Orientation" << orientation*180/PI;
            }

            fs.release();
    }

    return 0;

}

QList<QVector3D> ImageAnayser::centers(Mat image, float marker_size, float marker_distance, float marker_elevation, int origin)
{
    vector< int > ids;
    vector< vector< Point2f > > corners, rejected;
    vector< Vec3d > rvecs, tvecs;
    Matx33d rmat;
    QList<QVector3D> centers;

    aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);

    size_t number_of_tags = corners.size();

    for(size_t i = 0; i < number_of_tags; i++){
        for(size_t c = 0; c < 4; c++){
            Point2f pt = corners[i][c];
            QVector3D toAdd(pt.x, pt.y, ids.at(i));
            centers.append(toAdd);
        }
    }

    return centers;
}



int ImageAnayser::getBottomLeftMarker(vector<int> array, int origin){
    size_t size = array.size();
    for(size_t i = 0; i < size; i++){
        if(array[i] == origin){
            return i;
        }
    }
}

Mat ImageAnayser::calcTransfMat (Vec3d tvec, Matx33d rotation){
    Mat final = (cv::Mat_<float>(4,4) <<
                 rotation.operator ()(0,0), rotation.operator ()(0,1), rotation.operator ()(0,2), tvec[0],
                 rotation.operator ()(1,0), rotation.operator ()(1,1), rotation.operator ()(1,2), tvec[1],
                 rotation.operator ()(2,0), rotation.operator ()(2,1), rotation.operator ()(2,2), tvec[2],
                                         0,                         0,                         0,       1);
    return final;
}

QVector<Vec3d> ImageAnayser::approximations(QVector<Vec3d> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation){
    double y_chunk = ((double) marker_distance) * cos(PI/3.0);

    double y_mod = fmod(((double) exact[0][1]), y_chunk);
    if(y_mod < 0.0){
        y_mod += y_chunk;
    }


    double to_add = 0;
    if(y_mod > y_chunk/2.0) to_add = y_chunk;

    double y_new = ( exact[0][1]) - (y_mod) + (to_add);

    double x_chunk = ((double)marker_distance) * cos(PI/6.0);
    double x_dist = exact[0][0];
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


    Vec3d new_center = {x_new, y_new, 0.0};

    if(exact.size() == 5){
    double alpha = orientationOfMarker(exact[0], exact[1], exact[2], marker_size);
    double halfLen = ((double)marker_size)/2.0;

    double new_front_x = x_new - halfLen*sin(alpha);
    double new_front_y = y_new - halfLen*cos(alpha);

    Vec3d new_topRight = {new_front_x + halfLen*cos(alpha), new_front_y - halfLen*sin(alpha), 0};

    Vec3d new_topLeft  = {new_front_x - halfLen*cos(alpha), new_front_y + halfLen*sin(alpha), 0};

    double new_back_x = x_new + halfLen*sin(alpha);
    double new_back_y = y_new + halfLen*cos(alpha);

    Vec3d new_botRight = {new_back_x + halfLen*cos(alpha), new_back_y - halfLen*sin(alpha), 0};
    Vec3d new_botLeft  = {new_back_x - halfLen*cos(alpha), new_back_y + halfLen*sin(alpha), 0};

    QVector<Vec3d> squad(5);
    squad = {new_center, new_topRight, new_topLeft, new_botLeft, new_botRight};
    orientation = alpha;
    return squad;} else{
        QVector<Vec3d> squad(1);
        squad = {new_center};
        orientation = 0;
        return squad;
    }


}

double ImageAnayser::orientationOfMarker (Vec3d center, Vec3d topRight, Vec3d topLeft, float marker_size){
    double xc = center[0];
    double yc = center[1];
    double xl = topLeft[0];
    double yl = topLeft[1];
    double xr = topRight[0];
    double yr = topRight[1];

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
