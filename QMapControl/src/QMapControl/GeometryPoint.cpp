/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2007 - 2008 Kai Winter
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with QMapControl. If not, see <http://www.gnu.org/licenses/>.
*
* Contact e-mail: kaiwinter@gmx.de
* Program URL   : http://qmapcontrol.sourceforge.net/
*
*/

#include "GeometryPoint.h"

// STL includes.
#include <cmath>

// Local includes.
#include "Projection.h"

namespace qmapcontrol
{
    GeometryPoint::GeometryPoint(const qreal& longitude, const qreal& latitude, const QPixmap& pixmap, const int& zoom_minimum, const int& zoom_maximum)
        : Geometry(Geometry::GeometryType::GeometryPoint, zoom_minimum, zoom_maximum),
          m_point_coord(PointWorldCoord(longitude, latitude)),
          m_pixmap(std::make_shared<QPixmap>(pixmap)),
          m_alignment_type(AlignmentType::Middle),
          m_base_zoom(-1),
          m_base_size_px(pixmap.size()),
          m_draw_minimum_px(-1.0, -1.0),
          m_draw_maximum_px(-1.0, -1.0)
    {

    }

    GeometryPoint::GeometryPoint(const PointWorldCoord& point_coord, const QPixmap& pixmap, const int& zoom_minimum, const int& zoom_maximum)
        : Geometry(Geometry::GeometryType::GeometryPoint, zoom_minimum, zoom_maximum),
          m_point_coord(point_coord),
          m_pixmap(std::make_shared<QPixmap>(pixmap)),
          m_alignment_type(AlignmentType::Middle),
          m_base_zoom(-1),
          m_base_size_px(pixmap.size()),
          m_draw_minimum_px(-1.0, -1.0),
          m_draw_maximum_px(-1.0, -1.0)
    {

    }

    PointWorldCoord GeometryPoint::coord() const
    {
        // Return the longitude/latitude coordinate (x/y).
        return m_point_coord;
    }

    void GeometryPoint::setCoord(const PointWorldCoord& point)
    {
        // Has the point change.
        if(m_point_coord != point)
        {
            // Set the new point.
            m_point_coord = point;

            // Emit that we need to redraw to display this change.
            emit requestRedraw();

            // Emit that the position has changed.
            emit positionChanged(this);
        }
    }

    QPixmap& GeometryPoint::getPixmap() const
    {
        // Return the pixmap.
        return *(m_pixmap.get());
    }

    void GeometryPoint::setPixmap(const std::shared_ptr<QPixmap>& pixmap)
    {
        // Set the pixmap.
        m_pixmap = pixmap;

        // Was a valid pixmap set?
        if(pixmap != nullptr)
        {
            // Update the base size.
            m_base_size_px = pixmap->size();
        }

        // Emit that we need to redraw to display this change.
        emit requestRedraw();
    }

    void GeometryPoint::setPixmap(const QPixmap& pixmap)
    {
        // Set the pixmap.
        m_pixmap = std::make_shared<QPixmap>(pixmap);

        // Update the base size.
        m_base_size_px = pixmap.size();

        // Emit that we need to redraw to display this change.
        emit requestRedraw();
    }

    void GeometryPoint::setPen(const std::shared_ptr<QPen>& pen)
    {
        // Set the pen to draw with.
        Geometry::setPen(pen);

        // Update the pixmap.
        updatePixmap();
    }

    void GeometryPoint::setPen(const QPen& pen)
    {
        // Set the pen to draw with.
        Geometry::setPen(pen);

        // Update the pixmap.
        updatePixmap();
    }

    void GeometryPoint::setBrush(const std::shared_ptr<QBrush>& brush)
    {
        // Set the brush to draw with.
        Geometry::setBrush(brush);

        // Update the pixmap.
        updatePixmap();
    }

    void GeometryPoint::setBrush(const QBrush& brush)
    {
        // Set the brush to draw with.
        Geometry::setBrush(brush);

        // Update the pixmap.
        updatePixmap();
    }

    void GeometryPoint::setAlignmentType(const AlignmentType& alignment_type)
    {
        // Set the alignment type.
        m_alignment_type = alignment_type;
    }

    void GeometryPoint::setBaseZoom(const int& zoom)
    {
        // Set the base zoom level.
        m_base_zoom = zoom;
    }

    void GeometryPoint::setDrawMinimumPx(const QSizeF& size_px)
    {
        // Set the minimum size.
        m_draw_minimum_px = size_px;
    }

    void GeometryPoint::setDrawMaximumPx(const QSizeF& size_px)
    {
        // Set the maximum size.
        m_draw_maximum_px = size_px;
    }

    QRectF GeometryPoint::boundingBox(const int& controller_zoom) const
    {
        // Calculate the world point in pixels.
        const PointWorldPx point_px = projection::get().toPointWorldPx(m_point_coord, controller_zoom);

        // Calculate the current size for this controller zoom.
        const QSizeF object_size_px = calculateGeometrySizePx(controller_zoom);

        // Calculate the top-left point.
        const PointWorldPx top_left_point_px = calculateTopLeftPoint(point_px, m_alignment_type, object_size_px);

        // Calculate the bottom-right point.
        const PointWorldPx bottom_right_point_px(top_left_point_px.x() + object_size_px.width(), top_left_point_px.y() + object_size_px.height());

        // Return the converted coord points.
        return QRectF(projection::get().toPointWorldCoord(top_left_point_px, controller_zoom).rawPoint(), projection::get().toPointWorldCoord(bottom_right_point_px, controller_zoom).rawPoint());
    }

