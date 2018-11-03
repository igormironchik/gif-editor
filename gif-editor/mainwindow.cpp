
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
#include "busyindicator.hpp"
#include "about.hpp"

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
#include <QVector>
#include <QStackedWidget>
#include <QRunnable>
#include <QThreadPool>

// Magick++ include.
#include <Magick++.h>
#include <Magick++/Exception.h>

// C++ include.
#include <vector>
#include <algorithm>


//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	m_editMode( EditMode::Unknow )
		,	m_busyFlag( false )
		,	m_stack( new QStackedWidget( parent ) )
		,	m_busy( new BusyIndicator( m_stack ) )
		,	m_view( new View( m_stack ) )
		,	m_about( new About( parent ) )
		,	m_crop( nullptr )
		,	m_save( nullptr )
		,	m_saveAs( nullptr )
		,	m_open( nullptr )
		,	m_applyEdit( nullptr )
		,	m_cancelEdit( nullptr )
		,	m_quit( nullptr )
		,	m_editToolBar( nullptr )
		,	q( parent )
	{
		m_busy->setRadius( 75 );
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
	//! Initialize tape.
	void initTape()
	{
		std::for_each( m_frames.cbegin(), m_frames.cend(),
			[this] ( const Magick::Image & img )
			{
				this->m_view->tape()->addFrame( this->convert( img ) );

				QApplication::processEvents();
			} );
	}
	//! Busy state.
	void busy()
	{
		m_busyFlag = true;

		m_stack->setCurrentWidget( m_busy );

		m_busy->setRunning( true );

		m_crop->setEnabled( false );
		m_save->setEnabled( false );
		m_saveAs->setEnabled( false );
		m_open->setEnabled( false );
		m_quit->setEnabled( false );

		m_editToolBar->hide();
	}
	//! Ready state.
	void ready()
	{
		m_busyFlag = false;

		m_stack->setCurrentWidget( m_view );

		m_busy->setRunning( false );

		m_crop->setEnabled( true );

		if( !m_currentGif.isEmpty() )
		{
			if( q->isWindowModified() )
				m_save->setEnabled( true );

			m_saveAs->setEnabled( true );
		}

		m_open->setEnabled( true );
		m_quit->setEnabled( true );

		m_editToolBar->show();
	}
	//! Wait for thread pool.
	void waitThreadPool()
	{
		while( !QThreadPool::globalInstance()->waitForDone( 100 / 6 ) )
			QApplication::processEvents();
	}
	//! Set modified state.
	void setModified( bool on )
	{
		q->setWindowModified( on );

		if( on )
			m_save->setEnabled( true );
		else
			m_save->setEnabled( false );
	}

	//! Current file name.
	QString m_currentGif;
	//! Frames.
	std::vector< Magick::Image > m_frames;
	//! Edit mode.
	EditMode m_editMode;
	//! Busy flag.
	bool m_busyFlag;
	//! Stacked widget.
	QStackedWidget * m_stack;
	//! Busy indicator.
	BusyIndicator * m_busy;
	//! View.
	View * m_view;
	//! Widget about.
	About * m_about;
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
	//! Quit action.
	QAction * m_quit;
	//! Edit toolbar.
	QToolBar * m_editToolBar;
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

		QApplication::processEvents();
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
	d->m_quit = file->addAction( QIcon( ":/img/application-exit.png" ), tr( "Quit" ),
		this, &MainWindow::quit, tr( "Ctrl+Q" ) );

	d->m_save->setEnabled( false );
	d->m_saveAs->setEnabled( false );

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

	d->m_editToolBar = new QToolBar( tr( "Edit" ), this );
	d->m_editToolBar->addAction( d->m_crop );

	addToolBar( Qt::LeftToolBarArea, d->m_editToolBar );

	d->m_editToolBar->hide();

	auto help = menuBar()->addMenu( tr( "&Help" ) );
	help->addAction( QIcon( ":/img/icon_22x22.png" ), tr( "About" ),
		this, &MainWindow::about );
	help->addAction( QIcon( ":/img/qt.png" ), tr( "About Qt" ),
		this, &MainWindow::aboutQt );

	d->m_stack->addWidget( d->m_about );
	d->m_stack->addWidget( d->m_view );
	d->m_stack->addWidget( d->m_busy );

	setCentralWidget( d->m_stack );

	connect( d->m_view->tape(), &Tape::checkStateChanged,
		this, &MainWindow::frameChecked );
}

