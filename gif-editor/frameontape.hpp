
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

#ifndef GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED
#define GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


//
// FrameOnTape
//

class FrameOnTapePrivate;

//! Frame on tape.
class FrameOnTape final
	:	public QWidget
{
	Q_OBJECT

signals:
	//! Clicked.
	void clicked();
	//! Checked.
	void checked( bool on );

public:
	FrameOnTape( const QImage & img, quint64 counter, QWidget * parent = nullptr );
	~FrameOnTape() noexcept override;

	//! \return Image.
	const QImage & image() const;
	//! Set image.
	void setImage( const QImage & img );

	//! \return Is frame checked.
	bool isChecked() const;

	//! \return Counter.
	quint64 counter() const;
	//! Set counter.
	void setCounter( quint64 c );

private:
	Q_DISABLE_COPY( FrameOnTape )

	QScopedPointer< FrameOnTapePrivate > d;
}; // class FrameOnTape

#endif // GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED
