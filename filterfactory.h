#ifndef FILTERFACTORY_H
#define FILTERFACTORY_H

#include <QtMultimedia>
#include "imageanayser.h"
#include "filter.h"
#include "opencv2/imgproc.hpp"
#include <opencv2/video.hpp>


class FilterFactory : public QAbstractVideoFilter {
    Q_OBJECT
    Q_PROPERTY(QVariantList corners READ corners NOTIFY cornersChanged)
public:
    FilterFactory(QObject* parent=0);
    QVideoFilterRunnable *createFilterRunnable();
    QVariantList corners(){
        QVariantList list;
        Q_FOREACH(QVector3D v,m_corners)
            list.append(v);
        return list;
    }

private slots:
public slots:
    void setCorners(QList<QVector3D> corners){m_corners=corners;emit cornersChanged();}
signals:
    void cornersChanged();

private:
    QList<QVector3D> m_corners;
};

#endif // FILTERFACTORY_H
