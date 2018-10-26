
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

#ifndef GIF_EDITOR_CROP_HPP_INCLUDED
#define GIF_EDITOR_CROP_HPP_INCLUDED

// Qt include.
#include <QWidget>
#include <QScopedPointer>

class Frame;


//
// CropFrame
//

class CropFramePrivate;

//! Crop frame.
class CropFrame final
	:	public QWidget
{
	Q_OBJECT

public:
	CropFrame( Frame * parent = nullptr );
	~CropFrame() noexcept override;

	//! Set available rectangle.
	void setAvailableRect( const QRect & r );
	//! \return Selected rectangle.
	const QRect & selectedRect() const;

public slots:
	//! Start.
	void start();
	//! Stop.
	void stop();

private slots:
	//! Frame resized.
	void frameResized();

protected:
	void paintEvent( QPaintEvent * ) override;
	void mousePressEvent( QMouseEvent * e ) override;
	void mouseMoveEvent( QMouseEvent * e ) override;
	void mouseReleaseEvent( QMouseEvent * e ) override;
	void enterEvent( QEvent * e ) override;
	void leaveEvent( QEvent * e ) override;

private:
	Q_DISABLE_COPY( CropFrame )

	QScopedPointer< CropFramePrivate > d;
}; // class CropFrame

#endif // GIF_EDITOR_CROP_HPP_INCLUDED
