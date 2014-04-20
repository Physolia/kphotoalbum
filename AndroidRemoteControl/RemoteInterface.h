#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "RemoteCommand.h"
#include "SearchInfo.h"
#include "ThumbnailModel.h"

#include <QObject>
#include <QMap>
#include <QImage>
#include <QStringList>
#include "History.h"
#include "Types.h"

class QTcpSocket;

namespace RemoteControl {

class Client;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )
    Q_PROPERTY(RemoteControl::CategoryModel* categories MEMBER m_categories NOTIFY categoriesChanged)
    Q_PROPERTY(ThumbnailModel* categoryItems MEMBER m_categoryItems NOTIFY categoryItemsChanged)
    Q_PROPERTY(QImage home MEMBER m_homeImage NOTIFY homeImageChanged)
    Q_PROPERTY(QImage kphotoalbum MEMBER m_kphotoalbumImage NOTIFY kphotoalbumImageChange)
    Q_PROPERTY(RemoteControl::Types::Page currentPage MEMBER m_currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(ThumbnailModel* thumbnailModel MEMBER m_thumbnailModel NOTIFY thumbnailModelChanged)

public:
    static RemoteInterface& instance();
    bool isConnected() const;
    void sendCommand(const RemoteCommand& command);
    QString currentCategory() const;

public slots:
    void goHome();
    void goBack();
    void goForward();
    void selectCategory(const QString& category);
    void selectCategoryValue(const QString& value);
    void showThumbnails();
    void showImage(int imageId);
    void requestDetails(int imageId);
    void activateSearch(const QString& search);

signals:
    void connectionChanged();
    void categoriesChanged();
    void homeImageChanged();
    void kphotoalbumImageChange();    
    void categoryItemsChanged();
    void currentPageChanged();
    void thumbnailModelChanged();
    void jumpToImage(int index);

public:
    void setCurrentView(int imageIf);

private slots:
    void requestInitialData();
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateCategoryList(const CategoryListCommand&);
    void gotSearchResult(const SearchResultCommand&);

private:
    RemoteInterface();
    friend class Action;
    void setCurrentPage(Page page);

    Client* m_connection = nullptr;
    CategoryModel* m_categories;
    QImage m_homeImage;
    QImage m_kphotoalbumImage;
    SearchInfo m_search;
    ThumbnailModel* m_categoryItems;
    RemoteControl::Page m_currentPage = RemoteControl::Page::UnconnectedPage;
    ThumbnailModel* m_thumbnailModel;
    History m_history;
};

}

#endif // REMOTEINTERFACE_H
