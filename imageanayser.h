#ifndef IMAGEANAYSER_H
#define IMAGEANAYSER_H
#include <math.h>
#include <QMatrix4x4>
#include <QObject>
#include <QImage>
#include <QVector2D>

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


/*GUI Works only for 640x480
*/

class ImageAnayser: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap poses READ poses NOTIFY posesChanged)
private:
    bool write_output_file(QList<QVector4D> markers,QList<double> orientations);
    int reconstruct_board(QVariantList markers, float marker_size, float marker_distance, float marker_elevation, QString origin);

    int getOriginMarkerID(QList<QVariantMap> markers, QString origin);

    QVector<QVector3D> approximations(QVector<QVector3D> exact, float marker_distance, float marker_size, float marker_elevation, double &orientation);

    double orientationOfMarker (QVector3D center, QVector3D topRight, QVector3D topLeft, float marker_size);

    void generate_valid_positions();

    QVariantMap m_poses;
    QList<QVector2D> m_valid_positions;

signals:
    void posesChanged();
public:
    explicit ImageAnayser(QObject* parent =0);

    QVariantMap poses(){return m_poses;}
    Q_INVOKABLE bool run(QVariantList markers, QString origin);



};

#endif // IMAGEANAYSER_H
