#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include <QObject>
#include <QImage>
#include <QMap>
#include "Types.h"
#include <QMutex>

namespace RemoteControl {
class RemoteImage;

class ImageStore : public QObject
{
    Q_OBJECT
public:
    static ImageStore& instance();
    void updateImage(const QString& fileName, const QImage& image, ViewType type);
    QImage image(RemoteImage* client, const QString& fileName, const QSize& size, ViewType type);

private slots:
    void reset();
    void clientDeleted();

private:
    explicit ImageStore();
    void requestImage(RemoteImage* client, const QString& fileName, const QSize& size, ViewType type);

    using RequestType = QPair<QString,ViewType>;
    QMap<RequestType,QImage> m_imageMap;
    QMap<RequestType,RemoteImage*> m_requestMap;
    QMap<RemoteImage*,RequestType> m_reverseRequestMap;
    QMutex m_mutex;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H
