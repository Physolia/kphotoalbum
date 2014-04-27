/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Action.h"
#include "RemoteInterface.h"
#include "ScreenInfo.h"


namespace RemoteControl {


Action::Action(const SearchInfo& searchInfo)
    :m_searchInfo(searchInfo)
{
}

void Action::run()
{
    execute();
    RemoteInterface::instance().m_search = m_searchInfo;
}

void Action::setCurrentPage(Page page)
{
    RemoteInterface::instance().setCurrentPage(page);
}

void Action::sendCommand(const RemoteCommand& command)
{
    RemoteInterface::instance().sendCommand(command);
}

void Action::clearCategoryModel()
{
    RemoteInterface::instance().m_categoryItems->setImages({});
}




ShowOverviewAction::ShowOverviewAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowOverviewAction::execute()
{
    int size = ScreenInfo::instance().overviewIconSize();
    sendCommand(SearchCommand(SearchType::Categories, m_searchInfo, size));
    setCurrentPage(Page::OverviewPage);
}

ShowCategoryValueAction::ShowCategoryValueAction(const SearchInfo& searchInfo, CategoryViewType type)
    :Action(searchInfo), m_type(type)
{
}

void ShowCategoryValueAction::execute()
{
    sendCommand(SearchCommand(SearchType::CategoryItems, m_searchInfo));
    clearCategoryModel();
    if (m_type == CategoryViewType::CategoryIconView)
        setCurrentPage(Page::CategoryItemsPage);
    else
        setCurrentPage(Page::CategoryListPage);
}

ShowThumbnailsAction::ShowThumbnailsAction(const SearchInfo& searchInfo)
    :Action(searchInfo)
{
}

void ShowThumbnailsAction::execute()
{
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Thumbnail);
    setCurrentPage(Page::ThumbnailsPage);
}

ShowImagesAction::ShowImagesAction(int imageId, const SearchInfo& searchInfo)
    :Action(searchInfo), m_imageId(imageId)
{
}

void ShowImagesAction::execute()
{
    setCurrentPage(Page::ImageViewerPage);
    RemoteInterface::instance().setCurrentView(m_imageId);
}

DiscoverAction::DiscoverAction(const SearchInfo& searchInfo)
 : Action(searchInfo)
{
}

void DiscoverAction::execute()
{
    setCurrentPage(Page::DiscoverPage);
    RemoteInterface::instance().setActiveThumbnailModel(RemoteInterface::ModelType::Discovery);
    sendCommand(SearchCommand(SearchType::Images, m_searchInfo));
}

} // namespace RemoteControl
