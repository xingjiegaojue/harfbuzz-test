#include "test.h"

#include <QVarLengthArray>
#include <QFile>
#include <QString>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <harfbuzz-shaper.h>
#include <harfbuzz-global.h>
#include <harfbuzz-gpos.h>

#ifdef NO_OPENTYPE
#include <QCoreApplication>

#include "shapedstringbuffer.h"

static ShapedStringBuffer *s_shapedStringBuffer = new ShapedStringBuffer(qApp);
#endif // NO_OPENTYPE

static FT_Library freetype;

static FT_Face loadFace(const char *name)
{
	FT_Face face;
	char path[256];

	//strcpy(path, SRCDIR);
	strcpy(path, ".");
	strcat(path, "/fonts/");
	strcat(path, name);

	if (FT_New_Face(freetype, path, /*index*/0, &face))
		return 0;
	return face;
}

static HB_UChar32 getChar(const HB_UChar16 *string, hb_uint32 length, hb_uint32 &i)
{
	HB_UChar32 ch;
	if (HB_IsHighSurrogate(string[i])
		&& i < length - 1
		&& HB_IsLowSurrogate(string[i + 1])) {
			ch = HB_SurrogateToUcs4(string[i], string[i + 1]);
			++i;
	} else {
		ch = string[i];
	}
	return ch;
}

static HB_Bool hb_stringToGlyphs(HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool /*rightToLeft*/)
{
	FT_Face face = (FT_Face)font->userData;
	if (length > *numGlyphs)
		return false;

#ifdef NO_OPENTYPE
	wchar_t *pWCBuffer = new wchar_t[length + 1];
	memset(pWCBuffer,0,sizeof(wchar_t) * (length + 1));
#endif // NO_OPENTYPE

	int glyph_pos = 0;
	for (hb_uint32 i = 0; i < length; ++i) {
		HB_UChar32 code = getChar(string, length, i);

		glyphs[glyph_pos] = FT_Get_Char_Index(face, code);
		++glyph_pos;

#ifdef NO_OPENTYPE
		pWCBuffer[i] = code;
#endif // NO_OPENTYPE
	}

	*numGlyphs = glyph_pos;

#ifdef NO_OPENTYPE
	s_shapedStringBuffer->createShapedString(pWCBuffer);
	delete pWCBuffer;
#endif // NO_OPENTYPE

	return true;
}

static void hb_getAdvances(HB_Font font, const HB_Glyph *glyphs, hb_uint32 numGlyphs, HB_Fixed *advances, int flags)
{
	FT_Face face = ( FT_Face ) font->userData;
	for ( hb_uint32 i = 0; i < numGlyphs; ++i ) {
		// 		qDebug() << "\tLoad index "<< i;
		// 		qDebug() << "\tWhich is glyph "<<glyphs[i];
		FT_Load_Glyph ( face, glyphs[i],FT_LOAD_NO_SCALE );
		// 		qDebug() << "ADV("<< glyphs[i] <<")("<< face->glyph->metrics.horiAdvance <<")";
		advances[i] = face->glyph->metrics.horiAdvance;
	}

	//for (hb_uint32 i = 0; i < numGlyphs; ++i)
	//	advances[i] = 0; // ### not tested right now
}

static HB_Bool hb_canRender(HB_Font font, const HB_UChar16 *string, hb_uint32 length)
{
	FT_Face face = (FT_Face)font->userData;

	for (hb_uint32 i = 0; i < length; ++i)
		if (!FT_Get_Char_Index(face, getChar(string, length, i)))
			return false;

	return true;
}

static HB_Error hb_getSFntTable(void *font, HB_Tag tableTag, HB_Byte *buffer, HB_UInt *length)
{
	FT_Face face = (FT_Face)font;
	FT_ULong ftlen = *length;
	FT_Error error = 0;

	if (!FT_IS_SFNT(face))
		return HB_Err_Invalid_Argument;

	error = FT_Load_Sfnt_Table(face, tableTag, 0, buffer, &ftlen);
	*length = ftlen;
	return (HB_Error)error;
}