MainWindow::~MainWindow() noexcept
{
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	if ( d->m_busyFlag )
	{
		const auto btn = QMessageBox::question( this, tr( "GIF editor is busy..." ),
			tr( "GIF editor is busy.\nDo you want to terminate the application?" ) );

		if( btn == QMessageBox::Yes )
			exit( -1 );
		else
			e->ignore();
	}
	else
		e->accept();

	quit();
}

namespace /* anonymous */ {

class RunnableWithException
	:	public QRunnable
{
public:
	std::exception_ptr exception() const
	{
		return m_eptr;
	}

protected:
	std::exception_ptr m_eptr;
}; // class RunnableWithException


class ReadGIF final
	:	public RunnableWithException
{
public:
	ReadGIF( std::vector< Magick::Image > * container,
		const std::string & fileName )
		:	m_container( container )
		,	m_fileName( fileName )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		try {
			Magick::readImages( m_container, m_fileName );
		}
		catch( ... )
		{
			m_eptr = std::current_exception();
		}
	}

private:
	std::vector< Magick::Image > * m_container;
	std::string m_fileName;
}; // class ReadGIF


class CoalesceGIF final
	:	public RunnableWithException
{
public:
	CoalesceGIF( std::vector< Magick::Image > * container,
		std::vector< Magick::Image >::iterator first,
		std::vector< Magick::Image >::iterator last )
		:	m_container( container )
		,	m_first( first )
		,	m_last( last )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		try {
			Magick::coalesceImages( m_container, m_first, m_last );
		}
		catch( ... )
		{
			m_eptr = std::current_exception();
		}
	}

private:
	std::vector< Magick::Image > * m_container;
	std::vector< Magick::Image >::iterator m_first;
	std::vector< Magick::Image >::iterator m_last;
}; // class CoalesceGIF

} /* namespace anonymous */

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

		d->busy();

		d->clearView();

		d->setModified( false );

		setWindowTitle( tr( "GIF Editor" ) );

		d->m_currentGif = QString();

		QApplication::processEvents();

		d->m_view->currentFrame()->setImage( QImage() );

		try {
			std::vector< Magick::Image > frames;

			ReadGIF read( &frames, fileName.toStdString() );
			QThreadPool::globalInstance()->start( &read );

			d->waitThreadPool();

			if( read.exception() )
				std::rethrow_exception( read.exception() );

			CoalesceGIF coalesce( &d->m_frames, frames.begin(), frames.end() );
			QThreadPool::globalInstance()->start( &coalesce );

			d->waitThreadPool();

			if( coalesce.exception() )
				std::rethrow_exception( coalesce.exception() );

			QFileInfo info( fileName );

			setWindowTitle( tr( "GIF Editor - %1[*]" ).arg( info.fileName() ) );

			d->m_currentGif = fileName;

			d->initTape();

			if( !d->m_frames.empty() )
				d->m_view->tape()->setCurrentFrame( 1 );

			d->m_crop->setEnabled( true );
			d->m_saveAs->setEnabled( true );

			d->ready();
		}
		catch( const Magick::Exception & x )
		{
			d->clearView();

			d->ready();

			d->m_editToolBar->hide();

			d->m_stack->setCurrentWidget( d->m_about );

			QMessageBox::warning( this, tr( "Failed to open GIF..." ),
				QString::fromLocal8Bit( x.what() ) );
		}
		catch( const std::bad_alloc & )
		{
			d->clearView();

			d->ready();

			d->m_editToolBar->hide();

			d->m_stack->setCurrentWidget( d->m_about );

			QMessageBox::critical( this, tr( "Failed to open GIF..." ),
				tr( "Out of memory." ) );
		}
	}
}

