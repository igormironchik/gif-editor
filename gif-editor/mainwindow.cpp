
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
#include "view.hpp"
#include "tape.hpp"
#include "frame.hpp"

// Qt include.
#include <QMenuBar>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>

// Magick++ include.
#include <Magick++.h>
#include <Magick++/Exception.h>

// C++ include.
#include <list>


//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	m_view( new View( parent ) )
		,	q( parent )
	{
	}

	//! Clear view.
	void clearView();
	//! Convert Magick::Image to QImage.
	QImage convert( const Magick::Image & img );

	//! Current file name.
	QString m_currentGif;
	//! Frames.
	std::list< Magick::Image > m_frames;
	//! View.
	View * m_view;
	//! Parent.
	MainWindow * q;
}; // class MainWindowPrivate

void
MainWindowPrivate::clearView()
{
	m_frames.clear();
	m_view->tape()->clear();
	m_view->currentFrame()->setImage( QImage() );
}

QImage
MainWindowPrivate::convert( const Magick::Image & img )
{
    QImage qimg( static_cast< int > ( img.columns() ),
		static_cast< int > ( img.rows() ), QImage::Format_RGB888 );
    const Magick::PixelPacket * pixels;
    Magick::ColorRGB rgb;

    for( int y = 0; y < qimg.height(); ++y)
	{
        pixels = img.getConstPixels( 0, y, static_cast< std::size_t > ( qimg.width() ), 1 );

        for( int x = 0; x < qimg.width(); ++x )
		{
            rgb = ( *( pixels + x ) );

            qimg.setPixel( x, y, QColor( static_cast< int> ( 255 * rgb.red() ),
				static_cast< int > ( 255 * rgb.green() ),
				static_cast< int > ( 255 * rgb.blue() ) ).rgb());
        }
    }

	return qimg;
}


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

	setCentralWidget( d->m_view );
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
	const auto fileName = QFileDialog::getOpenFileName( this,
		tr( "Open GIF..." ), QString(), tr( "GIF (*.gif)" ) );

	if( !fileName.isEmpty() )
	{
		d->clearView();

		try {
			std::vector< Magick::Image > frames;

			Magick::readImages( &frames, fileName.toStdString() );

			Magick::coalesceImages( &d->m_frames, frames.begin(), frames.end() );

			std::for_each( d->m_frames.cbegin(), d->m_frames.cend(),
				[this] ( const Magick::Image & img )
				{
					this->d->m_view->tape()->addFrame( this->d->convert( img ) );
				} );
		}
		catch( const Magick::Exception & x )
		{
			d->clearView();

			QMessageBox::warning( this, tr( "Failed to open GIF..." ),
				QString::fromLocal8Bit( x.what() ) );
		}
	}
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
			saveGif();
	}

	QApplication::quit();
}
