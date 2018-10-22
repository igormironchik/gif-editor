
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
#include "frameontape.hpp"
#include "frame.hpp"

// Qt include.
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>


//
// FrameOnTapePrivate
//

class FrameOnTapePrivate {
public:
	FrameOnTapePrivate( const QImage & img, int counter, FrameOnTape * parent )
		:	m_counter( counter )
		,	m_frame( new Frame( img, Frame::ResizeMode::FitToHeight, parent ) )
		,	m_label( new QLabel( parent ) )
		,	m_checkBox( new QCheckBox( parent ) )
		,	q( parent )
	{
		m_checkBox->setChecked( true );

		m_label->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
		m_label->setText( FrameOnTape::tr( "#%1" ).arg( m_counter ) );
	}

	//! Counter.
	int m_counter;
	//! Frame.
	Frame * m_frame;
	//! Counter label.
	QLabel * m_label;
	//! Check box.
	QCheckBox * m_checkBox;
	//! Parent.
	FrameOnTape * q;
}; // class FrameOnTapePrivate


//
// FrameOnTape
//

FrameOnTape::FrameOnTape( const QImage & img, int counter, QWidget * parent )
	:	QFrame( parent )
	,	d( new FrameOnTapePrivate( img, counter, this ) )
{
	auto vlayout = new QVBoxLayout( this );
	vlayout->setMargin( 0 );
	vlayout->addWidget( d->m_frame );

	auto hlayout = new QHBoxLayout;
	hlayout->setMargin( 0 );
	hlayout->addWidget( d->m_checkBox );
	hlayout->addWidget( d->m_label );

	vlayout->addLayout( hlayout );

	setFrameStyle( QFrame::Box | QFrame::Raised );

	connect( d->m_checkBox, &QCheckBox::stateChanged,
		[this] ( int state ) { emit this->checked( state != 0 ); } );
	connect( d->m_frame, &Frame::clicked,
		[this] () { emit this->clicked( this->d->m_counter ); } );
}

FrameOnTape::~FrameOnTape() noexcept
{
}

const QImage &
FrameOnTape::image() const
{
	return d->m_frame->image();
}

void
FrameOnTape::setImage( const QImage & img )
{
	d->m_frame->setImage( img );
}

bool
FrameOnTape::isChecked() const
{
	return d->m_checkBox->isChecked();
}

int
FrameOnTape::counter() const
{
	return d->m_counter;
}

void
FrameOnTape::setCounter( int c )
{
	d->m_counter = c;

	d->m_label->setText( tr( "#%1" ).arg( c ) );
}