namespace /* anonymous */ {

class WriteGIF final
	:	public RunnableWithException
{
public:
	WriteGIF( std::vector< Magick::Image >::iterator first,
		std::vector< Magick::Image >::iterator last,
		const std::string & fileName )
		:	m_first( first )
		,	m_last( last )
		,	m_fileName( fileName )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		try {
			Magick::writeImages( m_first, m_last, m_fileName );
		}
		catch( ... )
		{
			m_eptr = std::current_exception();
		}
	}

private:
	std::vector< Magick::Image >::iterator m_first;
	std::vector< Magick::Image >::iterator m_last;
	std::string m_fileName;
}; // class WriteGIF

} /* namespace anonymous */

void
MainWindow::saveGif()
{
	try {
		d->busy();

		std::vector< Magick::Image > toSave;

		for( int i = 0; i < d->m_view->tape()->count(); ++i )
		{
			if( d->m_view->tape()->frame( i + 1 )->isChecked() )
				toSave.push_back( d->m_frames.at( static_cast< std::size_t > ( i ) ) );
		}

		QApplication::processEvents();

		if( !toSave.empty() )
		{
			try {
				WriteGIF runnable( toSave.begin(), toSave.end(), d->m_currentGif.toStdString() );
				QThreadPool::globalInstance()->start( &runnable );

				d->waitThreadPool();

				if( runnable.exception() )
					std::rethrow_exception( runnable.exception() );

				d->m_view->tape()->removeUnchecked();

				QApplication::processEvents();

				d->m_frames = toSave;

				d->setModified( false );
			}
			catch( const Magick::Exception & x )
			{
				d->ready();

				QMessageBox::warning( this, tr( "Failed to save GIF..." ),
					QString::fromLocal8Bit( x.what() ) );
			}
		}
		else
		{
			QMessageBox::information( this, tr( "Can't save GIF..." ),
				tr( "Can't save GIF image with no frames." ) );
		}

		d->ready();
	}
	catch( const std::bad_alloc & )
	{
		d->ready();

		QMessageBox::critical( this, tr( "Failed to save GIF..." ),
			tr( "Out of memory." ) );
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
	if( !d->m_busyFlag )
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
}

void
MainWindow::frameChecked( int, bool )
{
	d->setModified( true );
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

			d->m_editMode = MainWindowPrivate::EditMode::Unknow;

			QApplication::processEvents();
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
			const auto rect = d->m_view->cropRect();

			if( !rect.isNull() && rect != d->m_view->currentFrame()->image().rect() )
			{
				d->busy();

				QVector< int > unchecked;

				for( int i = 1; i <= d->m_view->tape()->count(); ++i )
				{
					if( !d->m_view->tape()->frame( i )->isChecked() )
						unchecked.append( i );
				}

				QApplication::processEvents();

				try {
					auto tmpFrames = d->m_frames;

					std::for_each( tmpFrames.begin(), tmpFrames.end(),
						[&rect] ( auto & frame )
						{
							frame.crop( Magick::Geometry( rect.width(), rect.height(),
								rect.x(), rect.y() ) );

							QApplication::processEvents();

							frame.repage();

							QApplication::processEvents();
						} );

					const auto current = d->m_view->tape()->currentFrame()->counter();
					d->m_view->tape()->clear();
					d->m_frames = tmpFrames;

					QApplication::processEvents();

					d->initTape();

					d->m_view->tape()->setCurrentFrame( current );

					for( const auto & i : qAsConst( unchecked ) )
						d->m_view->tape()->frame( i )->setChecked( false );

					d->setModified( true );

					cancelEdit();

					d->ready();
				}
				catch( const Magick::Exception & x )
				{
					d->ready();

					cancelEdit();

					QMessageBox::warning( this, tr( "Failed to crop GIF..." ),
						QString::fromLocal8Bit( x.what() ) );
				}
				catch( const std::bad_alloc & )
				{
					d->ready();

					cancelEdit();

					QMessageBox::critical( this, tr( "Failed to crop GIF..." ),
						tr( "Out of memory." ) );
				}
			}
			else
				cancelEdit();
		}
			break;

		default :
			break;
	}
}

void
MainWindow::about()
{
	QMessageBox::about( this, tr( "About GIF editor" ),
		tr( "GIF editor.\n\n"
			"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
			"Copyright (c) 2018 Igor Mironchik.\n\n"
			"Licensed under GNU GPL 3.0." ) );
}

void
MainWindow::aboutQt()
{
	QMessageBox::aboutQt( this );
}
