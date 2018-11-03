
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

#ifndef GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED
#define GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


//
// BusyIndicator
//

class BusyIndicatorPrivate;

/*!
	BusyIndicator is a widget that shows activity of
	the application, more precisely busy of the application.
	This widget is a ring that spinning.
*/
class BusyIndicator
	:	public QWidget
{
	Q_OBJECT

	/*!
		\property running

		This property holds whether the busy indicator is currently
		indicating activity.

		\note The indicator is only visible when this property
		is set to true.

		The default value is true.
	*/
	Q_PROPERTY( bool running READ isRunning WRITE setRunning )
	/*!
		\property color

		\brief color used to paint indicator

		By default this color is QPalette::Highlight.
	*/
	Q_PROPERTY( QColor color READ color WRITE setColor )
	/*!
		\property radius

		\brief radius of the busy indicator.

		By default, this property is 10.
	*/
	Q_PROPERTY( int radius READ radius WRITE setRadius )

public:
	BusyIndicator( QWidget * parent = nullptr );
	~BusyIndicator() noexcept override;

	//! \return Is busy indicator running?
	bool isRunning() const;
	//! Set busy indicator running property.
	void setRunning( bool on );

	//! \return Color used to paint indicator.
	const QColor & color() const;
	//! Set color used to paint indicator.
	void setColor( const QColor & c );

	//! \return Radius.
	int radius() const;
	//! Set radius.
	void setRadius( int r );

	QSize minimumSizeHint() const override;
	QSize sizeHint() const override;

protected:
	void paintEvent( QPaintEvent * ) override;

private slots:
	void _q_update( const QVariant & );

private:
	friend class BusyIndicatorPrivate;

	Q_DISABLE_COPY( BusyIndicator )

	QScopedPointer< BusyIndicatorPrivate > d;
}; // class BusyIndicator

#endif // GIF_EDITOR_BUSYINDICATOR_HPP_INCLUDED