HB_Error hb_getPointInOutline(HB_Font font, HB_Glyph glyph, int flags, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos, hb_uint32 *nPoints)
{
	HB_Error error = HB_Err_Ok;
	FT_Face face = (FT_Face)font->userData;

	int load_flags = (flags & HB_ShaperFlag_UseDesignMetrics) ? FT_LOAD_NO_HINTING : FT_LOAD_DEFAULT;

	if ((error = (HB_Error)FT_Load_Glyph(face, glyph, load_flags)))
		return error;

	if (face->glyph->format != ft_glyph_format_outline)
		return (HB_Error)HB_Err_Invalid_SubTable;

	*nPoints = face->glyph->outline.n_points;
	if (!(*nPoints))
		return HB_Err_Ok;

	if (point > *nPoints)
		return (HB_Error)HB_Err_Invalid_SubTable;

	*xpos = face->glyph->outline.points[point].x;
	*ypos = face->glyph->outline.points[point].y;

	return HB_Err_Ok;
}

void hb_getGlyphMetrics(HB_Font, HB_Glyph, HB_GlyphMetrics *metrics)
{
	// ###
	metrics->x = metrics->y = metrics->width = metrics->height = metrics->xOffset = metrics->yOffset = 0;
}

HB_Fixed hb_getFontMetric(HB_Font, HB_FontMetric )
{
	return 0; // ####
}

const HB_FontClass hb_fontClass = {
	hb_stringToGlyphs, hb_getAdvances, hb_canRender,
	hb_getPointInOutline, hb_getGlyphMetrics, hb_getFontMetric
};


//TESTED_CLASS=
//TESTED_FILES= gui/text/qscriptengine.cpp


tst_QScriptEngine::tst_QScriptEngine()
{
}

tst_QScriptEngine::~tst_QScriptEngine()
{
}

void tst_QScriptEngine::initTestCase()
{
	FT_Init_FreeType(&freetype);
}

void tst_QScriptEngine::cleanupTestCase()
{
	FT_Done_FreeType(freetype);
}

class Shaper
{
public:
	Shaper(FT_Face face, HB_Script script, const QString &str);

	HB_FontRec hbFont;
	HB_ShaperItem shaper_item;
	QVarLengthArray<HB_Glyph> hb_glyphs;
	QVarLengthArray<HB_GlyphAttributes> hb_attributes;
	QVarLengthArray<HB_Fixed> hb_advances;
	QVarLengthArray<HB_FixedPoint> hb_offsets;
	QVarLengthArray<unsigned short> hb_logClusters;
};

Shaper::Shaper(FT_Face face, HB_Script script, const QString &str)
{
	HB_Face hbFace = HB_NewFace(face, hb_getSFntTable);

	hbFont.klass = &hb_fontClass;
	hbFont.userData = face;
	hbFont.x_ppem  = face->size->metrics.x_ppem;
	hbFont.y_ppem  = face->size->metrics.y_ppem;
	hbFont.x_scale = face->size->metrics.x_scale;
	hbFont.y_scale = face->size->metrics.y_scale;

	shaper_item.kerning_applied = false;
	shaper_item.string = reinterpret_cast<const HB_UChar16 *>(str.constData());
	shaper_item.stringLength = str.length();
	shaper_item.item.script = script;
	shaper_item.item.pos = 0;
	shaper_item.item.length = shaper_item.stringLength;
	shaper_item.item.bidiLevel = 0; // ###
	shaper_item.shaperFlags = 0;
	shaper_item.font = &hbFont;
	shaper_item.face = hbFace;
	shaper_item.num_glyphs = shaper_item.item.length;
	shaper_item.glyphIndicesPresent = false;
	shaper_item.initialGlyphCount = 0;


	while (1) {
		hb_glyphs.resize(shaper_item.num_glyphs);
		hb_attributes.resize(shaper_item.num_glyphs);
		hb_advances.resize(shaper_item.num_glyphs);
		hb_offsets.resize(shaper_item.num_glyphs);
		hb_logClusters.resize(shaper_item.num_glyphs);

		memset(hb_glyphs.data(), 0, hb_glyphs.size() * sizeof(HB_Glyph));
		memset(hb_attributes.data(), 0, hb_attributes.size() * sizeof(HB_GlyphAttributes));
		memset(hb_advances.data(), 0, hb_advances.size() * sizeof(HB_Fixed));
		memset(hb_offsets.data(), 0, hb_offsets.size() * sizeof(HB_FixedPoint));

		shaper_item.glyphs = hb_glyphs.data();
		shaper_item.attributes = hb_attributes.data();
		shaper_item.advances = hb_advances.data();
		shaper_item.offsets = hb_offsets.data();
		shaper_item.log_clusters = hb_logClusters.data();

		if (HB_ShapeItem(&shaper_item))
			break;
	}

	HB_FreeFace(hbFace);
}


