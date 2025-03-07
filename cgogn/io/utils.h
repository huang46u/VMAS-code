/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * Copyright (C), IGG Group, ICube, University of Strasbourg, France            *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#ifndef CGOGN_IO_UTILS_H_
#define CGOGN_IO_UTILS_H_

#include <cgogn/core/utils/numerics.h>

#include <clocale>
#include <iostream>

namespace cgogn
{

namespace io
{

struct Scoped_C_Locale
{
	// set numeric locale to C after saving current locale
	inline Scoped_C_Locale()
	{
		current_locale_ = std::string(std::setlocale(LC_NUMERIC, nullptr));
		setlocale(LC_NUMERIC, "C");
	}
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(Scoped_C_Locale);

	// restore locale
	inline ~Scoped_C_Locale()
	{
		std::setlocale(LC_NUMERIC, current_locale_.c_str());
	}

private:
	std::string current_locale_;
};

inline std::istream& getline_safe(std::istream& is, std::string& str)
{
	str.clear();
	std::istream::sentry se(is, true); // http://en.cppreference.com/w/cpp/io/basic_istream/sentry
	std::streambuf* sb = is.rdbuf();

	while (true)
	{
		const auto c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF: // Also handle the case when the last line has no line ending
			if (str.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			str.push_back(static_cast<char>(c));
		}
	}
}

inline float64 read_double(std::istream& fp, std::string& line)
{
	fp >> line;
	while (line[0] == '#')
	{
		fp.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		fp >> line;
	}
	return std::stod(line);
}

inline uint32 read_uint(std::istream& fp, std::string& line)
{
	fp >> line;
	while (line[0] == '#')
	{
		fp.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		fp >> line;
	}
	return uint32((std::stoul(line)));
}

} // namespace io

} // namespace cgogn

#endif // CGOGN_IO_UTILS_H_
