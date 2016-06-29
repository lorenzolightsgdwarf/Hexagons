#ifndef IMAGEANAYSER_H
#define IMAGEANAYSER_H

#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <math.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <QMatrix4x4>
#include <QObject>
#include <QImage>

#define PI 3.14159265
#define WIDTH 3600 // was 640
#define HEIGHT 2160 // was 480
#define RED_RADIUS 20
#define RED_THICKNESS -1 //-1 for filled
#define RED_LINETYPE 8

#define MARKER_LEN 35.f
#define MARKER_DIST 90.f
#define MARKER_ORIGIN 0
#define MARKER_ELEV 0.f


using namespace std;
using namespace cv;


class ImageAnayser: public QObject
{
    Q_OBJECT
public:
    explicit ImageAnayser(QObject* parent =0);

    double m_focalLength;
    int dictionaryId;
    Mat camMatrix3by3, distCoeffs;

    Ptr<aruco::Dictionary> dictionary;
    Ptr<aruco::DetectorParameters> detectorParams;
    Q_INVOKABLE void run(QString img, int origin = 0);
    int reconstruct_board(Mat image, float marker_size, float marker_distance, float marker_elevation, int origin);

    QList<QVector3D> centers(Mat image, float marker_size, float marker_distance, float marker_elevation, int origin);

    int getBottomLeftMarker(vector<int> array, int origin);

    Mat calcTransfMat (Vec3d tvec, Matx33d rotation);

    QVector<Vec3d> approximations(QVector<Vec3d> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation);

    double orientationOfMarker(Vec3d center, Vec3d topRight, Vec3d TopLeft, float marker_size);

};

#endif // IMAGEANAYSER_H
