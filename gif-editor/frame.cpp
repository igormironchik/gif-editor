
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


//
// convert
//

QImage
convert( const Magick::Image & img )
{
    QImage qimg( static_cast< int > ( img.columns() ),
		static_cast< int > ( img.rows() ), QImage::Format_RGB888 );
    const Magick::PixelPacket * pixels;
    Magick::ColorRGB rgb;

    for( int y = 0; y < qimg.height(); ++y)
	{
        pixels = img.getConstPixels( 0, y, static_cast< std::size_t > ( qimg.width() ), 1 );

        for( int x = 0; x < qimg.width(); ++x )
		{
            rgb = ( *( pixels + x ) );

            qimg.setPixel( x, y, QColor( static_cast< int> ( 255 * rgb.red() ),
				static_cast< int > ( 255 * rgb.green() ),
				static_cast< int > ( 255 * rgb.blue() ) ).rgb());
        }
    }

	return qimg;
}


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
	//! Parent.
	Frame * q;
}; // class FramePrivate

void
FramePrivate::createThumbnail( int height )
{
	m_dirty = false;

	if( !m_image.m_isEmpty )
	{
		if( m_image.m_data.at( m_image.m_pos ).columns() > (ImageRef::PosType) q->width() ||
			m_image.m_data.at( m_image.m_pos ).rows() > (ImageRef::PosType) q->height() )
		{
			QImage img = convert( m_image.m_data.at( m_image.m_pos ) );

			switch( m_mode )
			{
				case Frame::ResizeMode::FitToSize :
					m_thumbnail = img.scaled( q->width(), q->height(),
						Qt::KeepAspectRatio, Qt::SmoothTransformation );
				break;

				case Frame::ResizeMode::FitToHeight :
					m_thumbnail = img.scaledToHeight( height > 0 ? height : q->height(),
						Qt::SmoothTransformation );
				break;
			}
		}
		else
			m_thumbnail = convert( m_image.m_data.at( m_image.m_pos ) );
	}
}

void
FramePrivate::resized( int height )
{
	createThumbnail( height );

	q->updateGeometry();

	emit q->resized();
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

	d->resized();

	update();
}

void
Frame::clearImage()
{
	d->m_image.m_isEmpty = true;
	d->m_thumbnail = QImage();
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
		const auto & img = d->m_image.m_data.at( d->m_image.m_pos );

		return QRect( 0, 0, (int) img.columns(), (int) img.rows() );
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
