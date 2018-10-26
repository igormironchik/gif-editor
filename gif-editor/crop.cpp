
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2018 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// GIF editor include.
#include "crop.hpp"
#include "frame.hpp"

// Qt include.
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>


//! Size of the handle to change geometry of selected region.
static const int c_handleSize = 15;


//
// CropFramePrivate
//

class CropFramePrivate {
public:
	CropFramePrivate( CropFrame * parent, Frame * toObserve )
		:	m_started( false )
		,	m_nothing( true )
		,	m_clicked( false )
		,	m_hovered( false )
		,	m_cursorOverriden( false )
		,	m_handle( Handle::Unknown )
		,	m_frame( toObserve )
		,	q( parent )
	{
	}

	enum class Handle {
		Unknown,
		TopLeft,
		Top,
		TopRight,
		Right,
		BottomRight,
		Bottom,
		BottomLeft,
		Left
	}; // enum class Handle

	//! Bound point to available space.
	QPoint boundToAvailable( const QPoint & p ) const;
	//! Bound left top point to available space.
	QPoint boundLeftTopToAvailable( const QPoint & p ) const;
	//! Check and override cursor if necessary.
	void checkAndOverrideCursor( Qt::CursorShape shape );
	//! \return Is handles should be outside selected rect.
	bool isHandleOutside() const
	{
		return ( m_selected.width() / 3 < c_handleSize ||
			m_selected.height() / 3 < c_handleSize );
	}
	//! \return Top-left handle rect.
	QRect topLeftHandleRect() const
	{
		return ( isHandleOutside() ? QRect( m_selected.x() - c_handleSize,
				m_selected.y() - c_handleSize, c_handleSize, c_handleSize ) :
			QRect( m_selected.x(), m_selected.y(), c_handleSize, c_handleSize ) );
	}
	//! \return Top-right handle rect.
	QRect topRightHandleRect() const
	{
		return ( isHandleOutside() ? QRect( m_selected.x() + m_selected.width() - 1,
				m_selected.y() - c_handleSize, c_handleSize, c_handleSize ) :
			QRect( m_selected.x() + m_selected.width() - c_handleSize - 1,
				m_selected.y(), c_handleSize, c_handleSize ) );
	}
	//! \return Bottom-right handle rect.
	QRect bottomRightHandleRect() const
	{
		return ( isHandleOutside() ? QRect( m_selected.x() + m_selected.width() - 1,
				m_selected.y() + m_selected.height(), c_handleSize, c_handleSize ) :
			QRect( m_selected.x() + m_selected.width() - c_handleSize - 1,
				m_selected.y() + m_selected.height() - c_handleSize - 1,
				c_handleSize, c_handleSize ) );
	}
	//! \return Bottom-left handle rect.
	QRect bottomLeftHandleRect() const
	{
		return ( isHandleOutside() ? QRect( m_selected.x() - c_handleSize,
				m_selected.y() + m_selected.height() - 1, c_handleSize, c_handleSize ) :
			QRect( m_selected.x(), m_selected.y() + m_selected.height() - c_handleSize - 1,
				c_handleSize, c_handleSize ) );
	}

	//! Selected rectangle.
	QRect m_selected;
	//! Available rectangle.
	QRect m_available;
	//! Mouse pos.
	QPoint m_mousePos;
	//! Current handle rect.
	QRect m_currentHandleRect;
	//! Selecting started.
	bool m_started;
	//! Nothing selected yet.
	bool m_nothing;
	//! Clicked.
	bool m_clicked;
	//! Hover entered.
	bool m_hovered;
	//! Cursor overriden.
	bool m_cursorOverriden;
	//! Current handle.
	Handle m_handle;
	//! Frame to observe resize event.
	Frame * m_frame;
	//! Parent.
	CropFrame * q;
}; // class CropFramePrivate

QPoint
CropFramePrivate::boundToAvailable( const QPoint & p ) const
{
	QPoint ret = p;

	if( p.x() < m_available.x() )
		ret.setX( m_available.x() );
	else if( p.x() > m_available.x() + m_available.width() - 1 )
		ret.setX( m_available.x() + m_available.width() - 1 );

	if( p.y() < m_available.y() )
		ret.setY( m_available.y() );
	else if( p.y() > m_available.y() + m_available.height() - 1 )
		ret.setY( m_available.y() + m_available.height() - 1 );

	return ret;
}

