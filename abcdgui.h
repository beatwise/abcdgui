/*
 * Copyright (c) 2021 Alessandro De Santis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <string>
#include <functional>

#include "abcddraw.h"

namespace abcd {


class theme
{
	std::vector<color> m_colors;
	std::string m_font_family;
	uint32_t m_font_size;

public:

	theme()
	{
		dark();
	}

	color bg() {return m_colors[0];}
	color back() {return m_colors[1];}
	color fore() {return m_colors[2];}
	color text() {return m_colors[3];}
	const char * font_family() {return m_font_family.c_str();}
	uint32_t font_size() {return m_font_size;}

	void set_colors(std::vector<color> colors)
	{
		m_colors = colors;
	}

	void set_font(const char * family, uint32_t size)
	{
		m_font_family = family;
		m_font_size = size;
	}


	void dark()
	{
		m_colors =
		{
			{51, 63, 85, 255},
			{20, 27, 43, 255},
			{49, 125, 250, 255},
			{225, 225, 225, 255},
		};

		m_font_family = "Roboto";
		m_font_size = 20;
	}

};


class grid;

// ---------------------------------------------------------
// HBOX
// ---------------------------------------------------------

class hbox 
{
	friend class grid;
	rect next;
	std::vector<int> xpos;
	int x1;
public:

	void create(rect bounds, uint32_t columns)
	{
		std::vector<uint32_t> weight;
		weight.resize(columns);
		std::fill(weight.begin(), weight.end(), 1);
		create(bounds, weight);
	}

	void create(rect bounds, const std::vector<uint32_t> weight)
	{
		xpos.clear();
		xpos.reserve(weight.size() + 1);
		double sum = 0;
		x1 = bounds.x1;
		double x = 0;

		xpos.push_back(x);

		for (auto w = weight.begin(); w != weight.end(); ++w) sum += *w;

		for (auto w = weight.begin(); w != weight.end(); ++w)
		{
			x = x + bounds.width() * *w / sum;
			xpos.push_back(int(0.5 + x));
		}


		next.y1 = bounds.y1;
		next.y2 = bounds.y2;
	}

	void create_fixed(rect bounds, const std::vector<int> extent)
	{
		xpos.clear();
		xpos.reserve(extent.size() + 1);
		double x = 0;
		x1 = bounds.x1;

		xpos.push_back(x);

		for (auto e = extent.begin(); e != extent.end(); ++e)
		{
			x = x + *e;
			xpos.push_back(x);
		}

		next.y1 = bounds.y1;
		next.y2 = bounds.y2;
	}
	
	rect cell(uint32_t i)
	{
		next.x1 = x1 + xpos[i];
		next.x2 = x1 + xpos[i + 1];
		return next;
	}
};

// ---------------------------------------------------------
// VBOX
// ---------------------------------------------------------

class vbox
{
	friend class grid;
	rect next;
	std::vector<int> ypos;
	int y1;
public:

	void create(rect bounds, uint32_t rows)
	{
		std::vector<uint32_t> weight;
		weight.resize(rows);
		std::fill(weight.begin(), weight.end(), 1);
		create(bounds, weight);
	}

	void create(rect bounds, const std::vector<uint32_t> weight)
	{
		ypos.clear();
		ypos.reserve(weight.size() + 1);
		double sum = 0;
		double y = 0;
		y1 = bounds.y1;

		ypos.push_back(y);

		for (auto w = weight.begin(); w != weight.end(); ++w) sum += *w;

		for (auto w = weight.begin(); w != weight.end(); ++w)
		{
			y = y + bounds.height() * *w / sum;
			ypos.push_back(int(0.5 + y));
		}


		next.x1 = bounds.x1;
		next.x2 = bounds.x2;
	}

	void create_fixed(rect bounds, const std::vector<int> extent)
	{
		ypos.clear();
		ypos.reserve(extent.size() + 1);
		double y = 0; 
		y1 = bounds.y1;

		ypos.push_back(y);

		for (auto e = extent.begin(); e != extent.end(); ++e)
		{
			y = y + *e;
			ypos.push_back(y);
		}

		next.y1 = bounds.y1;
		next.y2 = bounds.y2;
	}
	
	rect cell(uint32_t i)
	{
		next.y1 = y1 + ypos[i];
		next.y2 = y1 + ypos[i + 1];
		return next;
	}
};

// ---------------------------------------------------------
// GRID
// ---------------------------------------------------------

class grid
{
	hbox hb;
	vbox vb;
public:

	void create(rect bounds, int rows, int columns)
	{
		hb.create(bounds, columns);
		vb.create(bounds, rows);
	}

	void create(rect bounds, 
		const std::vector<uint32_t> rweight, const std::vector<uint32_t> cweight)
	{
		hb.create(bounds, cweight);
		vb.create(bounds, rweight);
	}

	void create_fixed(rect bounds, 
		const std::vector<int> rextent, const std::vector<int> cextent)
	{
		hb.create_fixed(bounds, cextent);
		vb.create_fixed(bounds, rextent);
	}
	
	rect cell(uint32_t r, uint32_t c)
	{
		rect next;
		next.x1 = hb.x1 + hb.xpos[c];
		next.x2 = hb.x1 + hb.xpos[c + 1];
		next.y1 = vb.y1 + vb.ypos[r];
		next.y2 = vb.y1 + vb.ypos[r + 1];
		return next;
	}
};

// ---------------------------------------------------------
// GRID
// ---------------------------------------------------------

struct span
{
	int padding1;
	int length;
	int padding2;
	int size() {return padding1 + length + padding2;}
};

// ---------------------------------------------------------
// GRID
// ---------------------------------------------------------

class guide
{
	int pos {0};

public:
	guide(int position = 0)
	{
		pos = position;
	}

	int position()
	{
		return pos;
	}


	void move(int position)
	{
		pos = position;
	}

	void shift(int delta)
	{
		pos = pos + delta;
	}

	void left(rect &r)
	{
		auto w = r.width();
		r.x1 = pos;
		r.x2 = pos + w;
	}

	void right(rect &r)
	{
		auto w = r.width();
		r.x1 = pos - w;
		r.x2 = pos;
	}

	void xcenter(rect &r)
	{
		auto w = r.width();
		r.x1 = pos - w / 2;
		r.x2 = r.x1 + w;
	}

	void top(rect &r)
	{
		auto h = r.height();
		r.y1 = pos;
		r.y2 = pos + h;
	}

	void bottom(rect &r)
	{
		auto h = r.height();
		r.y1 = pos - h;
		r.y2 = pos;
	}

	void ycenter(rect &r)
	{
		auto h = r.height();
		r.y1 = pos - h / 2;
		r.y2 = r.y1 + h;
	}

};


struct widget
{
	std::string name;
};

struct window
{
	theme m_theme;

	widget background;
	Draw *draw;

	bool mouse_down {false}; 
	uint32_t mouse_button {0}; 
	int mouse_x {-1};	
	int mouse_y {-1};
	widget *mouse_widget {nullptr};
	widget *focus_widget {nullptr};

	bool key_down {false};
	std::string key_utf8;

	window();
	void begin(Draw *draw);
	void end();
	void begin_widget(rect &r);
	void end_widget();

};

struct slider_widget : public widget
{
	int delta;
};

struct knob_widget : public widget
{
	float x1, y1, angle {0};
};

struct list_widget : public widget
{
	int yref;
	float yvalue {0};
	bool scrolling {false};
};


struct panel_widget : public widget
{
	rect r;
};

void move(rect &r, int x, int y);
bool contains(const rect &r, point pt);
void inflate(rect &r, int dx, int dy);
rect split(rect &r, int side, int size);

/**
 * creates and returns a scaled and aligned version of r
 */

