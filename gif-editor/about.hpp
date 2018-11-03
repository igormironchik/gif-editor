
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

#ifndef GIF_EDITOR_ABOUT_HPP_INCLUDED
#define GIF_EDITOR_ABOUT_HPP_INCLUDED

// Qt include.
#include <QWidget>


//
// About
//

//! Widget about.
class About final
	:	public QWidget
{
	Q_OBJECT

public:
	explicit About( QWidget * parent );
	~About() noexcept override = default;
}; // class About

#endif // GIF_EDITOR_ABOUT_HPP_INCLUDED
