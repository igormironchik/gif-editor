
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
#include <QStandardPaths>
#include <QResizeEvent>
#include <QTimer>

// C++ include.
#include <vector>
#include <algorithm>

// Widgets include.
#include <Widgets/LicenseDialog>

// bitmap include.
#include <bmp.h>
#include <gif.h>


//
// MainWindowPrivate
//

class MainWindowPrivate {
public:
	MainWindowPrivate( MainWindow * parent )
		:	m_editMode( EditMode::Unknow )
		,	m_busyFlag( false )
		,	m_quitFlag( false )
		,	m_playing( false )
		,	m_stack( new QStackedWidget( parent ) )
		,	m_busy( new BusyIndicator( m_stack ) )
		,	m_view( new View( m_frames, m_stack ) )
		,	m_about( new About( parent ) )
		,	m_crop( nullptr )
		,	m_playStop( nullptr )
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
	//! Enable file actions.
	void enableFileActions( bool on = true )
	{
		m_save->setEnabled( on );
		m_saveAs->setEnabled( on );
		m_open->setEnabled( on );

		m_applyEdit->setEnabled( !on );
		m_cancelEdit->setEnabled( !on );

		m_playStop->setEnabled( on );
	}
	//! Initialize tape.
	void initTape()
	{
		for( int i = 0, last = m_frames.size(); i < last; ++i )
		{
			m_view->tape()->addFrame( { m_frames, i, false } );

			QApplication::processEvents();
		};
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
		while( !QThreadPool::globalInstance()->waitForDone( 5 ) )
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
	QVector< QPair< QImage, int > > m_frames;
	//! Edit mode.
	EditMode m_editMode;
	//! Busy flag.
	bool m_busyFlag;
	//! Quit flag.
	bool m_quitFlag;
	//! Play/stop flag.
	bool m_playing;
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
	//! Play/stop action.
	QAction * m_playStop;
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
	//! Play timer.
	QTimer * m_playTimer;
	//! Parent.
	MainWindow * q;
}; // class MainWindowPrivate

void
MainWindowPrivate::clearView()
{
	m_view->currentFrame()->clearImage();
	m_view->tape()->clear();
	m_frames.clear();
}


//
// MainWindow
//

MainWindow::MainWindow()
	:	d( new MainWindowPrivate( this ) )
{
	setWindowTitle( tr( "GIF Editor" ) );

	auto file = menuBar()->addMenu( tr( "&File" ) );
	d->m_open = file->addAction( QIcon( QStringLiteral( ":/img/document-open.png" ) ), tr( "Open" ),
		tr( "Ctrl+O" ), this, &MainWindow::openGif );
	file->addSeparator();
	d->m_save = file->addAction( QIcon( QStringLiteral( ":/img/document-save.png" ) ), tr( "Save" ),
		tr( "Ctrl+S" ), this, &MainWindow::saveGif );
	d->m_saveAs = file->addAction( QIcon( QStringLiteral( ":/img/document-save-as.png" ) ), tr( "Save As" ),
		this, &MainWindow::saveGifAs );
	file->addSeparator();
	d->m_quit = file->addAction( QIcon( QStringLiteral( ":/img/application-exit.png" ) ), tr( "Quit" ),
		tr( "Ctrl+Q" ), this, &MainWindow::quit );

	d->m_save->setEnabled( false );
	d->m_saveAs->setEnabled( false );

	d->m_crop = new QAction( QIcon( QStringLiteral( ":/img/transform-crop.png" ) ),
		tr( "Crop" ), this );
	d->m_crop->setShortcut( tr( "Ctrl+C" ) );
	d->m_crop->setShortcutContext( Qt::ApplicationShortcut );
	d->m_crop->setCheckable( true );
	d->m_crop->setChecked( false );
	d->m_crop->setEnabled( false );

	d->m_playStop = new QAction( QIcon( QStringLiteral( ":/img/media-playback-start.png" ) ),
		tr( "Play" ), this );
	d->m_playStop->setEnabled( false );

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

	d->m_playTimer = new QTimer( this );

	connect( d->m_crop, &QAction::triggered, this, &MainWindow::crop );
	connect( d->m_playStop, &QAction::triggered, this, &MainWindow::playStop );
	connect( d->m_applyEdit, &QAction::triggered, this, &MainWindow::applyEdit );
	connect( d->m_cancelEdit, &QAction::triggered, this, &MainWindow::cancelEdit );
	connect( d->m_playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame );

	auto edit = menuBar()->addMenu( tr( "&Edit" ) );
	edit->addAction( d->m_crop );

	d->m_editToolBar = new QToolBar( tr( "Tools" ), this );
	d->m_editToolBar->addAction( d->m_playStop );
	d->m_editToolBar->addSeparator();
	d->m_editToolBar->addAction( d->m_crop );

	addToolBar( Qt::LeftToolBarArea, d->m_editToolBar );

	d->m_editToolBar->hide();

	auto help = menuBar()->addMenu( tr( "&Help" ) );
	help->addAction( QIcon( QStringLiteral( ":/img/icon_22x22.png" ) ), tr( "About" ),
		this, &MainWindow::about );
	help->addAction( QIcon( QStringLiteral( ":/img/qt.png" ) ), tr( "About Qt" ),
		this, &MainWindow::aboutQt );
	help->addAction( QIcon( QStringLiteral( ":/img/bookmarks-organize.png" ) ),
		tr( "Licenses" ), this, &MainWindow::licenses );

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

inline QImage toQImage( Bitmap * b )
{
	return QImage( bm_raw_data( b ), bm_width( b ), bm_height( b ), QImage::Format_ARGB32 ).copy();
}


class ReadGIF final
	:	public QRunnable
{
public:
	ReadGIF( QVector< QPair< QImage, int > > & container,
		const QString & fileName )
		:	m_container( container )
		,	m_fileName( fileName )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		auto gif = gif_load( m_fileName.toLocal8Bit().data() );

		auto frame = gif->frames;

		for( int i = 0; i < gif->n; ++i )
		{
			m_container.push_back( { toQImage( frame->image ), frame->delay * 10 } );

			++frame;
		}

		gif_free( gif );
	}

private:
	QVector< QPair< QImage, int > > & m_container;
	QString m_fileName;
}; // class ReadGIF


class CropGIF final
	:	public QRunnable
{
public:
	CropGIF( QVector< QPair< QImage, int > > & container,
		const QRect & rect )
		:	m_container( container )
		,	m_rect( rect )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		for( int i = 0; i < m_container.size(); ++i )
			m_container[ i ].first = m_container[ i ].first.copy( m_rect );
	}

private:
	QVector< QPair< QImage, int > > & m_container;
	QRect m_rect;
}; // class CropGIF

} /* namespace anonymous */

