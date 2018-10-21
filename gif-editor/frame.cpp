
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


//
// FramePrivate
//

class FramePrivate {
public:
	FramePrivate( const QImage & img, Frame::ResizeMode mode,
		Frame * parent )
		:	m_image( img )
		,	m_mode( mode )
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
	//! Parent.
	Frame * q;
}; // class FramePrivate

void
FramePrivate::createThumbnail()
{
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
}

void
FramePrivate::resized()
{
	createThumbnail();

	q->updateGeometry();

	q->update();
}


//
// Frame
//

Frame::Frame( const QImage & img, ResizeMode mode, QWidget * parent )
	:	QWidget( parent )
	,	d( new FramePrivate( img, mode, this ) )
{
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

QSize
Frame::sizeHint() const
{
	return d->m_thumbnail.size();
}

void
Frame::paintEvent( QPaintEvent * )
{
	const int x = ( width() - d->m_thumbnail.width() ) / 2;
	const int y = ( height() - d->m_thumbnail.height() ) / 2;

	QPainter p( this );
	QRect r = d->m_thumbnail.rect();
	r.moveTopLeft( QPoint( x, y ) );
	p.drawImage( r, d->m_thumbnail, d->m_thumbnail.rect() );
}

void
Frame::resizeEvent( QResizeEvent * e )
{
	if( d->m_mode == ResizeMode::FitToSize ||
		( d->m_mode == ResizeMode::FitToHeight && e->size().height() != d->m_thumbnail.height() ) )
			d->resized();

	e->accept();
}
