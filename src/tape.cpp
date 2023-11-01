
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
#include "tape.hpp"
#include "frameontape.hpp"

// Qt include.
#include <QList>
#include <QHBoxLayout>
#include <QApplication>

// C++ include.
#include <utility>


//
// TapePrivate
//

class TapePrivate {
public:
	TapePrivate( Tape * parent )
		:	m_currentFrame( nullptr )
		,	m_layout( new QHBoxLayout( parent ) )
		,	q( parent )
	{
		m_layout->setContentsMargins( q->spacing(), q->spacing(),
			q->spacing(), q->spacing() );
		m_layout->setSpacing( q->spacing() );
	}

	void clearImages()
	{
		for( auto & f : std::as_const( m_frames ) )
			f->clearImage();
	}

	//! Frames.
	QList< FrameOnTape* > m_frames;
	//! Current frame.
	FrameOnTape * m_currentFrame;
	//! Layout.
	QHBoxLayout * m_layout;
	//! Parent.
	Tape * q;
}; // class TapePrivate


//
// Tape
//

Tape::Tape( QWidget * parent )
	:	QWidget( parent )
	,	d( new TapePrivate( this ) )
{
}

Tape::~Tape() noexcept
{
}

int
Tape::count() const
{
	return d->m_frames.count();
}

void
Tape::addFrame( const ImageRef & img )
{
	d->m_frames.append( new FrameOnTape( img, count() + 1,
		height() - d->m_layout->contentsMargins().bottom() - d->m_layout->contentsMargins().top(),
		this ) );
	connect( d->m_frames.back(), &FrameOnTape::checkTillEnd,
		this, &Tape::checkTillEnd );
	d->m_layout->addWidget( d->m_frames.back() );

	QApplication::processEvents();

	connect( d->m_frames.back(), &FrameOnTape::clicked, this,
		[this] ( int idx )
		{
			if( this->currentFrame() )
				this->currentFrame()->setCurrent( false );

			this->d->m_currentFrame = this->frame( idx );

			this->d->m_currentFrame->setCurrent( true );

			emit this->currentFrameChanged( idx );

			emit this->clicked( idx );
		} );

	connect( d->m_frames.back(), &FrameOnTape::checked,
		this, &Tape::checkStateChanged );

	adjustSize();
}

FrameOnTape *
Tape::frame( int idx ) const
{
	if( idx >= 1 && idx <= count() )
		return d->m_frames.at( idx - 1 );
	else
		return nullptr;
}

FrameOnTape *
Tape::currentFrame() const
{
	return d->m_currentFrame;
}

void
Tape::setCurrentFrame( int idx )
{
	if( idx >= 1 && idx <= count() )
	{
		if( d->m_currentFrame )
			d->m_currentFrame->setCurrent( false );

		d->m_currentFrame = frame( idx );
		d->m_currentFrame->setCurrent( true );

		emit currentFrameChanged( idx );
	}
	else
		d->m_currentFrame = nullptr;
}

void
Tape::removeFrame( int idx )
{
	if( idx <= count() )
	{
		d->m_layout->removeWidget( d->m_frames.at( idx - 1 ) );
		d->m_frames.at( idx - 1 )->deleteLater();

		if( d->m_frames.at( idx - 1 ) == d->m_currentFrame )
		{
			d->m_currentFrame = nullptr;

			if( idx > 1 )
			{
				d->m_currentFrame = d->m_frames.at( idx - 2 );
				d->m_currentFrame->setCurrent( true );

				emit currentFrameChanged( idx - 1 );
			}
			else if( idx < count() )
			{
				d->m_currentFrame = d->m_frames.at( idx );
				d->m_currentFrame->setCurrent( true );

				emit currentFrameChanged( idx + 1 );
			}
			else
			{
				d->m_currentFrame = nullptr;

				emit currentFrameChanged( 0 );
			}
		}

		d->m_frames.removeAt( idx - 1 );

		adjustSize();
	}
}

void
Tape::clear()
{
	const int c = count();

	for( int i = 0; i < c; ++i )
	{
		d->m_layout->removeWidget( d->m_frames.at( i ) );
		d->m_frames.at( i )->deleteLater();
		QApplication::processEvents();
	}

	d->m_frames.clear();

	d->m_currentFrame = nullptr;

	emit currentFrameChanged( 0 );
}

void
Tape::removeUnchecked()
{
	const int c = count();
	int removed = 0;

	for( int i = 1; i <= c; ++i )
	{
		if( !frame( i - removed )->isChecked() )
		{
			removeFrame( i - removed );

			++removed;

			QApplication::processEvents();
		}
		else
			frame( i - removed )->setCounter( i - removed );
	}
}

void
Tape::checkTillEnd( int idx, bool on )
{
	for( int i = idx; i <= count(); ++i )
		frame( i )->setChecked( on );
}

int
Tape::xOfFrame( int idx ) const
{
	if( idx >= 1 && idx <= count() )
		return d->m_layout->contentsMargins().left() + ( idx - 1 ) * d->m_currentFrame->width() +
			( idx - 1 ) * d->m_layout->spacing();
	else
		return -1;
}

int
Tape::spacing() const
{
	return 5;
}
