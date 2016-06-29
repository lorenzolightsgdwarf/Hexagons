#include "filterfactory.h"
FilterFactory::FilterFactory(QObject* p):QAbstractVideoFilter(p)
{
}

QVideoFilterRunnable *FilterFactory::createFilterRunnable()
{
    Filter* filter=new Filter(this);
    connect(filter,SIGNAL(newCorners(QList<QVector3D>)),this,SLOT(setCorners(QList<QVector3D>)));
    return filter;
}