void
MainWindow::openGif()
{
	static const auto pictureLocations =
		QStandardPaths::standardLocations( QStandardPaths::PicturesLocation );

	const auto fileName = QFileDialog::getOpenFileName( this,
		tr( "Open GIF..." ),
		( !pictureLocations.isEmpty() ? pictureLocations.first() : QString() ),
		tr( "GIF (*.gif)" ) );

	if( !fileName.isEmpty() )
	{
		if( isWindowModified() )
		{
			const auto btn = QMessageBox::question( this,
				tr( "GIF was changed..." ),
				tr( "\"%1\" was changed.\n"
					"Do you want to save it?" ).arg( fileName ) );

			if( btn == QMessageBox::Yes )
				saveGif();
		}

		d->busy();

		d->clearView();

		d->setModified( false );

		setWindowTitle( tr( "GIF Editor" ) );

		d->m_currentGif = QString();

		QApplication::processEvents();

		try {
			ReadGIF read( d->m_frames, fileName );
			QThreadPool::globalInstance()->start( &read );

			d->waitThreadPool();

			QFileInfo info( fileName );

			setWindowTitle( tr( "GIF Editor - %1[*]" ).arg( info.fileName() ) );

			d->m_currentGif = fileName;

			d->initTape();

			if( !d->m_frames.isEmpty() )
				d->m_view->tape()->setCurrentFrame( 1 );

			d->m_crop->setEnabled( true );
			d->m_playStop->setEnabled( true );
			d->m_saveAs->setEnabled( true );

			d->ready();
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
	:	public QRunnable
{
public:
	WriteGIF( const QVector< QPair< QImage, int > > & container, const QString & fileName )
		:	m_container( container )
		,	m_fileName( fileName )
	{
		setAutoDelete( false );
	}

	void run() override
	{
		if( !m_container.isEmpty() )
		{
			auto gif = gif_create( m_container.first().first.width(),
				m_container.first().first.height() );
			gif->repetitions = 0;

			for( const auto & p : m_container )
			{
				auto b = bm_create( p.first.width(), p.first.height() );
				memcpy( bm_raw_data( b ), p.first.bits(), p.first.sizeInBytes() );
				auto frame = gif_add_frame( gif, b );
				frame->delay = p.second / 10;
				bm_free( b );
			}

			gif_save( gif, m_fileName.toLocal8Bit().data() );

			gif_free( gif );
		}
	}

private:
	const QVector< QPair< QImage, int > > & m_container;
	QString m_fileName;
}; // class WriteGIF

} /* namespace anonymous */

void
MainWindow::saveGif()
{
	try {
		d->busy();

		QVector< QPair< QImage, int > > toSave;

		for( int i = 0; i < d->m_view->tape()->count(); ++i )
		{
			if( d->m_view->tape()->frame( i + 1 )->isChecked() )
				toSave.push_back( d->m_frames.at( i ) );

			QApplication::processEvents();
		}

		if( !toSave.empty() )
		{
			d->m_frames = toSave;
			toSave.clear();

			WriteGIF runnable( d->m_frames, d->m_currentGif );
			QThreadPool::globalInstance()->start( &runnable );

			d->waitThreadPool();

			d->m_view->currentFrame()->clearImage();
			d->m_view->tape()->removeUnchecked();

			QApplication::processEvents();

			for( int i = 1; i <= d->m_view->tape()->count(); ++i )
			{
				d->m_view->tape()->frame( i )->setImagePos( i - 1 );
				d->m_view->tape()->frame( i )->applyImage();
				QApplication::processEvents();
			}

			d->m_view->currentFrame()->setImagePos( d->m_view->currentFrame()->image().m_pos );
			d->m_view->currentFrame()->applyImage();

			d->setModified( false );
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
		if( !fileName.endsWith( QStringLiteral( ".gif" ), Qt::CaseInsensitive ) )
			fileName.append( QStringLiteral( ".gif" ) );

		d->m_currentGif = fileName;

		QFileInfo info( fileName );

		setWindowTitle( tr( "GIF Editor - %1[*]" ).arg( info.fileName() ) );

		saveGif();
	}
}

void
MainWindow::quit()
{
	if( !d->m_busyFlag && !d->m_quitFlag )
	{
		if( isWindowModified() )
		{
			auto btn = QMessageBox::question( this, tr( "GIF was changed..." ),
				tr( "GIF was changed. Do you want to save changes?" ) );

			if( btn == QMessageBox::Yes )
				saveGif();
		}

		d->m_quitFlag = true;

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

			if( !rect.isNull() && rect != d->m_view->currentFrame()->imageRect() )
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
					CropGIF crop( d->m_frames, rect );
					QThreadPool::globalInstance()->start( &crop );

					d->waitThreadPool();

					const auto current = d->m_view->tape()->currentFrame()->counter();
					d->m_view->tape()->clear();

					QApplication::processEvents();

					d->initTape();

					d->m_view->tape()->setCurrentFrame( current );

					for( const auto & i : qAsConst( unchecked ) )
						d->m_view->tape()->frame( i )->setChecked( false );

					d->setModified( true );

					cancelEdit();

					d->ready();
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

void
MainWindow::licenses()
{
	LicenseDialog msg( this );
	msg.addLicense( QStringLiteral( "The Oxygen Icon Theme" ),
		QStringLiteral( "<p><b>The Oxygen Icon Theme</b>\n\n</p>"
		"<p>Copyright (C) 2007 Nuno Pinheiro &lt;nuno@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 David Vignoni &lt;david@icon-king.com&gt;\n</p>"
		"<p>Copyright (C) 2007 David Miller &lt;miller@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Johann Ollivier Lapeyre &lt;johann@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Kenneth Wimer &lt;kwwii@bootsplash.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Riccardo Iaconelli &lt;riccardo@oxygen-icons.org&gt;\n</p>"
		"<p>\nand others\n</p>"
		"\n"
		"<p>This library is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU Lesser General Public "
		"License as published by the Free Software Foundation; either "
		"version 3 of the License, or (at your option) any later version.\n</p>"
		"\n"
		"<p>This library is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		"Lesser General Public License for more details.\n</p>"
		"\n"
		"<p>You should have received a copy of the GNU Lesser General Public "
		"License along with this library. If not, see "
		"<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>.\n</p>"
		"\n"
		"<p>Clarification:\n</p>"
		"\n"
		"<p>The GNU Lesser General Public License or LGPL is written for "
		"software libraries in the first place. We expressly want the LGPL to "
		"be valid for this artwork library too.\n</p>"
		"\n"
		"<p>KDE Oxygen theme icons is a special kind of software library, it is an "
		"artwork library, it's elements can be used in a Graphical User Interface, or "
		"GUI.\n</p>"
		"\n"
		"<p>Source code, for this library means:\n</p>"
		"<p><ul> <li>where they exist, SVG;\n</li>"
		" <li>otherwise, if applicable, the multi-layered formats xcf or psd, or "
		"otherwise png.\n</li></ul></p>"
		"\n"
		"<p>The LGPL in some sections obliges you to make the files carry "
		"notices. With images this is in some cases impossible or hardly useful.\n</p>"
		"\n"
		"<p>With this library a notice is placed at a prominent place in the directory "
		"containing the elements. You may follow this practice.\n</p>"
		"\n"
		"<p>The exception in section 5 of the GNU Lesser General Public License covers "
		"the use of elements of this art library in a GUI.\n</p>"
		"\n"
		"<p>kde-artists [at] kde.org\n</p>"
		"\n"
		"<p><b>GNU LESSER GENERAL PUBLIC LICENSE</b>\n</p>"
		"<p>Version 3, 29 June 2007\n</p>"
		"\n"
		"<p>Copyright (C) 2007 Free Software Foundation, Inc. <a href=\"http://fsf.org/\">&lt;http://fsf.org/&gt;</a> "
		"Everyone is permitted to copy and distribute verbatim copies "
		"of this license document, but changing it is not allowed.\n</p>"
		"\n"
		"\n"
		"<p>This version of the GNU Lesser General Public License incorporates "
		"the terms and conditions of version 3 of the GNU General Public "
		"License, supplemented by the additional permissions listed below.\n</p>"
		"\n"
		"<p><b>0.</b> Additional Definitions.\n</p>"
		"\n"
		"<p>As used herein, \"this License\" refers to version 3 of the GNU Lesser "
		"General Public License, and the \"GNU GPL\" refers to version 3 of the GNU "
		"General Public License.\n</p>"
		"\n"
		"<p>\"The Library\" refers to a covered work governed by this License, "
		"other than an Application or a Combined Work as defined below.\n</p>"
		"\n"
		"<p>An \"Application\" is any work that makes use of an interface provided "
		"by the Library, but which is not otherwise based on the Library. "
		"Defining a subclass of a class defined by the Library is deemed a mode "
		"of using an interface provided by the Library.\n</p>"
		"\n"
		"<p>A \"Combined Work\" is a work produced by combining or linking an "
		"Application with the Library.  The particular version of the Library "
		"with which the Combined Work was made is also called the \"Linked "
		"Version\".\n</p>"
		"\n"
		"<p>The \"Minimal Corresponding Source\" for a Combined Work means the "
		"Corresponding Source for the Combined Work, excluding any source code "
		"for portions of the Combined Work that, considered in isolation, are "
		"based on the Application, and not on the Linked Version.\n</p>"
		"\n"
		"<p>The \"Corresponding Application Code\" for a Combined Work means the "
		"object code and/or source code for the Application, including any data "
		"and utility programs needed for reproducing the Combined Work from the "
		"Application, but excluding the System Libraries of the Combined Work.\n</p>"
		"\n"
		"<p><b>1.</b> Exception to Section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p>You may convey a covered work under sections 3 and 4 of this License "
		"without being bound by section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p><b>2.</b> Conveying Modified Versions.\n</p>"
		"\n"
		"<p>If you modify a copy of the Library, and, in your modifications, a "
		"facility refers to a function or data to be supplied by an Application "
		"that uses the facility (other than as an argument passed when the "
		"facility is invoked), then you may convey a copy of the modified "
		"version:\n</p>"
		"\n"
		"<p><b>a)</b> under this License, provided that you make a good faith effort to "
		"ensure that, in the event an Application does not supply the "
		"function or data, the facility still operates, and performs "
		"whatever part of its purpose remains meaningful, or\n</p>"
		"\n"
		"<p><b>b)</b> under the GNU GPL, with none of the additional permissions of "
		"this License applicable to that copy.\n</p>"
		"\n"
		"<p><b>3.</b> Object Code Incorporating Material from Library Header Files.\n</p>"
		"\n"
		"<p>The object code form of an Application may incorporate material from "
		"a header file that is part of the Library.  You may convey such object "
		"code under terms of your choice, provided that, if the incorporated "
		"material is not limited to numerical parameters, data structure "
		"layouts and accessors, or small macros, inline functions and templates "
		"(ten or fewer lines in length), you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the object code that the "
		"Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the object code with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>4.</b> Combined Works.\n</p>"
		"\n"
		"<p>You may convey a Combined Work under terms of your choice that, "
		"taken together, effectively do not restrict modification of the "
		"portions of the Library contained in the Combined Work and reverse "
		"engineering for debugging such modifications, if you also do each of "
		"the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the Combined Work that "
		"the Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the Combined Work with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>c)</b> For a Combined Work that displays copyright notices during "
		"execution, include the copyright notice for the Library among "
		"these notices, as well as a reference directing the user to the "
		"copies of the GNU GPL and this license document.\n</p>"
		"\n"
		"<p><b>d)</b> Do one of the following:\n</p>"
		"\n"
		"<p>    <b>0)</b> Convey the Minimal Corresponding Source under the terms of this "
		"License, and the Corresponding Application Code in a form "
		"suitable for, and under terms that permit, the user to "
		"recombine or relink the Application with a modified version of "
		"the Linked Version to produce a modified Combined Work, in the "
		"manner specified by section 6 of the GNU GPL for conveying "
		"Corresponding Source.\n</p>"
		"\n"
		"<p>    <b>1)</b> Use a suitable shared library mechanism for linking with the "
		"Library.  A suitable mechanism is one that (a) uses at run time "
		"a copy of the Library already present on the user's computer "
		"system, and (b) will operate properly with a modified version "
		"of the Library that is interface-compatible with the Linked "
		"Version.\n</p>"
		"\n"
		"<p><b>e)</b> Provide Installation Information, but only if you would otherwise "
		"be required to provide such information under section 6 of the "
		"GNU GPL, and only to the extent that such information is "
		"necessary to install and execute a modified version of the "
		"Combined Work produced by recombining or relinking the "
		"Application with a modified version of the Linked Version. (If "
		"you use option 4d0, the Installation Information must accompany "
		"the Minimal Corresponding Source and Corresponding Application "
		"Code. If you use option 4d1, you must provide the Installation "
		"Information in the manner specified by section 6 of the GNU GPL "
		"for conveying Corresponding Source.)\n</p>"
		"\n"
		"<p><b>5.</b> Combined Libraries.\n</p>"
		"\n"
		"<p>You may place library facilities that are a work based on the "
		"Library side by side in a single library together with other library "
		"facilities that are not Applications and are not covered by this "
		"License, and convey such a combined library under terms of your "
		"choice, if you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Accompany the combined library with a copy of the same work based "
		"on the Library, uncombined with any other library facilities, "
		"conveyed under the terms of this License.\n</p>"
		"\n"
		"<p><b>b)</b> Give prominent notice with the combined library that part of it "
		"is a work based on the Library, and explaining where to find the "
		"accompanying uncombined form of the same work.\n</p>"
		"\n"
		"<p><b>6.</b> Revised Versions of the GNU Lesser General Public License.\n</p>"
		"\n"
		"<p>The Free Software Foundation may publish revised and/or new versions "
		"of the GNU Lesser General Public License from time to time. Such new "
		"versions will be similar in spirit to the present version, but may "
		"differ in detail to address new problems or concerns.\n</p>"
		"\n"
		"<p>Each version is given a distinguishing version number. If the "
		"Library as you received it specifies that a certain numbered version "
		"of the GNU Lesser General Public License \"or any later version\" "
		"applies to it, you have the option of following the terms and "
		"conditions either of that published version or of any later version "
		"published by the Free Software Foundation. If the Library as you "
		"received it does not specify a version number of the GNU Lesser "
		"General Public License, you may choose any version of the GNU Lesser "
		"General Public License ever published by the Free Software Foundation.\n</p>"
		"\n"
		"<p>If the Library as you received it specifies that a proxy can decide "
		"whether future versions of the GNU Lesser General Public License shall "
		"apply, that proxy's public statement of acceptance of any version is "
		"permanent authorization for you to choose that version for the "
		"Library.</p>" ) );
	msg.addLicense( QStringLiteral( "Bitmap API" ),
		QStringLiteral( "<p><b>Bitmap API</b>\n\n</p>"
			"<p>MIT No Attribution</p>\n\n"
			"<p>Copyright (c) 2017 Werner Stoop</p>\n\n"
			"<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this "
			"software and associated documentation files (the \"Software\"), to deal in the Software "
			"without restriction, including without limitation the rights to use, copy, modify, "
			"merge, publish, distribute, sublicense, and/or sell copies of the Software, and to "
			"permit persons to whom the Software is furnished to do so.</p>\n\n"
			"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, "
			"INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A "
			"PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT "
			"HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION "
			"OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE "
			"SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>" ) );

	msg.exec();
}

void
MainWindow::resizeEvent( QResizeEvent * e )
{
	static bool tapeHeightInit = false;

	if( !tapeHeightInit && d->m_stack->currentWidget() != d->m_view )
	{
		tapeHeightInit = true;

		d->m_view->resize( 800, 600 );

		QApplication::processEvents();

		d->m_view->tape()->setMinimumHeight( d->m_view->tape()->height() );
	}

	e->accept();
}

void
MainWindow::playStop()
{
	if( d->m_playing )
	{
		d->m_playTimer->stop();
		d->m_playStop->setText( tr( "Play" ) );
		d->m_playStop->setIcon( QIcon( ":/img/media-playback-start.png" ) );
	}
	else
	{
		d->m_playStop->setText( tr( "Stop" ) );
		d->m_playStop->setIcon( QIcon( ":/img/media-playback-stop.png" ) );
		const auto & img = d->m_view->tape()->currentFrame()->image();
		d->m_playTimer->start( img.m_data.at( img.m_pos ).second );
	}

	d->m_playing = !d->m_playing;
}

void
MainWindow::showNextFrame()
{
	bool frameSet = false;

	for( int i = d->m_view->tape()->currentFrame()->counter() + 1;
		i <= d->m_view->tape()->count(); ++i )
	{
		if( d->m_view->tape()->frame( i )->isChecked() )
		{
			const auto & img = d->m_view->tape()->frame( i )->image();
			d->m_playTimer->start( img.m_data.at( img.m_pos ).second );
			d->m_view->tape()->setCurrentFrame( i );
			frameSet = true;
			break;
		}
	}

	if( !frameSet )
	{
		for( int i = 1; i < d->m_view->tape()->currentFrame()->counter(); ++i )
		{
			if( d->m_view->tape()->frame( i )->isChecked() )
			{
				const auto & img = d->m_view->tape()->frame( i )->image();
				d->m_playTimer->start( img.m_data.at( img.m_pos ).second );
				d->m_view->tape()->setCurrentFrame( i );
				frameSet = true;
				break;
			}
		}
	}


	if( frameSet )
		d->m_view->scrollTo( d->m_view->tape()->currentFrame()->counter() );
}
