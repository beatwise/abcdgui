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

#include <algorithm>
#include <math.h>

#include "abcdgui.h"

namespace abcd {


// TODO: review x bearing usage


bool contains(const rect &r, point pt)
{
	bool b = r.x1 <= pt.x && pt.x < r.x2;
	return b && r.y1 <= pt.y && pt.y < r.y2;
}

void move(rect &r, int x, int y) 
{ 
	auto w = r.width(), h = r.height();
	r.x1 = x;
	r.y1 = y;
	r.x2 = x + w;
	r.y2 = y + h; 
}


void inflate(rect &r, int dx, int dy)
{
	r.x1 -= dx;
	r.y1 -= dy;
	r.x2 += dx;
	r.y2 += dy;
}

rect split(rect &r, int side, int size)
{
	rect r2 = r;
	switch (side)
	{
		case 0: // left
			if (size >= r.width())
			{
				r.x1 = r.x2;
			}
			else
			{
				r2.x2 = r2.x1 + size;
				r.x1 = r2.x2;
			}
			break;

		case 2: // right
			if (size >= r.width())
			{
				r.x2 = r.x1;
			}
			else
			{
				r2.x1 = r2.x2 - size;
				r.x2 = r2.x1;
			}
			break;

		case 1: // top
			if (size >= r.height())
			{
				r.y1 = r.y2;
			}
			else
			{
				r2.y2 = r2.y1 + size;
				r.y1 = r2.y2;
			}
			break;

		case 3: // bottom
			if (size >= r.height())
			{
				r.y2 = r.y1;
			}
			else
			{
				r2.y1 = r2.y2 - size;
				r.y2 = r2.y1;
			}
			break;
	}

	return r2;
}

rect adjust(rect &r, double sx, double sy, int xa, int ya)
{
	rect ro {0, 0, 0, 0};
	
	int w = r.width() * sx;
	int h = r.height() * sy;

	if (xa < 0)
	{
		ro.x1 = r.x1;
	}
	else if (xa > 0)
	{
		ro.x1 = r.x2 - w;
	}
	else
	{
		ro.x1 = r.x1 + r.width() / 2 - w / 2;
	}

	if (ya < 0)
	{
		ro.y1 = r.y1;
	}
	else if (ya > 0)
	{
		ro.y1 = r.y2 - h;
	}
	else
	{
		ro.y1 = r.y1 + r.height() / 2 - h / 2;
	}

	ro.x2 = ro.x1 + w;
	ro.y2 = ro.y1 + h;

	return ro;
}

rect adjust(rect &r, int w, int h, int xa, int ya)
{
	rect ro {0, 0, 0, 0};
	
	if (xa < 0)
	{
		ro.x1 = r.x1;
	}
	else if (xa > 0)
	{
		ro.x1 = r.x2 - w;
	}
	else
	{
		ro.x1 = r.x1 + r.width() / 2 - w / 2;
	}

	if (ya < 0)
	{
		ro.y1 = r.y1;
	}
	else if (ya > 0)
	{
		ro.y1 = r.y2 - h;
	}
	else
	{
		ro.y1 = r.y1 + r.height() / 2 - h / 2;
	}

	ro.x2 = ro.x1 + w;
	ro.y2 = ro.y1 + h;

	return ro;
}

rect pad(rect r, span h, span v)
{
	rect ro {0, 0, 0, 0};

	if (h.padding1 + h.padding2 < r.width() && v.padding1 + v.padding2 < r.height())
	{
		ro.x1 = r.x1 + h.padding1;
		ro.y1 = r.y1 + v.padding1;
		ro.x2 = r.x2 - h.padding2;
		ro.y2 = r.y2 - v.padding2;
	}

	return ro;
}



window::window()
{
}

void window::begin(Draw *draw)
{
	this->draw = draw;
}

void window::end()
{
	if (mouse_down && mouse_widget == nullptr)
		mouse_widget = &background;

	if (!mouse_down)
		mouse_widget = nullptr;

	key_down = false;
}

void window::begin_widget(rect &r)
{
	draw->push();
	draw->translate({r.x1, r.y1});
	move(r, 0, 0);
	draw->clip(r);
}

void window::end_widget()
{
	draw->pop();
}

// ----------------------------------------------------------------------------
// PANEL
// ----------------------------------------------------------------------------

abcd::rect begin_panel(window *win, panel_widget *id, abcd::rect r)
{
	id->r = r;
	win->begin_widget(r);

	win->mouse_x -= id->r.x1;
	win->mouse_y -= id->r.y1;

	return r;
}

rect end_panel(window *win, panel_widget *id)
{
	win->end_widget();
	win->mouse_x += id->r.x1;
	win->mouse_y += id->r.y1;

	return id->r;
}

// ----------------------------------------------------------------------------
// LABEL
// ----------------------------------------------------------------------------

void label(window *win, widget *id, abcd::rect r, std::string text, int xa, int ya)
{
	win->begin_widget(r);

	auto &t = win->m_theme;
	win->draw->set_font(t.font_family(), t.font_size());

	win->draw->set_solid_paint(win->m_theme.text());
	win->draw->text(text.c_str(), r, xa, ya);

	win->end_widget();
}

// ----------------------------------------------------------------------------
// BUTTON
// ----------------------------------------------------------------------------

bool button(window *win, widget *id, abcd::rect r, std::string text)
{
	bool clicked = 0;

	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
				clicked = true;
			}
		}
	}

	bool held = win->mouse_down && win->mouse_widget == id;


	auto &t = win->m_theme;
	win->draw->set_font(t.font_family(), t.font_size());

	if (held)
	{
		win->draw->set_solid_paint(win->m_theme.fore());
		win->draw->stroke_rounded_rectangle(r, 4, 4);

		win->draw->set_solid_paint(win->m_theme.text());
		win->draw->text(text.c_str(), r, 0, 0);
	}
	else
	{
		win->draw->set_solid_paint(win->m_theme.fore());
		win->draw->fill_rounded_rectangle(r, 4, 4);

		win->draw->set_solid_paint(win->m_theme.text());
		win->draw->text(text.c_str(), r, 0, 0);
	}

	win->end_widget();

	return clicked;
}