    bool GeometryPoint::touches(const QGraphicsItem& area_px, const int& controller_zoom)
    {
        /// @TODO This does not take into account the size of each GeometryPoint (ie: if it is a pixmap/widget instead of a point).

        // Default return success.
        bool return_touches(false);

        // Check the geometry is visible.
        if(isVisible(controller_zoom))
        {
            // Convert the geometry coord to pixels (quicker than converting polygon area into coords)
            const PointWorldPx point_px(projection::get().toPointWorldPx(m_point_coord, controller_zoom));

            // Is the geometry pixel contain within the polygon area?
            if(area_px.contains(point_px.rawPoint()))
            {
                // Emit that the geometry has been clicked.
                emit geometryClicked(this);

                // Set that we have touched.
                return_touches = true;
            }
        }

        // Return our success.
        return return_touches;
    }

    void GeometryPoint::draw(QPainter* painter, const QRectF& backbuffer_rect_px, const int& controller_zoom)
    {
        // Check the geometry is visible.
        if(isVisible(controller_zoom))
        {
            // Do we have a pixmap?
            if(m_pixmap != nullptr && m_pixmap->size().isEmpty() == false)
            {
                // Calculate the world point in pixels.
                const PointWorldPx point_px(projection::get().toPointWorldPx(m_point_coord, controller_zoom));

                // Update the object size for the pixmap size for this controller zoom.
                const QSizeF pixmap_size_px(calculateGeometrySizePx(controller_zoom));

                // Calculate the pixmap rect to draw within.
                const QRectF pixmap_rect(calculateTopLeftPoint(point_px, m_alignment_type, pixmap_size_px).rawPoint(), pixmap_size_px);

                // Does the pixmap rect intersect within the backbuffer rect?
                if(backbuffer_rect_px.intersects(pixmap_rect))
                {
                    // Draw the pixmap.
                    painter->drawPixmap(pixmap_rect, getPixmap(), QRectF());

                    // Do we have a meta-data value and should we display it at this zoom?
                    if(controller_zoom >= m_metadata_displayed_zoom_minimum && getMetadata(m_metadata_displayed_key).isNull() == false)
                    {
                        // Draw the text next to the point with an offset.
                        painter->drawText(pixmap_rect.topRight() + QPointF(m_metadata_displayed_alignment_offset_px, -m_metadata_displayed_alignment_offset_px), getMetadata(m_metadata_displayed_key).toString());
                    }
                }
            }
            // We can just draw the point.
            else
            {
                // Calculate the point in pixels.
                const PointWorldPx point_px(projection::get().toPointWorldPx(m_point_coord, controller_zoom));

                // Is the point within the backbuffer rect?
                if(backbuffer_rect_px.contains(point_px.rawPoint()))
                {
                    // Set the pen to use.
                    painter->setPen(getPen());

                    // Draw the point.
                    painter->drawPoint(point_px.rawPoint());

                    // Do we have a meta-data value and should we display it at this zoom?
                    if(controller_zoom >= m_metadata_displayed_zoom_minimum && getMetadata(m_metadata_displayed_key).isNull() == false)
                    {
                        // Draw the text next to the point with an offset.
                        painter->drawText((point_px + PointPx(m_metadata_displayed_alignment_offset_px, -m_metadata_displayed_alignment_offset_px)).rawPoint(), getMetadata(m_metadata_displayed_key).toString());
                    }
                }
            }
        }
    }

    void GeometryPoint::updatePixmap()
    {
        // Do nothing.

        // Emit that we need to redraw to display this change.
        emit requestRedraw();
    }

    QSizeF GeometryPoint::calculateGeometrySizePx(const int& controller_zoom) const
    {
        // Get the object size (default to point - ie: no size).
        QSizeF return_size_px(1.0, 1.0);

        // Do we have a pixmap?
        if(m_pixmap != nullptr && m_pixmap->size().isEmpty() == false)
        {
            // Get the pixmap size.
            return_size_px = m_pixmap->size();

            // Do we have a base zoom set?
            if(m_base_zoom > 0)
            {
                // What is the difference between the base and current zoom.
                const int zoom_difference = m_base_zoom - controller_zoom;

                // Calculate the zoom width/height as a ratio of the zoom difference.
                return_size_px.setHeight(m_base_size_px.height() / std::pow(2.0, zoom_difference));
                return_size_px.setWidth(m_base_size_px.width() / std::pow(2.0, zoom_difference));

                // Do we have a minimum height draw size set?
                if(m_draw_minimum_px.height() > -1.0)
                {
                    // Take the highest height.
                    return_size_px.setHeight(std::max(return_size_px.height(), m_draw_minimum_px.height()));
                }

                // Do we have a minimum width draw size set?
                if(m_draw_minimum_px.width() > -1.0)
                {
                    // Take the highest width.
                    return_size_px.setWidth(std::max(return_size_px.width(), m_draw_minimum_px.width()));
                }

                // Do we have a maximum height draw size set?
                if(m_draw_maximum_px.height() > -1.0)
                {
                    // Take the lowest height.
                    return_size_px.setHeight(std::min(return_size_px.height(), m_draw_maximum_px.height()));
                }

                // Do we have a maximum width draw size set?
                if(m_draw_maximum_px.width() > -1.0)
                {
                    // Take the lowest width.
                    return_size_px.setWidth(std::min(return_size_px.width(), m_draw_maximum_px.width()));
                }
            }
        }

        // Return the size.
        return return_size_px;
    }

}
