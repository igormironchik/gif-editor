
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
#include "mainwindow.hpp"

// Qt include.
#include <QMenuBar>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>


//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	q( parent )
	{
	}

	//! Current file name.
	QString m_currentGif;
	//! Parent.
	MainWindow * q;
}; // class MainWindowPrivate


//
// MainWindow
//

MainWindow::MainWindow()
	:	d( new MainWindowPrivate( this ) )
{
	setWindowTitle( tr( "GIF Editor" ) );

	auto file = menuBar()->addMenu( tr( "&File" ) );
	file->addAction( QIcon( ":/img/document-open.png" ), tr( "Open" ),
		this, &MainWindow::openGif, tr( "Ctrl+O" ) );
	file->addSeparator();
	file->addAction( QIcon( ":/img/document-save.png" ), tr( "Save" ),
		this, &MainWindow::saveGif, tr( "Ctrl+S" ) );
	file->addAction( QIcon( ":/img/document-save-as.png" ), tr( "Save As" ),
		this, &MainWindow::saveGifAs );
	file->addSeparator();
	file->addAction( QIcon( ":/img/application-exit.png" ), tr( "Quit" ),
		this, &MainWindow::quit, tr( "Ctrl+Q" ) );
}

MainWindow::~MainWindow() noexcept
{
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	quit();

	e->accept();
}

void
MainWindow::openGif()
{

}

void
MainWindow::saveGif()
{

}

void
MainWindow::saveGifAs()
{

}

void
MainWindow::quit()
{
	if( isWindowModified() )
	{
		auto btn = QMessageBox::question( this, tr( "GIF was changed..." ),
			tr( "GIF was changed. Do you want to save changes?" ) );

		if( btn == QMessageBox::Yes )
		{
			if( !d->m_currentGif.isEmpty() )
				saveGif();
			else
				saveGifAs();
		}
	}

	QApplication::quit();
}
