#include "freetypefont.h"
#include "codeconvert.h"

#include <QFile>
#include <QByteArray>

#include <freetype/ttnameid.h>
#include <freetype/ftoutln.h>


static float int26p6_to_float(int value)
{
	return ((double)value / 64);
}


FreeTypeFont::FreeTypeFont(QObject *parent) : 
	QObject(parent),
	m_library(NULL),
	m_face(NULL)
{
	reset();
}


FreeTypeFont::~FreeTypeFont()
{
	reset();
}


void FreeTypeFont::reset()
{
	m_name.clear();
	m_character = 0;
	m_bold = false;
	m_italic = false;

	m_ascent = 0;
	m_descent = 0;

	if(m_face) {
		FT_Done_Face(m_face);
		m_face = NULL;
	}

	if(m_library) {
		FT_Done_FreeType(m_library);
		m_library = NULL;
	}

	m_encoding = FT_ENCODING_NONE;
}


void FreeTypeFont::loadFace(const QString &filePath,int faceIndex)
{
	if(!QFile::exists(filePath) || (faceIndex < 0)) {
		return;
	}

	reset();

	/* Initialize library */
	if(FT_Init_FreeType(&m_library)) {
		return;
	}

	/* Load face */
	if(FT_New_Face(m_library,filePath.toLocal8Bit().data(),faceIndex,&m_face)) {
		FT_Done_FreeType(m_library);
		m_library = NULL;
		return;
	}

	/* Select charmap */
	for(int i = 0;i < m_face->num_charmaps;i++) {
		FT_CharMap cm = m_face->charmaps[i];
		FT_Encoding newEncode = cm->encoding;

		if((cm->platform_id == TT_PLATFORM_MICROSOFT) && \
			((newEncode == FT_ENCODING_MS_SYMBOL) || \
			(newEncode == FT_ENCODING_UNICODE) || \
			(newEncode == FT_ENCODING_GB2312) || \
			(newEncode == FT_ENCODING_BIG5))) {

			m_encoding = newEncode;
			break;
		}
	}

	if(m_encoding != FT_ENCODING_NONE) {
		FT_Select_Charmap(m_face,m_encoding);
	}

	/* Set char pixel size */
	FT_Set_Pixel_Sizes(m_face,96,96);

	FT_Size pSize = m_face->size;
	FT_Size_Metrics_ font_Size_Metrics = pSize->metrics;
	m_ascent = (font_Size_Metrics.ascender + 63)>>6;
	m_descent = (font_Size_Metrics.descender + 63)>>6;
}


ushort FreeTypeFont::convertCharCode()
{
	// default
	ushort code = m_character;

	// GB2312
	if(m_encoding == FT_ENCODING_GB2312) {
		if(unicodeToGB2312(m_character,&code)) {
			return 0;
		}
	}

	// BIG5
	else if(m_encoding == FT_ENCODING_BIG5) {
		if(utf16ToBig5(m_character,&code)) {
			return 0;
		}
	}

	// MS Symbol
	else if(m_encoding == FT_ENCODING_MS_SYMBOL) {
		code = 0xF000 + m_character;
	}

	return code;
}


