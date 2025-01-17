// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <DB/ImageInfo.h>
#include <ImageManager/ImageClientInterface.h>

#include <QLabel>
#include <QTimer>

class QResizeEvent;
class QRubberBand;

namespace AnnotationDialog
{
class ResizableFrame;

class ImagePreview : public QLabel, public ImageManager::ImageClientInterface
{
    Q_OBJECT
public:
    explicit ImagePreview(QWidget *parent);
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    void rotate(int angle);
    void setImage(const DB::ImageInfo &info);
    void setImage(const QString &fileName);
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    QRect areaPreviewToActual(QRect area) const;
    QRect minMaxAreaPreview() const;
    void createTaggedArea(QString category, QString tag, QRect geometry, bool showArea);
    QSize getActualImageSize();
    void acceptProposedTag(QPair<QString, QString> tagData, ResizableFrame *area);
    QPixmap grabAreaImage(QRect area);

public Q_SLOTS:
    void setAreaCreationEnabled(bool state);

Q_SIGNALS:
    void areaCreated(ResizableFrame *area);
    void proposedTagSelected(QString category, QString tag);

protected:
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void reload();
    void setCurrentImage(const QImage &image);
    QImage rotateAndScale(QImage, int width, int height, int angle) const;
    void updateScaleFactors();

    QRect areaActualToPreview(QRect area) const;
    void processNewArea();
    void remapAreas();
    void rotateAreas(int angle);

    class PreviewImage
    {
    public:
        bool has(const DB::FileName &fileName, int angle) const;
        QImage &getImage();
        void set(const DB::FileName &fileName, const QImage &image, int angle);
        void set(const PreviewImage &other);
        void setAngle(int angle);
        void reset();

    protected:
        DB::FileName m_fileName;
        QImage m_image;
        int m_angle;
    };

    struct PreloadInfo {
        PreloadInfo();
        void set(const DB::FileName &fileName, int angle);
        DB::FileName m_fileName;
        int m_angle;
    };

    class PreviewLoader : public ImageManager::ImageClientInterface, public PreviewImage
    {
    public:
        void preloadImage(const DB::FileName &fileName, int width, int height, int angle);
        void cancelPreload();
        void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    };
    PreviewLoader m_preloader;

protected Q_SLOTS:
    void resizeFinished();

private:
    DB::ImageInfo m_info;
    QString m_fileName;
    PreviewImage m_currentImage, m_lastImage;
    PreloadInfo m_anticipated;
    int m_angle = 0;
    int m_minX = 0;
    int m_maxX = 0;
    int m_minY = 0;
    int m_maxY = 0;
    QPoint m_areaStart;
    QPoint m_areaEnd;
    QPoint m_currentPos;
    QRubberBand *m_selectionRect = nullptr;
    double m_scaleWidth = 0;
    double m_scaleHeight = 0;
    double m_aspectRatio = 1;
    QTimer *m_reloadTimer = nullptr;
    bool m_areaCreationEnabled = false;
    QImage m_fullSizeImage;

    void createNewArea(QRect geometry, QRect actualGeometry);
    QRect rotateArea(QRect originalAreaGeometry, int angle);
    void fetchFullSizeImage();
    bool fuzzyAreaExists(QList<QRect> &existingAreas, QRect area);
    float distance(QPoint point1, QPoint point2);
};
}

#endif /* IMAGEPREVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
