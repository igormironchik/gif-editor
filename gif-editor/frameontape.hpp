
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
#include <QFrame>
#include <QScopedPointer>

// GIF editor include.
#include "frame.hpp"


//
// FrameOnTape
//

class FrameOnTapePrivate;

//! Frame on tape.
class FrameOnTape final
	:	public QFrame
{
	Q_OBJECT

signals:
	//! Clicked.
	void clicked( int idx );
	//! Checked.
	void checked( int idx, bool on );
	//! Check/uncheck till end action activated.
	void checkTillEnd( int idx, bool on );

public:
	FrameOnTape( const ImageRef & img, int counter, int height, QWidget * parent = nullptr );
	~FrameOnTape() noexcept override;

	//! \return Image.
	const ImageRef & image() const;
	//! Set image.
	void setImagePos( const ImageRef::PosType & pos );
	//! Clear image.
	void clearImage();
	//! Apply image.
	void applyImage();

	//! \return Is frame checked.
	bool isChecked() const;
	//! Set checked.
	void setChecked( bool on = true );

	//! \return Counter.
	int counter() const;
	//! Set counter.
	void setCounter( int c );

	//! \return Is this frame current?
	bool isCurrent() const;
	//! Set current flag.
	void setCurrent( bool on = true );

protected:
	void contextMenuEvent( QContextMenuEvent * e ) override;

private:
	Q_DISABLE_COPY( FrameOnTape )

	QScopedPointer< FrameOnTapePrivate > d;
}; // class FrameOnTape

#endif // GIF_EDITOR_FRAMEONTAPE_HPP_INCLUDED
