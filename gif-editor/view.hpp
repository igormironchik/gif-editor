
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

#ifndef GIF_EDITOR_VIEW_HPP_INCLUDED
#define GIF_EDITOR_VIEW_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>

// Magick++ include.
#include <Magick++.h>


class Tape;
class Frame;


//
// View
//

class ViewPrivate;

//! View with current frame and tape with frames.
class View final
	:	public QWidget
{
	Q_OBJECT

public:
	explicit View( const std::vector< QPair< QImage, size_t > > & data, QWidget * parent = nullptr );
	~View() noexcept override;

	//! \return Tape.
	Tape * tape() const;
	//! \return Current frame.
	Frame * currentFrame() const;

	//! \return Crop rectangle.
	QRect cropRect() const;

public slots:
	//! Start crop.
	void startCrop();
	//! Stop crop.
	void stopCrop();
	//! Scroll to frame.
	void scrollTo( int idx );

private slots:
	//! Frame selected.
	void frameSelected( int idx );

private:
	Q_DISABLE_COPY( View )

	QScopedPointer< ViewPrivate > d;
}; // class View

#endif // GIF_EDITOR_VIEW_HPP_INCLUDED