// ----------------------------------------------------------------------------
// CHECK BUTTON
// ----------------------------------------------------------------------------

bool checkbutton(window *win, widget *id, abcd::rect r, bool *value)
{
	bool clicked = false;

	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
				clicked = true;
				*value = !*value;
			}
		}
	}

	int a;
	abcd::rect ri;

	if (r.width() > r.height())
	{
		a = r.height();
		int radius = a * 0.8 / 2;
		int b = (a - radius*2) / 2; 
		int x1 = r.x1 + (!*value ? b : r.width() - b - 2 * radius);
		int y1 = r.y1 + b;
		ri = {x1, y1, x1 + 2*radius, y1 + 2*radius};
	}
	else
	{
		a = r.width();
		int radius = a * 0.8 / 2;
		int b = (a - radius*2) / 2; 
		int x1 = r.x1 + b;
		int y1 = r.y1 + (!*value ? b : r.height() - b - 2 * radius);
		ri = {x1, y1, x1 + 2*radius, y1 + 2*radius};
	}

	win->draw->set_solid_paint(win->m_theme.back());
	win->draw->fill_rounded_rectangle(r, a/2, a/2);

	win->draw->set_solid_paint(win->m_theme.fore());
	win->draw->fill_arc(ri, 0, 360);



	win->end_widget();

	return clicked;
}

// ----------------------------------------------------------------------------
// RADIO BUTTON
// ----------------------------------------------------------------------------

