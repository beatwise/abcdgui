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

#include <cairomm/cairomm.h>
#include <cairomm/enums.h>

namespace abcd {

	struct point
	{
		int x, y;
	};

	struct size
	{
		int width, height;
	};

	struct rect
	{
		int x1, y1, x2, y2;
		int width() {return x2 - x1;}
		int height() {return y2 - y1;}

		rect()
		{
			x1 = y1 = x2 = y2 = 0;
		}

		rect(int x1, int y1, int x2, int y2)
		{
			this->x1 = x1; this->y1 = y1; 
			this->x2 = x2; this->y2 = y2;
		}
	};


struct color
{
	uint8_t r, g, b, a;
};

class Draw
{
	::Cairo::RefPtr<::Cairo::Context> m_cr;
	::Cairo::RefPtr<::Cairo::ImageSurface> m_surface;

	void prepare()
	{
		m_cr->translate(0.5, 0.5);
	}

	void create_rounded_rectangle(rect r, int rx, int ry)
	{
		double s = ry / double(rx);

		rect ri;
		ri.x1 = r.x1 + rx;
		ri.y1 = r.y1 + ry;
		ri.x2 = r.x2 - rx;
		ri.y2 = r.y2 - ry;

		m_cr->begin_new_sub_path();

		m_cr->save();
		m_cr->translate(ri.x2, ri.y1);
		m_cr->scale(1, s);
		m_cr->arc(0, 0, rx, -M_PI / 2, 0);
		m_cr->restore();

		m_cr->save();
		m_cr->translate(ri.x2, ri.y2);
		m_cr->scale(1, s);
		m_cr->arc(0, 0, rx, 0, M_PI / 2);
		m_cr->restore();

		m_cr->save();
		m_cr->translate(ri.x1, ri.y2);
		m_cr->scale(1, s);
		m_cr->arc(0, 0, ry/s, M_PI / 2, M_PI);
		m_cr->restore();

		m_cr->save();
		m_cr->translate(ri.x1, ri.y1);
		m_cr->scale(1, s);
		m_cr->arc(0, 0, ry/s, M_PI, 3 * M_PI / 2);
		m_cr->restore();

		m_cr->close_path();

	}	


public:

	Draw(uint8_t *pixels, int w, int h)
	{
		m_surface = ::Cairo::ImageSurface::create(
			pixels, ::Cairo::Format::FORMAT_ARGB32, w, h, w * 4) ;

		m_cr = ::Cairo::Context::create(m_surface);

		prepare();
	}

	void set_stroke_width(float width)
	{
		m_cr->set_line_width(width);
	}

	void set_solid_paint(color c)
	{
		m_cr->set_source_rgba(c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
	}

	void clear()
	{
		m_cr->paint();
	}

	void stroke_rectangle(rect r)
	{
		m_cr->rectangle(r.x1, r.y1, r.width(), r.height());
		m_cr->stroke();
	}

	void fill_rectangle(rect r)
	{
		m_cr->rectangle(r.x1, r.y1, r.width(), r.height());
		m_cr->fill();
	}


	void stroke_rounded_rectangle(rect r, int rx, int ry)
	{
		create_rounded_rectangle(r, rx, ry);
		m_cr->stroke();
	}

	void fill_rounded_rectangle(rect r, int rx, int ry)
	{
		create_rounded_rectangle(r, rx, ry);
		m_cr->fill();
	}

	void stroke_arc(rect r, int sa, int ea)
	{
		float xc = (r.x1 + r.x2) / 2;
		float yc = (r.y1 + r.y2) / 2;
		float w = r.width();
		float h = r.height();
		float radius = w / 2;

		if (w != h)
		{
			m_cr->save();
			m_cr->scale(1, h / w);
		}

		m_cr->begin_new_path();
		m_cr->arc(xc, yc * w / h, radius, sa * M_PI / 180, ea * M_PI / 180);
		m_cr->stroke();

		if (w != h)
		{
			m_cr->restore();
		}

	}

	void fill_arc(rect r, int sa, int ea)
	{
		float xc = (r.x1 + r.x2) / 2;
		float yc = (r.y1 + r.y2) / 2;
		float w = r.width();
		float h = r.height();
		float radius = w / 2;

		if (w != h)
		{
			m_cr->save();
			m_cr->scale(1, h / w);
		}

		m_cr->begin_new_path();
		m_cr->arc(xc, yc * w / h, radius, sa * M_PI / 180, ea * M_PI / 180);
		m_cr->fill();

		if (w != h)
		{
			m_cr->restore();
		}
	}





	float set_font(const char *family, float size) 
	{
		m_cr->select_font_face (family, ::Cairo::FONT_SLANT_NORMAL, ::Cairo::FONT_WEIGHT_NORMAL);
		m_cr->set_font_size(size);

		::Cairo::FontExtents fe;
		m_cr->get_font_extents(fe);
		
		float ratio = fe.height / size;

		return ratio;
	}

	float get_font_height() 
	{
		::Cairo::FontExtents fe;
		m_cr->get_font_extents(fe);
		
		return fe.height;
	}

	void text(const char *text, rect r, int xalign, int yalign) 
	{
		::Cairo::FontExtents fe;
		m_cr->get_font_extents(fe);

		::Cairo::TextExtents te;
		m_cr->get_text_extents(text, te);

		::Cairo::TextExtents tex;
		float xh = 0;
		for (char c = 33; c < 127; ++c)
		{
			char s[] = " ";
			s[0] = c;
			m_cr->get_text_extents(s, tex);
			xh = xh + tex.height;
		}

		xh = xh / (127-33.f);

		float x, y;

		switch (xalign)
		{
			case -1: x = r.x1; break;
			case  1: x = r.x2 - te.width; break;
			default: x = r.x1 + r.width() / 2 - te.width / 2; break;
		}

		x = x - te.x_bearing;

		switch (yalign)
		{
			case -1: y = r.y1 + fe.ascent; break;
			case  1: y = r.y2 - fe.descent; break;
			default: y = r.y1 + r.height() / 2 - xh / 2 + xh; break;
		}

		m_cr->move_to(x, y);
		m_cr->show_text(text);
	}

	void draw_textline(const char *text, point pt)
	{
		::Cairo::FontExtents fe;
		m_cr->get_font_extents(fe);

		::Cairo::TextExtents te;
		m_cr->get_text_extents(text, te);

		float x = pt.x /*+ te.x_bearing*/;
		float y = pt.y + fe.ascent;

		m_cr->move_to(x, y);
		m_cr->show_text(text);
	}

	size get_textline_size(const char *text)
	{
		::Cairo::FontExtents fe;
		m_cr->get_font_extents(fe);

		::Cairo::TextExtents te;
		m_cr->get_text_extents(text, te);

		return {int(te.x_bearing + te.x_advance), int(fe.ascent + fe.descent)};
	}

	void push()
	{
		m_cr->save();
	}

	void pop()
	{
		m_cr->restore();
	}

	void clip(rect r)
	{
		m_cr->rectangle(r.x1, r.y1, r.width(), r.height());
		m_cr->clip();
	}

	void translate(point pt)
	{
		m_cr->translate(pt.x, pt.y);
	}

	void rotate(float degree)
	{
		m_cr->rotate(degree * M_PI / 180);
	}

};


} // abcd
