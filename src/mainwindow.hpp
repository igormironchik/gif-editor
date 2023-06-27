
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

#ifndef GIF_EDITOR_MAINWINDOW_HPP_INCLUDED
#define GIF_EDITOR_MAINWINDOW_HPP_INCLUDED

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>


//
// MainWindow
//

class MainWindowPrivate;

//! Main window.
class MainWindow final
	:	public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow() noexcept override;

protected:
	void closeEvent( QCloseEvent * e ) override;

private slots:
	//! Open GIF.
	void openGif();
	//! Save GIF.
	void saveGif();
	//! Save GIF as.
	void saveGifAs();
	//! Quit.
	void quit();
	//! Frame checked/unchecked.
	void frameChecked( int idx, bool on );
	//! Crop.
	void crop( bool on );
	//! Cancel edit.
	void cancelEdit();
	//! Apply edit.
	void applyEdit();
	//! About dialog
	void about();
	//! About Qt dialog.
	void aboutQt();
	//! Licenses.
	void licenses();
	//! Play/stop.
	void playStop();
	//! Show next frame.
	void showNextFrame();

protected:
	void resizeEvent( QResizeEvent * e ) override;

private:
	Q_DISABLE_COPY( MainWindow )

	QScopedPointer< MainWindowPrivate > d;
}; // class MainWindow

#endif // GIF_EDITOR_MAINWINDOW_HPP_INCLUDED
