
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

#ifndef GIF_EDITOR_FRAME_HPP_INCLUDED
#define GIF_EDITOR_FRAME_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>


//
// Frame
//

class FramePrivate;

//! This is just an image with frame that fit the given size or height.
class Frame final
	:	public QWidget
{
	Q_OBJECT

signals:
	//! Clicked.
	void clicked();
	//! Resized.
	void resized();

public:
	//! Resize mode.
	enum class ResizeMode {
		//! Fit to size.
		FitToSize,
		//! Fit to height.
		FitToHeight
	}; // enum class ResizeMode

	Frame( const QImage & img, ResizeMode mode, QWidget * parent = nullptr );
	~Frame() noexcept override;

	//! \return Image.
	const QImage & image() const;
	//! Set image.
	void setImage( const QImage & img );
	//! \return Thumbnail image rect.
	QRect imageRect() const;

	QSize sizeHint() const override;

private slots:
	//! Context menu requested.
	void contextMenuRequested( const QPoint & pos );

protected:
	void paintEvent( QPaintEvent * ) override;
	void resizeEvent( QResizeEvent * e ) override;
	void mouseReleaseEvent( QMouseEvent * e ) override;

private:
	Q_DISABLE_COPY( Frame )

	QScopedPointer< FramePrivate > d;
}; // class Frame

#endif // GIF_EDITOR_FRAME_HPP_INCLUDED
