#ifndef FILTER_H
#define FILTER_H

#include "filterfactory.h"

class Filter : public QObject, public QVideoFilterRunnable{
Q_OBJECT
public:
    Filter(QObject* parent);
    ~Filter();
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, RunFlags flags);

signals:
    void newCorners(QList<QVector3D>);

private:

};

#endif // FILTER_H