QPoint
CropFramePrivate::boundLeftTopToAvailable( const QPoint & p ) const
{
	QPoint ret = p;

	if( p.x() < m_available.x() )
		ret.setX( m_available.x() );
	else if( p.x() > m_available.x() + m_available.width() - m_selected.width() - 1)
		ret.setX( m_available.x() + m_available.width() - m_selected.width() - 1 );

	if( p.y() < m_available.y() )
		ret.setY( m_available.y() );
	else if( p.y() > m_available.y() + m_available.height() - m_selected.height() - 1 )
		ret.setY( m_available.y() + m_available.height() - m_selected.height() - 1 );

	return ret;
}

void
CropFramePrivate::checkAndOverrideCursor( Qt::CursorShape shape )
{
	if( QApplication::overrideCursor() )
	{
		if( *QApplication::overrideCursor() != QCursor( shape ) )
		{
			if( m_cursorOverriden )
				QApplication::restoreOverrideCursor();
			else
				m_cursorOverriden = true;

			QApplication::setOverrideCursor( QCursor( shape ) );
		}
	}
	else
	{
		m_cursorOverriden = true;

		QApplication::setOverrideCursor( QCursor( shape ) );
	}
}


//
// CropFrame
//

CropFrame::CropFrame( Frame * parent )
	:	QWidget( parent )
	,	d( new CropFramePrivate( this, parent ) )
{
	setAutoFillBackground( false );
	setAttribute( Qt::WA_TranslucentBackground, true );
	setMouseTracking( true );

	connect( d->m_frame, &Frame::resized,
		this, &CropFrame::frameResized );
}

CropFrame::~CropFrame() noexcept
{
	if( d->m_hovered )
		QApplication::restoreOverrideCursor();
}

void
CropFrame::setAvailableRect( const QRect & r )
{
	d->m_available = r;
}

const QRect &
CropFrame::selectedRect() const
{
	return d->m_selected;
}

void
CropFrame::start()
{
	d->m_started = true;
	d->m_nothing = true;

	update();
}

void
CropFrame::stop()
{
	d->m_started = false;

	update();
}

void
CropFrame::frameResized()
{
	const auto oldR = d->m_available;

	setGeometry( QRect( 0, 0, d->m_frame->width(), d->m_frame->height() ) );

	d->m_available = d->m_frame->imageRect();

	const auto newR = d->m_available;

	const qreal xRatio = static_cast< qreal > ( newR.width() ) /
		static_cast< qreal > ( oldR.width() );
	const qreal yRatio = static_cast< qreal > ( newR.height() ) /
		static_cast< qreal > ( oldR.height() );

	if( !d->m_nothing )
	{
		const auto x = static_cast< int >( ( d->m_selected.x() - oldR.x() ) * xRatio ) +
			newR.x();
		const auto y = static_cast< int >( ( d->m_selected.y() - oldR.y() ) * yRatio ) +
			newR.y();
		const auto dx = newR.bottomRight().x() - static_cast< int >(
			( oldR.bottomRight().x() - d->m_selected.bottomRight().x() ) * xRatio );
		const auto dy = newR.bottomRight().y() - static_cast< int >(
			( oldR.bottomRight().y() - d->m_selected.bottomRight().y() ) * yRatio );

		d->m_selected.setTopLeft( QPoint( x, y ) );
		d->m_selected.setBottomRight( QPoint( dx, dy ) );

		update();
	}
}

void
CropFrame::paintEvent( QPaintEvent * )
{
	static const QColor dark( 0, 0, 0, 100 );

	QPainter p( this );
	p.setPen( Qt::black );
	p.setBrush( dark );

	if( d->m_started && !d->m_nothing )
	{
		QPainterPath path;
		path.addRect( QRectF( d->m_available ).adjusted( 0, 0, -1, -1 ) );

		if( d->m_available != d->m_selected )
		{
			QPainterPath spath;
			spath.addRect( QRectF( d->m_selected ).adjusted( 0, 0, -1, -1 ) );
			path = path.subtracted( spath );
		}
		else
			p.setBrush( Qt::transparent );

		p.drawPath( path );
	}

	p.setBrush( Qt::transparent );

	if( d->m_started && !d->m_clicked && !d->m_nothing &&
		d->m_handle == CropFramePrivate::Handle::Unknown )
	{
		p.drawRect( d->topLeftHandleRect() );
		p.drawRect( d->topRightHandleRect() );
		p.drawRect( d->bottomRightHandleRect() );
		p.drawRect( d->bottomLeftHandleRect() );
	}
	else if( d->m_started && !d->m_nothing &&
		d->m_handle != CropFramePrivate::Handle::Unknown )
	{
		switch( d->m_handle )
		{
			case CropFramePrivate::Handle::TopLeft :
				p.drawRect( d->topLeftHandleRect() );
			break;

			case CropFramePrivate::Handle::TopRight :
				p.drawRect( d->topRightHandleRect() );
			break;

			case CropFramePrivate::Handle::BottomRight :
				p.drawRect( d->bottomRightHandleRect() );
			break;

			case CropFramePrivate::Handle::BottomLeft :
				p.drawRect( d->bottomLeftHandleRect() );
			break;

			default:
				break;
		}
	}
}

