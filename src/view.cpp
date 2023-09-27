
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
#include "view.hpp"
#include "tape.hpp"
#include "frame.hpp"
#include "frameontape.hpp"
#include "crop.hpp"

// Qt include.
#include <QVBoxLayout>
#include <QScrollArea>
#include <QApplication>
#include <QScrollBar>


//
// ScrollArea
//

class ScrollArea
	:	public QScrollArea
{
public:
	ScrollArea( QWidget * parent )
		:	QScrollArea( parent )
	{
	}

	~ScrollArea() override = default;

	void setZeroContentMargins()
	{
		QScrollArea::setContentsMargins( 0, 0, 0, 0 );
	}

	void scrollContentsTo( int x )
	{
		horizontalScrollBar()->setValue( x );
	}
};


//
// ViewPrivate
//

class ViewPrivate {
public:
	ViewPrivate( const QGifLib::Gif & data, View * parent )
		:	m_tape( nullptr )
		,	m_currentFrame( new Frame( { data, 0, true }, Frame::ResizeMode::FitToSize, parent ) )
		,	m_crop( nullptr )
		,	m_scroll( nullptr )
		,	q( parent )
	{
	}

	//! Tape.
	Tape * m_tape;
	//! Current frame.
	Frame * m_currentFrame;
	//! Crop.
	CropFrame * m_crop;
	//! Scroll area for tape.
	ScrollArea * m_scroll;
	//! Parent.
	View * q;
}; // class ViewPrivate


//
// View
//

View::View( const QGifLib::Gif & data, QWidget * parent )
	:	QWidget( parent )
	,	d( new ViewPrivate( data, this ) )
{
	QVBoxLayout * layout = new QVBoxLayout( this );
	layout->setContentsMargins( 0, 0, 0, 0 );
	layout->addWidget( d->m_currentFrame );

	d->m_scroll = new ScrollArea( this );
	d->m_scroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	d->m_scroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	d->m_scroll->setMinimumHeight( 150 );
	d->m_scroll->setMaximumHeight( 150 );
	d->m_scroll->setWidgetResizable( true );
	d->m_scroll->setZeroContentMargins();

	d->m_tape = new Tape( d->m_scroll );
	d->m_scroll->setWidget( d->m_tape );

	layout->addWidget( d->m_scroll );
	d->m_scroll->setFixedHeight( 150 );

	connect( d->m_tape, &Tape::currentFrameChanged,
		this, &View::frameSelected );
}

View::~View() noexcept
{
}

Tape *
View::tape() const
{
	return d->m_tape;
}

Frame *
View::currentFrame() const
{
	return d->m_currentFrame;
}

QRect
View::cropRect() const
{
	if( d->m_crop )
		return d->m_crop->cropRect();
	else
		return QRect();
}

void
View::startCrop()
{
	if( !d->m_crop )
	{
		d->m_crop = new CropFrame( d->m_currentFrame );
		d->m_crop->setGeometry( QRect( 0, 0,
			d->m_currentFrame->width(), d->m_currentFrame->height() ) );
		d->m_crop->show();
		d->m_crop->raise();
		d->m_crop->start();
	}
}

void
View::stopCrop()
{
	if( d->m_crop )
	{
		d->m_crop->stop();
		d->m_crop->deleteLater();
		d->m_crop = nullptr;
	}
}

void
View::frameSelected( int idx )
{
	if( idx >= 1 && idx <= d->m_tape->count() )
	{
		d->m_currentFrame->setImagePos( idx - 1 );
		d->m_currentFrame->applyImage();
	}
	else
		d->m_currentFrame->clearImage();
}

void
View::scrollTo( int idx )
{
	const auto x = d->m_scroll->horizontalScrollBar()->sliderPosition();
	const auto viewWidth = d->m_scroll->viewport()->width();
	const auto frameX = d->m_tape->xOfFrame( idx );
	const auto frameWidth = d->m_tape->currentFrame()->width();
	const auto spacing = d->m_tape->spacing();

	d->m_scroll->scrollContentsTo( ( frameX + frameWidth + spacing ) - viewWidth );
}
