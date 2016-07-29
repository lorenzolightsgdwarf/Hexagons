#ifndef IMAGEANAYSER_H
#define IMAGEANAYSER_H
#include <iostream>
#include <math.h>
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

class ImageAnayser: public QObject
{
    Q_OBJECT
public:
    explicit ImageAnayser(QObject* parent =0);


    Q_INVOKABLE void run(QVariantList markers, QString origin);
    int reconstruct_board(QVariantList markers, float marker_size, float marker_distance, float marker_elevation, QString origin);

    int getBottomLeftMarker(QList<QVariantMap> markers, QString origin);

    QVector<QVector3D> approximations(QVector<QVector3D> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation);

    double orientationOfMarker (QVector3D center, QVector3D topRight, QVector3D topLeft, float marker_size);

};

#endif // IMAGEANAYSER_H