void
CropFrame::mousePressEvent( QMouseEvent * e )
{
	if( e->button() == Qt::LeftButton )
	{
		d->m_clicked = true;

		if( !d->m_cursorOverriden )
			d->m_selected.setTopLeft( d->boundToAvailable( e->pos() ) );
		else
			d->m_mousePos = e->pos();

		update();

		e->accept();
	}
	else
		e->ignore();
}

void
CropFrame::mouseMoveEvent( QMouseEvent * e )
{
	if( d->m_clicked )
	{
		if ( !d->m_cursorOverriden )
		{
			d->m_selected.setBottomRight( d->boundToAvailable( e->pos() ) );

			d->m_nothing = false;
		}
		else
		{
			switch( d->m_handle )
			{
				case CropFramePrivate::Handle::Unknown :
					d->m_selected.moveTo( d->boundLeftTopToAvailable(
						d->m_selected.topLeft() - d->m_mousePos + e->pos() ) );
				break;

				case CropFramePrivate::Handle::TopLeft :
					d->m_selected.setTopLeft( d->boundToAvailable( d->m_selected.topLeft() -
						d->m_mousePos + e->pos() ) );
				break;

				case CropFramePrivate::Handle::TopRight :
					d->m_selected.setTopRight( d->boundToAvailable( d->m_selected.topRight() -
						d->m_mousePos + e->pos() ) );
				break;

				case CropFramePrivate::Handle::BottomRight :
					d->m_selected.setBottomRight( d->boundToAvailable( d->m_selected.bottomRight() -
						d->m_mousePos + e->pos() ) );
				break;

				case CropFramePrivate::Handle::BottomLeft :
					d->m_selected.setBottomLeft( d->boundToAvailable( d->m_selected.bottomLeft() -
						d->m_mousePos + e->pos() ) );
				break;

				default :
					break;
			}

			d->m_mousePos = e->pos();
		}

		update();

		e->accept();
	}
	else if( !d->m_hovered )
	{
		d->m_hovered = true;

		QApplication::setOverrideCursor( QCursor( Qt::CrossCursor ) );
	}
	else if( d->m_hovered && !d->m_nothing )
	{
		const auto contains = d->m_selected.contains( e->pos() );

		if( d->topLeftHandleRect().contains( e->pos() ) )
		{
			d->m_handle = CropFramePrivate::Handle::TopLeft;
			d->checkAndOverrideCursor( Qt::SizeFDiagCursor );
		}
		else if( d->bottomRightHandleRect().contains( e->pos() ) )
		{
			d->m_handle = CropFramePrivate::Handle::BottomRight;
			d->checkAndOverrideCursor( Qt::SizeFDiagCursor );
		}
		else if( d->topRightHandleRect().contains( e->pos() ) )
		{
			d->m_handle = CropFramePrivate::Handle::TopRight;
			d->checkAndOverrideCursor( Qt::SizeBDiagCursor );
		}
		else if( d->bottomLeftHandleRect().contains( e->pos() ) )
		{
			d->m_handle = CropFramePrivate::Handle::BottomLeft;
			d->checkAndOverrideCursor( Qt::SizeBDiagCursor );
		}
		else if( contains )
		{
			d->m_handle = CropFramePrivate::Handle::Unknown;
			d->checkAndOverrideCursor( Qt::SizeAllCursor );
		}
		else if( d->m_cursorOverriden )
		{
			d->m_cursorOverriden = false;
			d->m_handle = CropFramePrivate::Handle::Unknown;
			QApplication::restoreOverrideCursor();
		}

		update();
	}
	else
		e->ignore();
}

void
CropFrame::mouseReleaseEvent( QMouseEvent * e )
{
	d->m_clicked = false;

	if( e->button() == Qt::LeftButton )
	{
		d->m_selected = d->m_selected.normalized();

		update();

		e->accept();
	}
	else
		e->ignore();
}

void
CropFrame::enterEvent( QEvent * e )
{
	if( d->m_started )
	{
		d->m_hovered = true;

		QApplication::setOverrideCursor( QCursor( Qt::CrossCursor ) );

		e->accept();
	}
	else
		e->ignore();
}

void
CropFrame::leaveEvent( QEvent * e )
{
	if( d->m_started )
	{
		d->m_hovered = false;

		QApplication::restoreOverrideCursor();

		e->accept();
	}
	else
		e->ignore();
}
