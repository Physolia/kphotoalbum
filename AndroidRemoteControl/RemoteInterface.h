#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "CategoryModel.h"
#include "RemoteCommand.h"
#include "SearchInfo.h"
#include "ThumbnailModel.h"
#include "DiscoveryModel.h"

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
    Q_PROPERTY(QImage discover MEMBER m_discoveryImage NOTIFY discoverImageChanged)
    Q_PROPERTY(RemoteControl::Types::Page currentPage MEMBER m_currentPage NOTIFY currentPageChanged)
    Q_PROPERTY(ThumbnailModel* thumbnailModel MEMBER m_thumbnailModel NOTIFY thumbnailModelChanged)
    Q_PROPERTY(QStringList listCategoryValues MEMBER m_listCategoryValues NOTIFY listCategoryValuesChanged)
    Q_PROPERTY(DiscoveryModel* discoveryModel MEMBER m_discoveryModel NOTIFY discoveryModelChanged)

public:
    static RemoteInterface& instance();
    bool isConnected() const;
    void sendCommand(const RemoteCommand& command);
    QString currentCategory() const;
    QImage discoveryImage() const;

public slots:
    void goHome();
    void goBack();
    void goForward();
    void selectCategory(const QString& category, int /*CategoryViewType*/ type);
    void selectCategoryValue(const QString& value);
    void showThumbnails();
    void showImage(int imageId);
    void showDiscoveredImage(int imageId);
    void requestDetails(int imageId);
    void activateSearch(const QString& search);
    void doDiscover();

signals:
    void connectionChanged();
    void categoriesChanged();
    void homeImageChanged();
    void kphotoalbumImageChange();    
    void categoryItemsChanged();
    void currentPageChanged();
    void thumbnailModelChanged();
    void jumpToImage(int index);
    void listCategoryValuesChanged();

    void discoverImageChanged();
    void discoveryModelChanged();

public:
    void setCurrentView(int imageId);

private slots:
    void requestInitialData();
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateCategoryList(const CategoryListCommand&);
    void gotSearchResult(const SearchResultCommand&);
    void requestHomePageImages();

private:
    RemoteInterface();
    friend class Action;
    friend class ShowDiscoveredImage; // FIXME: REMOVE
    void setCurrentPage(Page page);
    void setListCategoryValues(const QStringList& values);
    void setHomePageImages(const HomePageData& command);

    Client* m_connection = nullptr;
    CategoryModel* m_categories;
    QImage m_homeImage;
    QImage m_kphotoalbumImage;
    SearchInfo m_search;
    ThumbnailModel* m_categoryItems;
    RemoteControl::Page m_currentPage = RemoteControl::Page::UnconnectedPage;
    ThumbnailModel* m_thumbnailModel;
    History m_history;
    QStringList m_listCategoryValues;
    QImage m_discoveryImage;
    DiscoveryModel* m_discoveryModel;
};

}

#endif // REMOTEINTERFACE_H