bool radiobutton(window *win, widget *id, abcd::rect r, int index, int *value)
{
	int clicked = 0;

	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
				clicked = 1;
			}
		}
	}

	bool changed = false;

	if (clicked)
	{
		changed = (*value != index);
		*value = index;
	}


	auto &t = win->m_theme;
	win->draw->set_font(t.font_family(), t.font_size());

	int size = std::min(r.width(), r.height());
	r = adjust(r, size, size);

	win->draw->set_solid_paint(win->m_theme.back());
	win->draw->fill_arc(r, 0, 360);

	if (*value == index)
	{
		inflate(r, -1, -1);
		win->draw->set_solid_paint(win->m_theme.fore());
		win->draw->fill_arc(r, 0, 360);
	}

	win->end_widget();

	return changed;
}

// ----------------------------------------------------------------------------
// SLIDER
// ----------------------------------------------------------------------------

bool slider(window *win, slider_widget *id, abcd::rect r, int thumbsize, float *value, bool horz)
{
	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	bool pressed = false;

	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
				pressed = true;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
			}
		}
	}

	float v = *value;
	v = std::max(0.f, v);
	v = std::min(1.f, v);

	auto thumb = r;

	int tracklen;
	int z;
	
	int *thumb_z1;
	int *thumb_z2;
	int mouse_z;
	int r_z1;
	int r_z2;
	int extent;

	if (horz)
	{
		thumb_z1 = &thumb.x1;
		thumb_z2 = &thumb.x2;
		mouse_z = mouse.x;
		r_z1 = r.x1;
		r_z2 = r.x2;
		extent = r.width();
	}
	else
	{
		thumb_z1 = &thumb.y1;
		thumb_z2 = &thumb.y2;
		mouse_z = mouse.y;
		r_z1 = r.y1;
		r_z2 = r.y2;
		extent = r.height();
	}


	thumbsize = std::min(extent, thumbsize);
	tracklen = extent - thumbsize;

	v = std::max(0.f, v);
	v = std::min(1.f, v);

	z = v * tracklen;

	*thumb_z1 = z;
	*thumb_z2 = z + thumbsize;

	if (pressed)
	{
		id->delta = mouse_z - *thumb_z1;

		if (id->delta < 0)
		{
			// BEFORE THUMB

			id->delta = thumbsize / 2;
			*thumb_z1 = mouse_z - id->delta;
			*thumb_z2 = *thumb_z1 + thumbsize;

			if (thumb_z1 < 0)
			{
				id->delta = mouse_z - r_z1;
				*thumb_z1 = mouse_z - id->delta;
				*thumb_z2 = *thumb_z1 + thumbsize;
			}

			v = float(*thumb_z1) / tracklen;
		}
		else if (id->delta >= thumbsize)
		{
			// AFTER THUMB

			id->delta = thumbsize / 2;
			*thumb_z1 = mouse_z - id->delta;
			*thumb_z2 = *thumb_z1 + thumbsize;

			if (*thumb_z2 > r_z2)
			{
				id->delta = thumbsize - (r_z2 - mouse_z);
				*thumb_z1 = mouse_z - id->delta;
				*thumb_z2 = *thumb_z1 + thumbsize;
			}

			v = float(*thumb_z1) / tracklen;
		}
	}
	else
	{
		if (win->mouse_widget == id)
		{
			z = std::min(tracklen, mouse_z - id->delta);
			z = std::max(0, z);

			*thumb_z1 = z;
			*thumb_z2 = z + thumbsize;

			v = float(z) / tracklen;
		}
	}

	win->draw->set_solid_paint(win->m_theme.back());
	win->draw->fill_rounded_rectangle(r, 3, 3);	

	win->draw->set_solid_paint(win->m_theme.fore());
	win->draw->fill_rounded_rectangle(thumb, 3, 3);

	win->end_widget();

	bool changed = v != *value;
	*value = v;	
	return changed;
}

// ----------------------------------------------------------------------------
// KNOB
// ----------------------------------------------------------------------------

