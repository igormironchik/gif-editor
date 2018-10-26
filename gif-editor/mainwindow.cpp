
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
#include "frameontape.hpp"

// Qt include.
#include <QMenuBar>
#include <QApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>

// Magick++ include.
#include <Magick++.h>
#include <Magick++/Exception.h>

// C++ include.
#include <vector>


//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	m_editMode( EditMode::Unknow )
		,	m_view( new View( parent ) )
		,	m_crop( nullptr )
		,	m_save( nullptr )
		,	m_saveAs( nullptr )
		,	m_open( nullptr )
		,	m_applyEdit( nullptr )
		,	m_cancelEdit( nullptr )
		,	q( parent )
	{
	}

	//! Edit mode.
	enum class EditMode {
		Unknow,
		Crop
	}; // enum class EditMode

	//! Clear view.
	void clearView();
	//! Convert Magick::Image to QImage.
	QImage convert( const Magick::Image & img );
	//! Enable file actions.
	void enableFileActions( bool on = true )
	{
		m_save->setEnabled( on );
		m_saveAs->setEnabled( on );
		m_open->setEnabled( on );

		m_applyEdit->setEnabled( !on );
		m_cancelEdit->setEnabled( !on );
	}

	//! Current file name.
	QString m_currentGif;
	//! Frames.
	std::vector< Magick::Image > m_frames;
	//! Edit mode.
	EditMode m_editMode;
	//! View.
	View * m_view;
	//! Crop action.
	QAction * m_crop;
	//! Save action.
	QAction * m_save;
	//! Save as action.
	QAction * m_saveAs;
	//! Open action.
	QAction * m_open;
	//! Apply edit action.
	QAction * m_applyEdit;
	//! Cancel edit action.
	QAction * m_cancelEdit;
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
	d->m_open = file->addAction( QIcon( ":/img/document-open.png" ), tr( "Open" ),
		this, &MainWindow::openGif, tr( "Ctrl+O" ) );
	file->addSeparator();
	d->m_save = file->addAction( QIcon( ":/img/document-save.png" ), tr( "Save" ),
		this, &MainWindow::saveGif, tr( "Ctrl+S" ) );
	d->m_saveAs = file->addAction( QIcon( ":/img/document-save-as.png" ), tr( "Save As" ),
		this, &MainWindow::saveGifAs );
	file->addSeparator();
	file->addAction( QIcon( ":/img/application-exit.png" ), tr( "Quit" ),
		this, &MainWindow::quit, tr( "Ctrl+Q" ) );

	d->m_crop = new QAction( QIcon( ":/img/transform-crop.png" ),
		tr( "Crop" ), this );
	d->m_crop->setShortcut( tr( "Ctrl+C" ) );
	d->m_crop->setShortcutContext( Qt::ApplicationShortcut );
	d->m_crop->setCheckable( true );
	d->m_crop->setChecked( false );
	d->m_crop->setEnabled( false );

	d->m_applyEdit = new QAction( this );
	d->m_applyEdit->setShortcut( Qt::Key_Return );
	d->m_applyEdit->setShortcutContext( Qt::ApplicationShortcut );
	d->m_applyEdit->setEnabled( false );

	d->m_cancelEdit = new QAction( this );
	d->m_cancelEdit->setShortcut( Qt::Key_Escape );
	d->m_cancelEdit->setShortcutContext( Qt::ApplicationShortcut );
	d->m_cancelEdit->setEnabled( false );

	addAction( d->m_applyEdit );
	addAction( d->m_cancelEdit );

	connect( d->m_crop, &QAction::triggered, this, &MainWindow::crop );
	connect( d->m_applyEdit, &QAction::triggered, this, &MainWindow::applyEdit );
	connect( d->m_cancelEdit, &QAction::triggered, this, &MainWindow::cancelEdit );

	auto edit = menuBar()->addMenu( tr( "&Edit" ) );
	edit->addAction( d->m_crop );

	auto editToolBar = new QToolBar( tr( "Edit" ), this );
	editToolBar->addAction( d->m_crop );

	addToolBar( Qt::LeftToolBarArea, editToolBar );

	setCentralWidget( d->m_view );

	connect( d->m_view->tape(), &Tape::checkStateChanged,
		this, &MainWindow::frameChecked );
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
		if( isWindowModified() )
		{
			const auto btn = QMessageBox::question( this,
				tr( "GIF was changed..." ),
				tr( "\"%1\" was changed.\n"
					"Do you want to save it?" ) );

			if( btn == QMessageBox::Yes )
				saveGif();
		}

		d->clearView();

		setWindowModified( false );

		setWindowTitle( tr( "GIF Editor" ) );

		d->m_view->currentFrame()->setImage( QImage() );

		try {
			std::vector< Magick::Image > frames;

			Magick::readImages( &frames, fileName.toStdString() );

			Magick::coalesceImages( &d->m_frames, frames.begin(), frames.end() );

			QFileInfo info( fileName );

			setWindowTitle( tr( "GIF Editor - %1[*]" ).arg( info.fileName() ) );

			d->m_currentGif = fileName;

			std::for_each( d->m_frames.cbegin(), d->m_frames.cend(),
				[this] ( const Magick::Image & img )
				{
					this->d->m_view->tape()->addFrame( this->d->convert( img ) );
				} );

			if( !d->m_frames.empty() )
				d->m_view->tape()->setCurrentFrame( 1 );

			d->m_crop->setEnabled( true );
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
	std::vector< Magick::Image > toSave;

	for( int i = 0; i < d->m_view->tape()->count(); ++i )
	{
		if( d->m_view->tape()->frame( i + 1 )->isChecked() )
			toSave.push_back( d->m_frames.at( static_cast< std::size_t > ( i ) ) );
	}

	if( !toSave.empty() )
	{
		try {
			Magick::writeImages( toSave.begin(), toSave.end(), d->m_currentGif.toStdString() );

			d->m_view->tape()->removeUnchecked();

			d->m_frames = toSave;

			setWindowModified( false );
		}
		catch( const Magick::Exception & x )
		{
			QMessageBox::warning( this, tr( "Failed to save GIF..." ),
				QString::fromLocal8Bit( x.what() ) );
		}
	}
	else
	{
		QMessageBox::information( this, tr( "Can't save GIF..." ),
			tr( "Can't save GIF image with no frames." ) );
	}
}