rect adjust(rect &r, double sx, double sy, int xa = 0, int ya = 0);

/**
 * creates and returns a fixed size rectangle aligned with r
 */
rect adjust(rect &r, int w, int h, int xa = 0, int ya = 0);


/**
 * 
 */
rect pad(rect r, span h, span v);


////////////////////////////////////////////////////////////////
// WIDGETS

/**
 * 
 */

void label(window *, widget *, abcd::rect, std::string text, int xa = 0, int ya = 0);

/**
 * 
 */

bool button(window *win, widget *id, abcd::rect r, std::string text);

/**
 * 
 */

bool checkbutton(window *win, widget *id, abcd::rect r, bool *value);

/**
 * 
 */

bool radiobutton(window *win, widget *id, abcd::rect r, int index, int *value);

/**
 * 
 */

bool slider(window *win, slider_widget *id, abcd::rect r, int thumbsize, float *value, bool horz);

/**
 * 
 */

bool knob(window *win, knob_widget *id, abcd::rect r, float *value);

/**
 * 
 */

bool input(window *win, widget *id, abcd::rect r, std::string& value);

/**
 * 
 */

bool list(window *win, list_widget *id, abcd::rect r, const std::vector<std::string> &items, int &value);

/**
 * 
 */

abcd::rect begin_panel(window *win, panel_widget *id, abcd::rect);

/**
 * 
 */

abcd::rect end_panel(window *win, panel_widget *id);


} // abcd