void tst_QScriptEngine::arabic()
{
	FT_Face face = loadFace("ArialArabic.ttf");

	if (face) {
		//wchar_t wch[] = { 0x644,0x627,0 };
		//wchar_t wch[] = { 0x634,0x634,0x41,0x42,0x43,0x634,0x634,0 };
		wchar_t wch[] = { 0x0634,0x0644,0x0627,0x0624,0x064a,0x062b,0x0628,0x0644,0x0041,0x0042,0x0043,0x0627,0x0646,0x062a,0x0645,0 };
		/*wchar_t wch[] = {
			0x0634,0x0635,0x062e,0x0647,0x0621,0x063a,0x0636,0x062d,0x0621,0x0641,0x0626,0x0645,0x0636,0x063a,0
		};*/
		QString str(QString::fromWCharArray(wch));

		HB_Glyph glyph = -1;
		HB_GlyphAttributes attr;
		HB_Fixed av = 0;
		HB_FixedPoint oft;
		unsigned short lc = 0;

		qDebug("glyph output");

		Shaper shaper(face, HB_Script_Arabic, str);
		for(int i = 0;i < shaper.shaper_item.num_glyphs;i++) {
			glyph = shaper.shaper_item.glyphs[i];
			attr = shaper.shaper_item.attributes[i];
			av = shaper.shaper_item.advances[i] / 64;
			oft = shaper.shaper_item.offsets[i];
			lc = shaper.shaper_item.log_clusters[i];

			qDebug(QString::number(glyph).toLocal8Bit().data());
		}

		FT_Done_Face(face);
	} else {
		QSKIP("couln't find DejaVu Sans", SkipAll);
	}
}


#ifdef NO_OPENTYPE
void tst_QScriptEngine::arabicUnicode()
{
	FT_Face face = loadFace("ArialArabic.ttf");

	if (face) {
		wchar_t wch[] = { 0x0634,0x0644,0x0627,0x0624,0x064a,0x062b,0x0628,0x0644,0x0041,0x0042,0x0043,0x0627,0x0646,0x062a,0x0645,0 };
		QString str(QString::fromWCharArray(wch));

		HB_Glyph glyph = -1;
		HB_GlyphAttributes attr;
		HB_Fixed av = 0;
		HB_FixedPoint oft;
		unsigned short lc = 0;

		Shaper shaper(face, HB_Script_Arabic, str);

		if(s_shapedStringBuffer->isValid()) {
			wchar_t *pRetString = s_shapedStringBuffer->shapedString();
			int len = wcslen(pRetString);

			qDebug("unicode output");

			for(int i = 0;i < len;i++) {
				qDebug(QString::number(pRetString[i]).toLocal8Bit().data());
			}

			s_shapedStringBuffer->clearShapedString();
		}

		FT_Done_Face(face);
	} else {
		QSKIP("couln't find DejaVu Sans", SkipAll);
	}
}
#endif // NO_OPENTYPE