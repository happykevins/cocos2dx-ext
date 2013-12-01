/****************************************************************************
 Copyright (c) 2013 Kevin Sun and RenRen Games

 email:happykevins@gmail.com
 http://wan.renren.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "CCRichCache.h"
#include "CCRichElement.h"

NS_CC_EXT_BEGIN;

//////////////////////////////////////////////////////////////////////////

RCacheBase::RCacheBase()
	: m_rHAlign(e_align_left)
	, m_rVAlign(e_align_bottom)
	, m_rLineHeight(0)
	, m_rSpacing(0)
	, m_rPadding(0)
	, m_rWrapLine(true)
{

}

RRect RLineCache::flush(class IRichCompositor* compositor)
{
	RRect line_rect;

	// no element yet, need not flush!
	element_list_t* line = getCachedElements();
	if ( line->size() == 0 )
		return line_rect;

	// line mark
	std::vector<element_list_t::iterator> line_marks;
	std::vector<short> line_widths;

	RRect zone = compositor->getMetricsState()->zone;
	bool wrapline = m_rWrapLine;

	// line width auto growth
	if ( zone.size.w == 0 )
		wrapline = false;
	
	RMetricsState* mstate = compositor->getMetricsState();

	RPos pen;
	RRect temp_linerect;
	short base_line_pos_y = 0;
	element_list_t::iterator inner_start_it = line->begin();
	line_marks.push_back(line->begin()); // push first line start
	for ( element_list_t::iterator it = line->begin(); it != line->end(); it++ )
	{
		RMetrics* metrics = (*it)->getMetrics();

		// prev composit event
		(*it)->onCachedCompositBegin(this, pen);

		// calculate baseline offset
		short baseline_correct = 0;
		if ( (*it)->needBaselineCorrect() )
		{
			baseline_correct = m_rBaselinePos;
		}

		// first element
		if ( pen.x == 0 )
		{
			pen.x -= metrics->rect.min_x();
		}
		
		// set position
		(*it)->setLocalPositionX(pen.x);
		(*it)->setLocalPositionY(pen.y + baseline_correct);

		RRect rect = metrics->rect;
		rect.pos.x += pen.x;
		rect.pos.y += baseline_correct;
		temp_linerect.extend(rect);

		// process wrapline
		element_list_t::iterator next_it = it + 1;
		if ( next_it == line->end() ||	// last element
			(*next_it)->isNewlineBefore() || // line-break before next element
			(*it)->isNewlineFollow() ||	// line-break after this element
			( wrapline && pen.x != 0		// wrap line
			&& pen.x + metrics->advance.x + (*next_it)->getMetrics()->rect.pos.x + (*next_it)->getMetrics()->rect.size.w + getPadding()*2 > zone.size.w 
			&& (*next_it)->canLinewrap() ) )
		{
			// correct out of bound correct
			short y2correct = -temp_linerect.max_y();

			for ( element_list_t::iterator inner_it = inner_start_it; inner_it != next_it; inner_it++ )
			{
				RPos pos = (*inner_it)->getLocalPosition();
				(*inner_it)->setLocalPositionY(pos.y + y2correct);
				(*inner_it)->setLocalPositionX(pos.x /*+ x2correct*/);
			}

			temp_linerect.pos.y = pen.y;
			line_rect.extend(temp_linerect);

			pen.y -= (temp_linerect.size.h + getSpacing());
			pen.x = 0;

			// push next line start
			line_marks.push_back(next_it);
			line_widths.push_back(temp_linerect.size.w);

			inner_start_it = next_it;
			temp_linerect = RRect();
		}
		else
		{
			pen.x += metrics->advance.x;
		}
		
		// post composit event
		(*it)->onCachedCompositEnd(this, pen);
	}

	short align_correct_x = 0;
	size_t line_mark_idx = 0;
	if ( getHAlign() == e_align_left )
		line_rect.size.w += getPadding() * 2;
	else
		line_rect.size.w = RMAX(zone.size.w, line_rect.size.w + getPadding() * 2); // auto rect
	for ( element_list_t::iterator it = line->begin(); it != line->end(); it++ )
	{
		// prev composit event
		(*it)->onCachedCompositBegin(this, pen);

		if ( it == line_marks[line_mark_idx] )
		{
			short lwidth = line_widths[line_mark_idx];

			// x correct
			switch ( getHAlign() )
			{
			case e_align_left:
				align_correct_x = getPadding();
				break;
			case e_align_center:
				align_correct_x = ( line_rect.size.w - lwidth ) / 2;
				break;
			case e_align_right:
				align_correct_x = line_rect.size.w - lwidth - getPadding();
				break;
			}

			line_mark_idx++; // until next line
		}

		RPos pos = (*it)->getLocalPosition();
		(*it)->setLocalPositionX(mstate->pen_x + pos.x + align_correct_x);
		(*it)->setLocalPositionY(mstate->pen_y + pos.y);

		// post composit event
		(*it)->onCachedCompositEnd(this, pen);
	}

	line_rect.pos.y = mstate->pen_y;

	// advance pen position
	mstate->pen_y -= (line_rect.size.h + getSpacing());
	mstate->pen_x = 0;
	
	clear();

	return line_rect;
}

