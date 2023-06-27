
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
#include <QThreadPool>
#include <QApplication>
#include <QRunnable>


//
// FramePrivate
//

class FramePrivate {
public:
	FramePrivate( const ImageRef & img, Frame::ResizeMode mode,
		Frame * parent )
		:	m_image( img )
		,	m_mode( mode )
		,	m_dirty( false )
		,	q( parent )
	{
	}

	//! Create thumbnail.
	void createThumbnail( int height );
	//! Frame widget was resized.
	void resized( int height = -1 );

	//! Image reference.
	ImageRef m_image;
	//! Thumbnail.
	QImage m_thumbnail;
	//! Resize mode.
	Frame::ResizeMode m_mode;
	//! Dirty frame. We need to resize the image to actual size before drawing.
	bool m_dirty;
	//! Width.
	int m_width = 0;
	//! Height.
	int m_height = 0;
	//! Desired height.
	int m_desiredHeight = -1;
	//! Parent.
	Frame * q;
}; // class FramePrivate

class ThumbnailCreator final
	:	public QRunnable
{
public:
	ThumbnailCreator( QImage img, int width, int height, int desiredHeight,
		Frame::ResizeMode mode )
		:	m_img( img )
		,	m_width( width )
		,	m_height( height )
		,	m_desiredHeight( desiredHeight )
		,	m_mode( mode )
	{
		setAutoDelete( false );
	}

	const QImage & image() const
	{
		return m_thumbnail;
	}

	void run() override
	{
		if( m_img.width() > m_width || m_img.height() > m_height )
		{
			m_thumbnail = m_img.scaledToHeight(
				m_desiredHeight > 0 ? m_desiredHeight : m_height,
				Qt::SmoothTransformation );
		}
		else
			m_thumbnail = m_img;
	}

private:
	QImage m_img;
	int m_width;
	int m_height;
	int m_desiredHeight;
	Frame::ResizeMode m_mode;
	QImage m_thumbnail;
};

void
FramePrivate::createThumbnail( int height )
{
	m_dirty = false;

	if( !m_image.m_isEmpty )
	{
		m_height = q->height();
		m_width = q->width();
		m_desiredHeight = height;

		if( m_mode == Frame::ResizeMode::FitToHeight )
		{
			ThumbnailCreator c( m_image.m_data.at( m_image.m_pos ).first, q->width(), q->height(),
				height, m_mode );

			QThreadPool::globalInstance()->start( &c );

			while( !QThreadPool::globalInstance()->waitForDone( 5 ) )
				QApplication::processEvents();

			m_thumbnail = c.image();
		}
		else
		{
			if( m_image.m_data.at( m_image.m_pos ).first.width() > q->width() ||
				m_image.m_data.at( m_image.m_pos ).first.height() > q->height() )
			{
				m_thumbnail = m_image.m_data.at( m_image.m_pos ).first
					.scaled( q->width(), q->height(),
						Qt::KeepAspectRatio, Qt::SmoothTransformation );
			}
			else
				m_thumbnail = m_image.m_data.at( m_image.m_pos ).first;
		}
	}
}

void
FramePrivate::resized( int height )
{
	if( m_dirty || height != m_desiredHeight ||
		q->width() != m_width || q->height() != m_height )
	{
		createThumbnail( height );

		q->updateGeometry();

		emit q->resized();
	}
}


//
// Frame
//

Frame::Frame( const ImageRef & img, ResizeMode mode, QWidget * parent, int height )
	:	QWidget( parent )
	,	d( new FramePrivate( img, mode, this ) )
{
	switch( mode )
	{
		case ResizeMode::FitToSize :
			setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
		break;

		case ResizeMode::FitToHeight :
		{
			setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
			d->resized( height );
		}
		break;
	}
}

Frame::~Frame() noexcept
{
}

const ImageRef &
Frame::image() const
{
	return d->m_image;
}

void
Frame::setImagePos( const ImageRef::PosType & pos )
{
	d->m_image.m_pos = pos;
	d->m_desiredHeight = -1;
	d->m_width = 0;
	d->m_height = 0;
}

void
Frame::clearImage()
{
	d->m_image.m_isEmpty = true;
	d->m_thumbnail = QImage();
	d->m_desiredHeight = -1;
	d->m_width = 0;
	d->m_height = 0;
	update();
}

void
Frame::applyImage()
{
	d->m_image.m_isEmpty = false;

	d->resized();

	update();
}

QRect
Frame::thumbnailRect() const
{
	const int x = ( width() - d->m_thumbnail.width() ) / 2;
	const int y = ( height() - d->m_thumbnail.height() ) / 2;

	QRect r = d->m_thumbnail.rect();
	r.moveTopLeft( QPoint( x, y ) );

	return r;
}

QRect
Frame::imageRect() const
{
	if( !d->m_image.m_isEmpty )
	{
		const auto & img = d->m_image.m_data.at( d->m_image.m_pos ).first;

		return QRect( 0, 0, img.width(), img.height() );
	}
	else
		return {};
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
	p.drawImage( thumbnailRect(), d->m_thumbnail, d->m_thumbnail.rect() );
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