bool knob(window *win, knob_widget *id, abcd::rect r, float *value)
{
	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	float v = *value;
	v = std::max(0.f, v);
	v = std::min(1.f, v);
	v = v * 270;

	id->angle = v;

	auto knob = r;

	int extent = r.width() < r.height() ? r.width() : r.height();
	
	knob.x1 = r.width() / 2 - extent / 2;
	knob.y1 = r.height() / 2 - extent / 2;
	knob.x2 = knob.x1 + extent;
	knob.y2 = knob.y1 + extent;


	bool pressed = false;

	if (contains(knob, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
				pressed = true;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
			}
		}
	}


	int xc = r.width() / 2;
	int yc = r.height() / 2;

	if (pressed) 
	{
		id->x1 = mouse.x - xc;
		id->y1 = mouse.y - yc;
	}
	else if (win->mouse_widget == id)
	{
		float x2 = mouse.x - xc;
		float y2 = mouse.y - yc;

		if (x2 != id->x1 || y2 != id->y1)
		{
			float v1 = sqrt(id->x1 * id->x1 + id->y1 * id->y1);
			float v2 = sqrt(x2 * x2 + y2 * y2);

			float dir = asin((id->x1 * y2 - x2 * id->y1) / (v1 * v2))* 180 / M_PI;
			float delta = acos((id->x1 * x2 + id->y1 * y2) / (v1 * v2)) * 180 / M_PI;

			id->x1 = x2;
			id->y1 = y2;

			id->angle += dir > 0 ? delta : -delta;

			id->angle = std::max(0.f, id->angle);
			id->angle = std::min(270.f, id->angle);

			v = id->angle;
		}
	}

	win->draw->set_solid_paint(win->m_theme.fore());
	win->draw->fill_arc(knob, 0, 360);


	win->draw->push();
	win->draw->translate({xc, yc});
	win->draw->rotate(id->angle+135);
	rect index {int(extent * 0.24), -2, int(extent * 0.45), 2};
	win->draw->set_solid_paint(win->m_theme.text());
	win->draw->fill_rectangle(index);
	win->draw->pop();

	win->end_widget();

	v = v / 270.f;



	bool changed = v != *value;
	*value = v;	
	return changed;
}


// ----------------------------------------------------------------------------
// INPUT
// ----------------------------------------------------------------------------

bool input(window *win, widget *id, abcd::rect r, std::string& value)
{
	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);


	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
				win->focus_widget = id;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
			}
		}
	}

	bool enter = false;

	if (win->focus_widget == id && win->key_down)
	{
		if (win->key_utf8[0] > 31)
			value = value + win->key_utf8;
		else if (win->key_utf8[0] == 8)
			value = value.substr(0, value.length()  - 1);
		else if (win->key_utf8[0] == 13)
			enter = true;

//		printf("CHAR %s\n", win->key_utf8.c_str());
	}

	auto &t = win->m_theme;
	win->draw->set_font(t.font_family(), t.font_size());


	win->draw->set_solid_paint(win->m_theme.back());


	win->draw->fill_rectangle(r);


	auto s = value.c_str();
	size m {0, 0};
	size_t i = 0;
	while (i < value.size())
	{
		m = win->draw->get_textline_size(s + i);
		if (m.width < r.width()) break;
		++i;
	}

	rect crsr {m.width, 0, m.width + 1, r.height()};

	win->draw->set_stroke_width(1);

	win->draw->set_solid_paint(win->m_theme.text());
	win->draw->draw_textline(value.substr(i).c_str(), {0, 0});

	if (win->focus_widget == id)
		win->draw->fill_rectangle(crsr);

	win->end_widget();

	return enter;
}

// ----------------------------------------------------------------------------
// LIST
// ----------------------------------------------------------------------------