element_list_t* RLineCache::getCachedElements()
{
	return &m_rCachedLine;
}

void RLineCache::appendElement(IRichElement* ele)
{
	m_rCachedLine.push_back(ele);

	m_rBaselinePos = RMIN(ele->getBaseline(), m_rBaselinePos);
}

void RLineCache::clear()
{
	m_rCachedLine.clear();
	m_rBaselinePos = 0;
}


RLineCache::RLineCache()
	: m_rBaselinePos(0)
{

}


//////////////////////////////////////////////////////////////////////////

element_list_t* RHTMLTableCache::getCachedElements()
{
	return &m_rCached;
}
void RHTMLTableCache::clear()
{
	m_rCached.clear();
}
void RHTMLTableCache::appendElement(IRichElement* ele)
{
	m_rCached.push_back(ele);
}
RRect RHTMLTableCache::flush(class IRichCompositor* compositor)
{
	RRect table_rect;

	if ( m_rCached.empty())
	{
		return table_rect;
	}

	// table content size
	std::vector<short> row_heights;
	std::vector<short> col_widths;
	std::vector<bool> width_set;
	short max_row_width = 0;
	short max_row_height = 0;
	for ( element_list_t::iterator it = m_rCached.begin(); it != m_rCached.end(); it++ )
	{
		REleHTMLRow* row = dynamic_cast<REleHTMLRow*>(*it);
		if ( !row )
		{
			CCLog("[CCRich] Table cache can only accept 'REleHTMLRow' element!");
			continue;
		}

		short current_row_height = 0;
		std::vector<class REleHTMLCell*>& cells = row->getCells();
		for ( size_t i = 0; i < cells.size(); i++ )
		{
			CCAssert(i <= col_widths.size(), "");
			if ( i == col_widths.size() )
			{
				col_widths.push_back(cells[i]->getMetrics()->rect.size.w + getPadding() * 2);
				width_set.push_back(cells[i]->isWidthSet());
			}
			else
			{
				if (width_set[i])
				{
					if (cells[i]->isWidthSet())
					{
						col_widths[i] = RMAX(col_widths[i], cells[i]->getMetrics()->rect.size.w + getPadding() * 2);
					}
					else
					{
						// do nothing
					}
				}
				else
				{
					if (cells[i]->isWidthSet())
					{
						col_widths[i] = cells[i]->getMetrics()->rect.size.w + getPadding() * 2;
						width_set[i] = true;
					}
					else
					{
						// do nothing use the first row default width
						//col_widths[i] = RMIN(col_widths[i], cells[i]->getMetrics()->rect.size.w + getPadding() * 2);
					}
				}
			}

			current_row_height = RMAX(current_row_height, cells[i]->getMetrics()->rect.size.h);
		}

		current_row_height += getPadding() * 2;
		row_heights.push_back(current_row_height);

		table_rect.size.h += current_row_height;
	}

	// max width
	for ( size_t i = 0; i < col_widths.size(); i++ )
	{
		table_rect.size.w += col_widths[i];
	}

	// set content metrics
	short spacing = getSpacing();
	short pen_x = 0;
	short pen_y = -m_rTable->m_rBorder;
	size_t row_idx = 0;
	for ( element_list_t::iterator it = m_rCached.begin(); it != m_rCached.end(); it++ )
	{
		REleHTMLRow* row = dynamic_cast<REleHTMLRow*>(*it);
		if ( !row )
		{
			CCLog("[CCRich] Table cache can only accept 'REleHTMLRow' element!");
			continue;
		}

		pen_x = m_rTable->m_rBorder;

		// set row metrics
		row->setLocalPositionX(pen_x);
		row->setLocalPositionY(pen_y);
		RMetrics* row_metrics = row->getMetrics();
		row_metrics->rect.size.h = row_heights[row_idx];
		row_metrics->rect.size.w = table_rect.size.w + spacing * (col_widths.size() - 1);

		// process cells in row
		short cell_pen_x = 0;
		std::vector<class REleHTMLCell*>& cells = row->getCells();
		for ( size_t i = 0; i < cells.size(); i++ )
		{
			cells[i]->setLocalPositionX(cell_pen_x);
			cells[i]->setLocalPositionY(0);
			RMetrics* cell_metrics = cells[i]->getMetrics();
			cell_metrics->rect.size.w = col_widths[i];
			cell_metrics->rect.size.h = row_heights[row_idx];

			recompositCell(cells[i]);

			cell_pen_x += col_widths[i];
			cell_pen_x += spacing;
		}

		pen_y -= row_heights[row_idx];
		pen_y -= spacing;
        row_idx++;
	}

	table_rect.size.h += m_rTable->m_rBorder * 2 + spacing * (row_heights.size() - 1);
	table_rect.size.w += m_rTable->m_rBorder * 2 + spacing * (col_widths.size() - 1);

	m_rCached.clear();

	return table_rect;
}

