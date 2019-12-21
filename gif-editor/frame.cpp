
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
#include "frame.hpp"

// Qt include.
#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>


//
// FramePrivate
//

class FramePrivate {
public:
	FramePrivate( const QImage & img, Frame::ResizeMode mode,
		Frame * parent )
		:	m_image( img )
		,	m_mode( mode )
		,	m_dirty( false )
		,	q( parent )
	{
	}

	//! Create thumbnail.
	void createThumbnail();
	//! Frame widget was resized.
	void resized();

	//! Image.
	QImage m_image;
	//! Thumbnail.
	QImage m_thumbnail;
	//! Resize mode.
	Frame::ResizeMode m_mode;
	//! Dirty frame. We need to resize the image to actual size before drawing.
	bool m_dirty;
	//! Parent.
	Frame * q;
}; // class FramePrivate

void
FramePrivate::createThumbnail()
{
	m_dirty = false;

	if( m_image.width() > q->width() || m_image.height() > q->height() )
	{
		switch( m_mode )
		{
			case Frame::ResizeMode::FitToSize :
				m_thumbnail = m_image.scaled( q->width(), q->height(),
					Qt::KeepAspectRatio, Qt::SmoothTransformation );
			break;

			case Frame::ResizeMode::FitToHeight :
				m_thumbnail = m_image.scaledToHeight( q->height(),
					Qt::SmoothTransformation );
			break;
		}
	}
	else
		m_thumbnail = m_image;
}

void
FramePrivate::resized()
{
	createThumbnail();

	q->updateGeometry();

	emit q->resized();
}


//
// Frame
//

Frame::Frame( const QImage & img, ResizeMode mode, QWidget * parent )
	:	QWidget( parent )
	,	d( new FramePrivate( img, mode, this ) )
{
	switch( mode )
	{
		case ResizeMode::FitToSize :
			setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		break;

		case ResizeMode::FitToHeight :
			setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
		break;
	}

	setContextMenuPolicy( Qt::CustomContextMenu );

	connect( this, &QWidget::customContextMenuRequested,
		this, &Frame::contextMenuRequested );
}

Frame::~Frame() noexcept
{
}

const QImage &
Frame::image() const
{
	return d->m_image;
}

void
Frame::setImage( const QImage & img )
{
	d->m_image = img;

	d->resized();
}

QRect
Frame::imageRect() const
{
	const int x = ( width() - d->m_thumbnail.width() ) / 2;
	const int y = ( height() - d->m_thumbnail.height() ) / 2;

	QRect r = d->m_thumbnail.rect();
	r.moveTopLeft( QPoint( x, y ) );

	return r;
}

QSize
Frame::sizeHint() const
{
	return ( d->m_thumbnail.isNull() ? QSize( 10, 10 ) : d->m_thumbnail.size() );
}

void
Frame::paintEvent( QPaintEvent * )
{
	if( d->m_dirty )
		d->resized();

	QPainter p( this );
	p.drawImage( imageRect(), d->m_thumbnail, d->m_thumbnail.rect() );
}

void
Frame::resizeEvent( QResizeEvent * e )
{
	if( d->m_mode == ResizeMode::FitToSize ||
		( d->m_mode == ResizeMode::FitToHeight && e->size().height() != d->m_thumbnail.height() ) )
			d->m_dirty = true;

	e->accept();
}

void
Frame::mouseReleaseEvent( QMouseEvent * e )
{
	if( e->button() == Qt::LeftButton )
	{
		emit clicked();

		e->accept();
	}
	else
		e->ignore();
}

void
Frame::contextMenuRequested( const QPoint & pos )
{
	QMenu menu( this );

	menu.addAction( QIcon( QStringLiteral( ":/img/document-save-as.png" ) ),
		tr( "Save Current Frame" ),
		[this] ()
		{
			auto fileName = QFileDialog::getSaveFileName( this,
				tr( "Choose file to save to..." ), QString(), tr( "PNG (*.png)" ) );

			if( !fileName.isEmpty() )
			{
				if( !fileName.endsWith( QStringLiteral( ".png" ), Qt::CaseInsensitive ) )
					fileName.append( QStringLiteral( ".png" ) );

				this->d->m_image.save( fileName );
			}
		} );

	menu.exec( mapToGlobal( pos ) );
}