QPainterPath FreeTypeFont::path()
{
	if(!m_library || !m_face) {
		return QPainterPath();
	}

	FT_UInt glyph_index = FT_Get_Char_Index(m_face,convertCharCode());
	if(!glyph_index) {
		return QPainterPath();
	}

	FT_Int flags = FT_LOAD_NO_BITMAP | FT_LOAD_DEFAULT;
	FT_Load_Glyph(m_face,glyph_index,flags);

	FT_GlyphSlot pGlyphSlot = m_face->glyph;
	FT_Outline *outline = &pGlyphSlot->outline;

	// Slightly em-bold a glyph without touching its metrics
	if(m_bold) {
		int str = FT_MulFix(pGlyphSlot->face->units_per_EM,pGlyphSlot->face->size->metrics.y_scale) / 64;
		FT_Outline_Embolden(outline,str);
	}

	// Coefficients are in 16.16 fixed float format
	if(m_italic) {
		FT_Matrix matrix;
		matrix.xx = (0x1 << 16);
		matrix.xy = 0x5800;
		matrix.yx = 0;
		matrix.yy = (0x1 << 16);
		FT_Outline_Transform(outline,&matrix);
	}

	QPainterPath outlinePath;

	float x1,y1,x2,y2,x3,y3;
	int first = 0;
	bool isErr = false;

	for(int n = 0;(n < outline->n_contours) && !isErr;n++) {
		int last = outline->contours[n];
		FT_Vector *limit = outline->points + last;
		FT_Vector v_start = outline->points[first];
		FT_Vector v_last = outline->points[last];
		FT_Vector v_control = v_start;
		FT_Vector *point = outline->points + first;
		char *tags = outline->tags + first;

		char tag = FT_CURVE_TAG(tags[0]);

		// A contour cannot start with a cubic control point!
		if(tag == FT_CURVE_TAG_CUBIC) {
			isErr = true;
			continue;
		}

		// check first point to determine origin
		if(tag == FT_CURVE_TAG_CONIC) {
			// first point is conic control.
			if(FT_CURVE_TAG(outline->tags[last]) == FT_CURVE_TAG_ON) {
				// start at last point if it is on the curve
				v_start = v_last;
				limit--;
			} else {
				// if both first and last points are conic,
				// start at their middle and record its position
				// for closure
				v_start.x = (v_start.x + v_last.x) / 2;
				v_start.y = (v_start.y + v_last.y) / 2;

				v_last = v_start;
			}

			point--;
			tags--;
		}

		x1 = int26p6_to_float(v_start.x);
		y1 = int26p6_to_float(v_start.y);

		outlinePath.moveTo(x1,y1);

		while(point < limit) {
			point++;
			tags++;

			tag = FT_CURVE_TAG(tags[0]);
			switch(tag)
			{
			// emit a single lineTo
			case FT_CURVE_TAG_ON:  
				{
					x1 = int26p6_to_float(point->x); 
					y1 = int26p6_to_float(point->y);
					outlinePath.lineTo(x1,y1);
					continue;
				}

			// consume conic arcs
			case FT_CURVE_TAG_CONIC:  
				{
					v_control.x = point->x;
					v_control.y = point->y;

Do_Conic:
					if(point < limit) {
						FT_Vector vec;
						FT_Vector v_middle;

						point++;
						tags++;
						tag = FT_CURVE_TAG(tags[0]);

						vec.x = point->x; 
						vec.y = point->y;

						if(tag == FT_CURVE_TAG_ON) {
							x1 = int26p6_to_float(v_control.x); 
							y1 = int26p6_to_float(v_control.y);

							x2 = int26p6_to_float(vec.x); 
							y2 = int26p6_to_float(vec.y);

							outlinePath.quadTo(x1,y1,x2,y2);

							continue;
						}

						if(tag != FT_CURVE_TAG_CONIC) {
							isErr = true;
							break;
						}

						v_middle.x = (v_control.x + vec.x) / 2;
						v_middle.y = (v_control.y + vec.y) / 2;

						x1 = int26p6_to_float(v_control.x);
						y1 = int26p6_to_float(v_control.y);

						x2 = int26p6_to_float(v_middle.x); 
						y2 = int26p6_to_float(v_middle.y);

						outlinePath.quadTo(x1,y1,x2,y2);

						v_control = vec;

						goto Do_Conic;
					}

					x1 = int26p6_to_float(v_control.x); 
					y1 = int26p6_to_float(v_control.y);

					x2 = int26p6_to_float(v_start.x); 
					y2 = int26p6_to_float(v_start.y);

					outlinePath.quadTo(x1,y1,x2,y2);

					goto Close;
				}

			// FT_CURVE_TAG_CUBIC
			default:  
				{
					FT_Vector vec1, vec2;

					if(point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC) {
						isErr = true;
						break;
					}

					vec1.x = point[0].x;  
					vec1.y = point[0].y;

					vec2.x = point[1].x;  
					vec2.y = point[1].y;

					point += 2;
					tags  += 2;

					if(point <= limit) {
						FT_Vector vec;

						vec.x = point->x; 
						vec.y = point->y;

						x1 = int26p6_to_float(vec1.x); 
						y1 = int26p6_to_float(vec1.y);

						x2 = int26p6_to_float(vec2.x); 
						y2 = int26p6_to_float(vec2.y);

						x3 = int26p6_to_float(vec.x); 
						y3 = int26p6_to_float(vec.y);

						outlinePath.cubicTo(x1,y1,x2,y2,x3,y3);

						continue;
					}

					x1 = int26p6_to_float(vec1.x); 
					y1 = int26p6_to_float(vec1.y);

					x2 = int26p6_to_float(vec2.x); 
					y2 = int26p6_to_float(vec2.y);

					x3 = int26p6_to_float(v_start.x); 
					y3 = int26p6_to_float(v_start.y);

					outlinePath.cubicTo(x1,y1,x2,y2,x3,y3);

					goto Close;
				}
			}
		}

		outlinePath.closeSubpath();

Close:
		first = last + 1; 
	}

	if(isErr) {
		return QPainterPath();
	}

	return outlinePath;
}


QString FreeTypeFont::name() const
{
	return m_name;
}


void FreeTypeFont::setName(const QString &nm)
{
	if(m_name != nm) {
		m_name = nm;
	}
}


bool FreeTypeFont::bold() const
{
	return m_bold;
}


void FreeTypeFont::setBold(bool flag)
{
	if(m_bold != flag) {
		m_bold = flag;
	}
}


bool FreeTypeFont::italic() const
{
	return m_italic;
}


void FreeTypeFont::setItalic(bool flag)
{
	if(m_italic != flag) {
		m_italic = flag;
	}
}


int FreeTypeFont::ascent() const
{
	return m_ascent;
}


int FreeTypeFont::descent() const
{
	return m_descent;
}


void FreeTypeFont::setCharacter(ushort code)
{
	if(m_character != code) {
		m_character = code;
	}
}