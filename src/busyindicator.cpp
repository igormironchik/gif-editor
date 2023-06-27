
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
#include "busyindicator.hpp"

// Qt include.
#include <QPainter>
#include <QVariantAnimation>
#include <QPainterPath>


//
// BusyIndicatorPrivate
//

class BusyIndicatorPrivate {
public:
	BusyIndicatorPrivate( BusyIndicator * parent )
		:	outerRadius( 10 )
		,	innerRadius( static_cast< int > ( outerRadius * 0.6 ) )
		,	size( outerRadius * 2, outerRadius * 2 )
		,	running( true )
		,	animation( nullptr )
		,	q( parent )
	{
	}

	void init();

	int outerRadius;
	int innerRadius;
	QSize size;
	QColor color;
	bool running;
	QVariantAnimation * animation;
	BusyIndicator * q;
}; // class BusyIndicatorPrivate

void
BusyIndicatorPrivate::init()
{
	animation = new QVariantAnimation( q );
	animation->setStartValue( 0.0 );
	animation->setEndValue( 359.0 );
	animation->setDuration( 1000 );
	animation->setLoopCount( -1 );

	QObject::connect( animation, &QVariantAnimation::valueChanged,
		q, &BusyIndicator::_q_update );

	color = q->palette().color( QPalette::Highlight );

	animation->start();
}


//
// BusyIndicator
//

BusyIndicator::BusyIndicator( QWidget * parent )
	:	QWidget( parent )
	,	d( new BusyIndicatorPrivate( this ) )
{
	d->init();
}

BusyIndicator::~BusyIndicator() noexcept
{
	d->animation->stop();
}

bool
BusyIndicator::isRunning() const
{
	return d->running;
}

void
BusyIndicator::setRunning( bool on )
{
	if( on != d->running )
	{
		d->running = on;

		if( d->running )
		{
			show();
			d->animation->start();
		}
		else
		{
			hide();
			d->animation->stop();
		}
	}
}

const QColor &
BusyIndicator::color() const
{
	return d->color;
}

void
BusyIndicator::setColor( const QColor & c )
{
	if( d->color != c )
	{
		d->color = c;
		update();
	}
}

int
BusyIndicator::radius() const
{
	return d->outerRadius;
}

void
BusyIndicator::setRadius( int r )
{
	if( d->outerRadius != r )
	{
		d->outerRadius = r;
		d->innerRadius = static_cast< int > ( d->outerRadius * 0.6 );
		d->size = QSize( d->outerRadius * 2, d->outerRadius * 2 );

		updateGeometry();
	}
}

QSize
BusyIndicator::minimumSizeHint() const
{
	return d->size;
}

QSize
BusyIndicator::sizeHint() const
{
	return d->size;
}

void
BusyIndicator::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	p.setRenderHint( QPainter::Antialiasing );
	p.translate( width() / 2, height() / 2 );

	QPainterPath path;
	path.setFillRule( Qt::OddEvenFill );
	path.addEllipse( - d->outerRadius, - d->outerRadius,
		d->outerRadius * 2, d->outerRadius * 2 );
	path.addEllipse( - d->innerRadius, - d->innerRadius,
		d->innerRadius * 2, d->innerRadius * 2 );

	p.setPen( Qt::NoPen );

	QConicalGradient gradient( 0, 0, - d->animation->currentValue().toReal() );
	gradient.setColorAt( 0.0, Qt::transparent );
	gradient.setColorAt( 0.05, d->color );
	gradient.setColorAt( 1.0, Qt::transparent );

	p.setBrush( gradient );

	p.drawPath( path );
}

void
BusyIndicator::_q_update( const QVariant & )
{
	update();
}
