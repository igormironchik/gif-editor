
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
#include "about.hpp"

// Qt include.
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>


//
// About
//

About::About( QWidget * parent )
	:	QWidget( parent )
{
	auto l = new QHBoxLayout( this );
	l->setSpacing( 50 );
	l->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed ) );

	auto p = new QLabel( this );
	p->setPixmap( QPixmap( ":/img/icon_128x128.png" ) );
	l->addWidget( p );

	auto t = new QLabel( this );
	t->setText( tr( "GIF editor.\n\n"
		"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
		"Copyright (c) 2018 Igor Mironchik.\n\n"
		"Licensed under GNU GPL 3.0." ) );
	l->addWidget( t );

	l->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed ) );
}
