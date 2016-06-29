#include "filter.h"


Filter::Filter(QObject *parent)
{

}

Filter::~Filter()
{

}

QVideoFrame Filter::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    auto pixelFormat=surfaceFormat.pixelFormat();
    auto handle=input->handleType();
    cv::Mat mat;
    ImageAnayser image_analyser;
    //Mat img_centers;
    QList<QVector3D> centers;

    switch (handle) {
        case QAbstractVideoBuffer::NoHandle: ///< Mainly desktop configuration
            if(input->map(QAbstractVideoBuffer::ReadOnly)){
                if( QVideoFrame::imageFormatFromPixelFormat(pixelFormat)!=QImage::Format_Invalid ){
                    QImage img( input->bits(),
                                input->width(),
                                input->height(),
                                input->bytesPerLine(),
                                QVideoFrame::imageFormatFromPixelFormat(pixelFormat));
                    img=img.convertToFormat(QImage::Format_Grayscale8);
                    mat=cv::Mat(img.height(), img.width(), CV_8UC1,
                                const_cast<uchar*>(img.constBits()), img.bytesPerLine()).clone();

                }
                else if (pixelFormat == QVideoFrame::PixelFormat::Format_YUV420P ||
                         pixelFormat == QVideoFrame::PixelFormat::Format_NV21 ||
                         pixelFormat == QVideoFrame::PixelFormat::Format_NV12
                         ) {
                    mat=cv::Mat(input->height(), input->width(),CV_8UC1 ,
                                const_cast<uchar*>(input->bits())).clone();

                }
                else{
                    qWarning()<<" Handle: NoHandle, unsupported pixel format:"<<pixelFormat;
                }
                input->unmap();
            }
            else{
                qWarning()<<"Cannot map video buffer";
            }
            break;
        case QAbstractVideoBuffer::GLTextureHandle:
            if ( pixelFormat == QVideoFrame::Format_BGR32){
            }
            else{
                qWarning()<<"Handle: GLTextureHandle, unsupported pixel format:"<<pixelFormat;
            }
            break;
        default:
            qWarning()<<"Unsupported Video Frame Handle:"<<handle;
            break;
        }

    centers = image_analyser.centers(mat, MARKER_LEN, MARKER_DIST, MARKER_ELEV, MARKER_ORIGIN);
    emit newCorners(centers);

    return *input;
}
