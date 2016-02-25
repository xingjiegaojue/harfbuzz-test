#ifndef FREETYPEFONT_H
#define FREETYPEFONT_H

#include <QObject>
#include <QPainterPath>

#include <ft2build.h>
#include <freetype/freetype.h>

class FreeTypeFont : public QObject
{
	Q_OBJECT
public:
	FreeTypeFont(QObject *parent = 0);
	~FreeTypeFont();

	void reset();

	void loadFace(const QString &filePath,int faceIndex = 0);

	QPainterPath path();

	QString name() const;
	void setName(const QString &nm);

	bool bold() const;
	void setBold(bool flag);

	bool italic() const;
	void setItalic(bool flag);

	int ascent() const;

	int descent() const;

	void setCharacter(ushort code);

private:
	ushort convertCharCode();

	QString				m_name;
	ushort				m_character;
	bool				m_bold;
	bool				m_italic;

	int					m_ascent;
	int					m_descent;

	FT_Library			m_library;
	FT_Face				m_face;
	FT_Encoding			m_encoding;
};

#endif // FREETYPEFONT_H