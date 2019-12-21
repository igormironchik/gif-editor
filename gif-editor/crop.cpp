
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
	//! Override cursor.
	void overrideCursor( const QPoint & pos );
	//! Resize crop.
	void resize( const QPoint & pos ) ;
	//! \return Cropped rect.
	QRectF cropped( const QRect & full ) const;
	//! Restore overriden cursor.
	void restoreOverridenCursor();
	//! \return Is handles should be outside selected rect.
	bool isHandleOutside() const
	{
		return ( qAbs( m_selected.width() ) / 3 < c_handleSize + 1 ||
			qAbs( m_selected.height() ) / 3 < c_handleSize + 1 );
	}
	//! \return Top-left handle rect.
	QRect topLeftHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? c_handleSize : 0 ) ),
				qRound( m_selected.y() - ( m_selected.height() > 0 ? c_handleSize : 0 ) ),
				c_handleSize, c_handleSize ) :
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				qRound( m_selected.y() - ( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				c_handleSize, c_handleSize ) );
	}
	//! \return Top-right handle rect.
	QRect topRightHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() + m_selected.width() - 1 -
					( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				qRound( m_selected.y() - ( m_selected.height() > 0 ? c_handleSize : 0 ) ),
				c_handleSize, c_handleSize ) :
			QRect( qRound( m_selected.x() + m_selected.width() -
					( m_selected.width() > 0 ? c_handleSize : 0 ) - 1 ),
				qRound( m_selected.y() - ( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				c_handleSize, c_handleSize ) );
	}
	//! \return Bottom-right handle rect.
	QRect bottomRightHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() + m_selected.width() - 1 -
					( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				qRound( m_selected.y() + m_selected.height() -
					( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				c_handleSize, c_handleSize ) :
			QRect( qRound( m_selected.x() + m_selected.width() -
					( m_selected.width() > 0 ? c_handleSize : 0 ) - 1 ),
				qRound( m_selected.y() + m_selected.height() -
					( m_selected.height() > 0 ? c_handleSize : 0 )  - 1 ),
				c_handleSize, c_handleSize ) );
	}
	//! \return Bottom-left handle rect.
	QRect bottomLeftHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? c_handleSize : 0 ) ),
				qRound( m_selected.y() + m_selected.height() - 1 -
					( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				c_handleSize, c_handleSize ) :
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				qRound( m_selected.y() + m_selected.height() -
					( m_selected.height() > 0 ? c_handleSize : 0 ) - 1 ),
				c_handleSize, c_handleSize ) );
	}
	//! \return Y handle width.
	int yHandleWidth() const
	{
		const int w = qRound( m_selected.width() - 1 );

		return ( isHandleOutside() ? w :
			w - 2 * c_handleSize - ( w - 2 * c_handleSize ) / 3 );
	}
	//! \return X handle height.
	int xHandleHeight() const
	{
		const int h = qRound( m_selected.height() - 1 );

		return ( isHandleOutside() ? h :
			h - 2 * c_handleSize - ( h - 2 * c_handleSize ) / 3 );
	}
	//! \return Y handle x position.
	int yHandleXPos() const
	{
		return qRound( m_selected.x() + ( m_selected.width() - yHandleWidth() ) / 2.0 );
	}
	//! \return X handle y position.
	int xHandleYPos() const
	{
		return qRound( m_selected.y() + ( m_selected.height() - xHandleHeight() ) / 2.0 );
	}
	//! \return Top handle rect.
	QRect topHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( yHandleXPos(), qRound( m_selected.y() - ( m_selected.height() > 0 ? c_handleSize : 0 ) ),
				yHandleWidth(), c_handleSize ) :
			QRect( yHandleXPos(), qRound( m_selected.y() - ( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				yHandleWidth(), c_handleSize ) );
	}
	//! \return Bottom handle rect.
	QRect bottomHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( yHandleXPos(), qRound( m_selected.y() + m_selected.height() - 1 -
					( m_selected.height() > 0 ? 0 : c_handleSize ) ),
				yHandleWidth(), c_handleSize ) :
			QRect( yHandleXPos(), qRound( m_selected.y() + m_selected.height() - 1 -
					( m_selected.height() > 0 ? c_handleSize : 0 ) ),
				yHandleWidth(), c_handleSize ) );
	}
	//! \return Left handle rect.
	QRect leftHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? c_handleSize : 0 ) ),
				xHandleYPos(), c_handleSize, xHandleHeight() ) :
			QRect( qRound( m_selected.x() - ( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				xHandleYPos(), c_handleSize, xHandleHeight() ) );
	}
	//! \return Right handle rect.
	QRect rightHandleRect() const
	{
		return ( isHandleOutside() ?
			QRect( qRound( m_selected.x() + m_selected.width() - 1 -
					( m_selected.width() > 0 ? 0 : c_handleSize ) ),
				xHandleYPos(), c_handleSize, xHandleHeight() ) :
			QRect( qRound( m_selected.x() + m_selected.width() - 1 -
					( m_selected.width() > 0 ? c_handleSize : 0 ) ),
				xHandleYPos(), c_handleSize, xHandleHeight() ) );
	}

	//! Selected rectangle.
	QRectF m_selected;
	//! Available rectangle.
	QRect m_available;
	//! Mouse pos.
	QPoint m_mousePos;
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
	else if( p.x() > m_available.x() + m_available.width() )
		ret.setX( m_available.x() + m_available.width() );

	if( p.y() < m_available.y() )
		ret.setY( m_available.y() );
	else if( p.y() > m_available.y() + m_available.height() )
		ret.setY( m_available.y() + m_available.height() );

	return ret;
}