void RHTMLTableCache::travesalRecompositChildren(element_list_t* eles, short x_fixed, short y_fixed)
{
	for( element_list_t::iterator it = eles->begin(); it != eles->end(); it++ )
	{
		// travesal children
		if ( !(*it)->pushMetricsState() )
		{
			element_list_t* eles = (*it)->getChildren();
			if ( eles && !eles->empty() )
				travesalRecompositChildren(eles, x_fixed, y_fixed);
		}

		RPos pos = (*it)->getLocalPosition();
		(*it)->setLocalPositionX(pos.x + x_fixed);
		(*it)->setLocalPositionY(pos.y + y_fixed);
	}
}

void RHTMLTableCache::recompositCell(class REleHTMLCell* cell)
{
	RSize content_size = cell->m_rContentSize.size;
	RSize zone_size = cell->getMetrics()->rect.size;

	short padding = getPadding();
	short x_fixed = 0;
	short y_fixed = 0;
	EAlignment halign = cell->m_rHAlignSpecified ? 
		cell->m_rHAlignment : 
		( cell->m_rRow->m_rHAlignSpecified ? cell->m_rRow->m_rHAlignment : m_rHAlign);
	EAlignment valign = cell->m_rVAlignSpecified ? 
		cell->m_rVAlignment :
		( cell->m_rRow->m_rVAlignSpecified ? cell->m_rRow->m_rVAlignment : m_rVAlign);

	switch ( halign )
	{
	case e_align_left:
		x_fixed = 0 + padding;
		break;
	case e_align_center:
		x_fixed = ( zone_size.w - content_size.w ) / 2;
		break;
	case e_align_right:
		x_fixed = zone_size.w - content_size.w - padding;
		break;
	}

	switch ( valign )
	{
	case e_align_top:
		y_fixed = 0 - padding;
		break;
	case e_align_middle:
		y_fixed = -( zone_size.h - content_size.h ) / 2;
		break;
	case e_align_bottom:
		y_fixed = -(zone_size.h - content_size.h) + padding;
		break;
	}

	//x_fixed = RMIN( RMAX(x_fixed, 0), (zone_size.w - content_size.w) );
	//y_fixed = RMAX( RMIN(y_fixed, 0), -(zone_size.h - content_size.h) );

	element_list_t* eles = cell->getChildren();
	if ( eles && !eles->empty() )
		travesalRecompositChildren(eles, x_fixed, y_fixed);
}

void RHTMLTableCache::setTable(class REleHTMLTable* table)
{
	m_rTable = table;
}

RHTMLTableCache::RHTMLTableCache()
	: m_rTable(NULL)
{

}

NS_CC_EXT_END;