void
MainWindow::saveGifAs()
{
	auto fileName = QFileDialog::getSaveFileName( this,
		tr( "Choose file to save to..." ), QString(), tr( "GIF (*.gif)" ) );

	if( !fileName.isEmpty() )
	{
		if( !fileName.endsWith( QLatin1String( ".gif" ), Qt::CaseInsensitive ) )
			fileName.append( QLatin1String( ".gif" ) );

		d->m_currentGif = fileName;

		QFileInfo info( fileName );

		setWindowTitle( tr( "GIF Editor - %1[*]" ).arg( info.fileName() ) );

		saveGif();
	}
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

void
MainWindow::frameChecked( int, bool )
{
	setWindowModified( true );
}

void
MainWindow::crop( bool on )
{
	if( on )
	{
		d->enableFileActions( false );

		d->m_editMode = MainWindowPrivate::EditMode::Crop;

		d->m_view->startCrop();
	}
	else
	{
		d->m_view->stopCrop();

		d->m_editMode = MainWindowPrivate::EditMode::Unknow;

		d->enableFileActions();
	}
}

void
MainWindow::cancelEdit()
{
	switch( d->m_editMode )
	{
		case MainWindowPrivate::EditMode::Crop :
		{
			d->m_view->stopCrop();

			d->enableFileActions();

			d->m_crop->setChecked( false );
		}
			break;

		default :
			break;
	}
}

void
MainWindow::applyEdit()
{
	switch( d->m_editMode )
	{
		case MainWindowPrivate::EditMode::Crop :
		{
		}
			break;

		default :
			break;
	}
}
