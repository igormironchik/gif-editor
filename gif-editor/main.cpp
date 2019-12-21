
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

// Qt include.
#include <QApplication>
#include <QTranslator>
#include <QLocale>

// GIF editor include.
#include "mainwindow.hpp"

// Magick++ include.
#include <Magick++.h>


int main( int argc, char ** argv )
{
	Magick::InitializeMagick( nullptr );

	QApplication app( argc, argv );

	QIcon appIcon( QStringLiteral( ":/img/icon_256x256.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_128x128.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_64x64.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_48x48.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_32x32.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_22x22.png" ) );
	appIcon.addFile( QStringLiteral( ":/img/icon_16x16.png" ) );
	app.setWindowIcon( appIcon );

	QTranslator appTranslator;
	appTranslator.load( QStringLiteral( "./tr/gif-editor_" ) + QLocale::system().name() );
	app.installTranslator( &appTranslator );

	MainWindow w;
	w.resize( 800, 600 );
	w.show();

	return app.exec();
}
