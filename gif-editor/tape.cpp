
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
#include <QVector>
#include <QHBoxLayout>


//
// TapePrivate
//

class TapePrivate {
public:
	TapePrivate( Tape * parent )
		:	m_layout( new QHBoxLayout( parent ) )
		,	q( parent )
	{
		m_layout->setMargin( 0 );
	}

	//! Frames.
	QVector< FrameOnTape* > m_frames;
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
Tape::addFrame( const QImage & img )
{
	d->m_frames.append( new FrameOnTape( img, count() + 1, this ) );
	d->m_layout->addWidget( d->m_frames.back() );

	connect( d->m_frames.back(), &FrameOnTape::clicked, this, &Tape::clicked );

	adjustSize();
}

FrameOnTape *
Tape::frame( int idx ) const
{
	if( idx <= count() )
		return d->m_frames.at( idx - 1 );
	else
		return nullptr;
}

void
Tape::removeFrame( int idx )
{
	if( idx <= count() )
	{
		d->m_layout->removeWidget( d->m_frames.at( idx - 1 ) );
		d->m_frames.at( idx - 1 )->deleteLater();
		d->m_frames.removeAt( idx - 1 );

		adjustSize();
	}
}

void
Tape::clear()
{
	for( int i = 1; i <= count(); ++i )
		removeFrame( 1 );
}