QPoint
CropFramePrivate::boundLeftTopToAvailable( const QPoint & p ) const
{
	QPoint ret = p;

	if( p.x() < m_available.x() )
		ret.setX( m_available.x() );
	else if( p.x() > m_available.x() + m_available.width() - qRound( m_selected.width() ) - 1 )
		ret.setX( m_available.x() + m_available.width() - qRound( m_selected.width() ) - 1 );

	if( p.y() < m_available.y() )
		ret.setY( m_available.y() );
	else if( p.y() > m_available.y() + m_available.height() - qRound( m_selected.height() ) - 1 )
		ret.setY( m_available.y() + m_available.height() - qRound( m_selected.height() ) - 1 );

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

void
CropFramePrivate::overrideCursor( const QPoint & pos )
{
	if( topLeftHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::TopLeft;
		checkAndOverrideCursor( Qt::SizeFDiagCursor );
	}
	else if( bottomRightHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::BottomRight;
		checkAndOverrideCursor( Qt::SizeFDiagCursor );
	}
	else if( topRightHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::TopRight;
		checkAndOverrideCursor( Qt::SizeBDiagCursor );
	}
	else if( bottomLeftHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::BottomLeft;
		checkAndOverrideCursor( Qt::SizeBDiagCursor );
	}
	else if( topHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::Top;
		checkAndOverrideCursor( Qt::SizeVerCursor );
	}
	else if( bottomHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::Bottom;
		checkAndOverrideCursor( Qt::SizeVerCursor );
	}
	else if( leftHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::Left;
		checkAndOverrideCursor( Qt::SizeHorCursor );
	}
	else if( rightHandleRect().contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::Right;
		checkAndOverrideCursor( Qt::SizeHorCursor );
	}
	else if( m_selected.contains( pos ) )
	{
		m_handle = CropFramePrivate::Handle::Unknown;
		checkAndOverrideCursor( Qt::SizeAllCursor );
	}
	else if( m_cursorOverriden )
	{
		m_cursorOverriden = false;
		m_handle = CropFramePrivate::Handle::Unknown;
		QApplication::restoreOverrideCursor();
	}
}

void
CropFramePrivate::resize( const QPoint & pos )
{
	switch( m_handle )
	{
		case CropFramePrivate::Handle::Unknown :
			m_selected.moveTo( boundLeftTopToAvailable(
				m_selected.topLeft().toPoint() - m_mousePos + pos ) );
		break;

		case CropFramePrivate::Handle::TopLeft :
			m_selected.setTopLeft( boundToAvailable( m_selected.topLeft().toPoint() -
				m_mousePos + pos ) );
		break;

		case CropFramePrivate::Handle::TopRight :
			m_selected.setTopRight( boundToAvailable( m_selected.topRight().toPoint() -
				m_mousePos + pos ) );
		break;

		case CropFramePrivate::Handle::BottomRight :
			m_selected.setBottomRight( boundToAvailable( m_selected.bottomRight().toPoint() -
				m_mousePos + pos ) );
		break;

		case CropFramePrivate::Handle::BottomLeft :
			m_selected.setBottomLeft( boundToAvailable( m_selected.bottomLeft().toPoint() -
				m_mousePos + pos ) );
		break;

		case CropFramePrivate::Handle::Top :
			m_selected.setTop( boundToAvailable( QPoint( qRound( m_selected.left() ),
				qRound( m_selected.top() ) ) - m_mousePos + pos ).y() );
		break;

		case CropFramePrivate::Handle::Bottom :
			m_selected.setBottom( boundToAvailable( QPoint( qRound( m_selected.left() ),
				qRound( m_selected.bottom() ) ) - m_mousePos + pos ).y() );
		break;

		case CropFramePrivate::Handle::Left :
			m_selected.setLeft( boundToAvailable( QPoint( qRound( m_selected.left() ),
				qRound( m_selected.top() ) ) - m_mousePos + pos ).x() );
		break;

		case CropFramePrivate::Handle::Right :
			m_selected.setRight( boundToAvailable( QPoint( qRound( m_selected.right() ),
				qRound( m_selected.top() ) ) - m_mousePos + pos ).x() );
		break;
	}

	m_mousePos = pos;
}

QRectF
CropFramePrivate::cropped( const QRect & full ) const
{
	const auto oldR = m_available;

	const qreal xRatio = static_cast< qreal > ( full.width() ) /
		static_cast< qreal > ( oldR.width() );
	const qreal yRatio = static_cast< qreal > ( full.height() ) /
		static_cast< qreal > ( oldR.height() );

	QRectF r;

	if( !m_nothing )
	{
		const auto x = ( m_selected.x() - oldR.x() ) * xRatio + full.x();
		const auto y = ( m_selected.y() - oldR.y() ) * yRatio + full.y();
		const auto w = m_selected.width() * xRatio;
		const auto h = m_selected.height() * yRatio;

		r.setTopLeft( QPointF( x, y ) );
		r.setSize( QSizeF( w, h ) );
	}

	return r;
}

void
CropFramePrivate::restoreOverridenCursor()
{
	if( m_cursorOverriden )
		QApplication::restoreOverrideCursor();

	if( m_hovered )
		QApplication::restoreOverrideCursor();

	m_cursorOverriden = false;
	m_hovered = false;
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

	d->m_available = parent->imageRect();

	connect( d->m_frame, &Frame::resized,
		this, &CropFrame::frameResized );
}

CropFrame::~CropFrame() noexcept
{
	d->restoreOverridenCursor();
}

QRect
CropFrame::cropRect() const
{
	return d->cropped( d->m_frame->image().rect() ).toRect();
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

	d->restoreOverridenCursor();

	update();
}

void
CropFrame::frameResized()
{
	d->m_selected = d->cropped( d->m_frame->imageRect() );

	setGeometry( QRect( 0, 0, d->m_frame->width(), d->m_frame->height() ) );

	d->m_available = d->m_frame->imageRect();

	update();
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

		if( d->m_available != d->m_selected.toRect() )
		{
			QPainterPath spath;
			spath.addRect( d->m_selected.adjusted( 0, 0, -1, -1 ) );
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

			case CropFramePrivate::Handle::Top :
				p.drawRect( d->topHandleRect() );
			break;

			case CropFramePrivate::Handle::Bottom :
				p.drawRect( d->bottomHandleRect() );
			break;

			case CropFramePrivate::Handle::Left :
				p.drawRect( d->leftHandleRect() );
			break;

			case CropFramePrivate::Handle::Right :
				p.drawRect( d->rightHandleRect() );
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
			d->resize( e->pos() );

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
		d->overrideCursor( e->pos() );

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