bool list(window *win, list_widget *id, abcd::rect r, const std::vector<std::string> &items, int &value)
{
	point mouse = {win->mouse_x - r.x1, win->mouse_y - r.y1};

	win->begin_widget(r);

	enum btn_action {none, pressed, released};

	btn_action btn = none;

	if (contains(r, mouse))
	{
		if (win->mouse_widget == nullptr)
		{
			if (win->mouse_down)
			{
				win->mouse_widget = id;
				win->focus_widget = id;
				btn = pressed;
			}
		}
		else
		{
			if (!win->mouse_down && win->mouse_widget == id)
			{
				win->mouse_widget = nullptr;
				btn = released;
			}
		}
	}

	else
	{
		if (!win->mouse_down && win->mouse_widget == id)
		{
			win->mouse_widget = nullptr;
			btn = released;
		}
	}

	auto &t = win->m_theme;
	win->draw->set_font(t.font_family(), t.font_size());
	float height = win->draw->get_font_height();

	int view = ceil(r.height() / float(height)); 
	int doc = int(items.size())+1;
	int vbar = r.height();
	int nd, ns;
	float ratio;

	rect thumb_rect, scr;

	bool vbar_visible = doc > view;

	if (vbar_visible)
	{
		scr = split(r, 2, 16);

		int thumb = vbar * view / float(doc);
		if (thumb >= 12)
		{
			ratio = float(doc) / vbar;

			nd = doc - view;
			ns = vbar - thumb;
		}
		else
		{
			thumb = 12;

			nd = doc - view;
			ns = vbar - thumb;

			ratio = float(nd) / ns;
		}

		thumb_rect = scr;
		thumb_rect.y1 = int(id->yvalue / ratio);
		thumb_rect.y2 = thumb_rect.y1 + thumb;
	}

	if (vbar_visible && id->scrolling)
	{
		int new_y1 = mouse.y - id->yref;
		new_y1 = std::max(0, new_y1);
		new_y1 = std::min(ns, new_y1);
		id->yvalue = std::min(float(nd), new_y1 * ratio);
		new_y1 = int(id->yvalue / ratio);
		move(thumb_rect, thumb_rect.x1, new_y1);
	}

	int k = -ceil(id->yvalue);
	int yoffset = k * height;

	bool index_changed = false;

	if (btn == pressed)
	{
		if (vbar_visible && contains(scr, mouse))
		{
			if (contains(thumb_rect, mouse))
			{
				id->yref = mouse.y - thumb_rect.y1;
				id->scrolling = true;	
			}
		}
		else
		{
			int new_value = (-yoffset + mouse.y) / height;			

			if ((new_value < doc -1) && new_value != value)
			{
				value = new_value;
				index_changed = true;
			}
		}

	}
	else if (btn == released)
	{
		id->scrolling = false;
	}


	if (vbar_visible)
	{
		win->draw->set_solid_paint(win->m_theme.back());
		win->draw->fill_rectangle(scr);

		win->draw->set_solid_paint(win->m_theme.fore());
		win->draw->fill_rounded_rectangle(thumb_rect, 3, 3);
	}

	win->draw->set_solid_paint(win->m_theme.back());
	win->draw->fill_rectangle(r);

	win->draw->push();
	win->draw->clip(r);

	int y = yoffset;
	for (int i = 0; i < (int)items.size(); ++i)
	{
		if (i != value)
		{
			win->draw->set_solid_paint(win->m_theme.text());
			win->draw->draw_textline(items[i].c_str(), {0, y});
		}
		else
		{
			win->draw->set_solid_paint(win->m_theme.fore());
			win->draw->fill_rectangle({0+1, y+1, r.width()-1, int(y+height-1)});

			win->draw->set_solid_paint(win->m_theme.text());
			win->draw->draw_textline(items[i].c_str(), {0, y});
		}

		
		y += height;
	}

	win->draw->pop();

	win->end_widget();

	return index_changed;
}



} // abcd
