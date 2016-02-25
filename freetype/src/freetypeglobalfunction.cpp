#include "freetypeglobalfunction.h"

#include <QFile>
#include <QTextCodec>
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include <freetype/ttnameid.h>
#include <freetype/ftstroke.h>

QStringList familyNameFromPath(const QString &filePath)
{
	if(!QFile::exists(filePath)) {
		return QStringList();
	}

	FT_Library library;
	FT_Face face;

    /* Initialize library */
	if(FT_Init_FreeType(&library)) {
		return QStringList();
	}

    /* Load face */
	QByteArray pathBytes = filePath.toLocal8Bit();
	if(FT_New_Face(library,pathBytes.data(),0,&face)) {
		FT_Done_FreeType(library);
		return QStringList();
	}

	int numOfFaces = face->num_faces;
	int faceIndex = 0;
	QStringList faceNameList;

	do {
		// default
		QString fontName(face->family_name);

		int count = FT_Get_Sfnt_Name_Count(face);
		FT_SfntName fnm = { 0 };

		for(int i = 0;i < count;i++) {
			FT_Get_Sfnt_Name(face,i,&fnm);

			/*
			Platform-specific ID code	Meaning
			0 							Symbol
			1 							Unicode BMP-only (UCS-2)
			2 							Shift-JIS
			3 							PRC
			4 							BigFive
			5 							Johab
			10 							Unicode UCS-4
			*/
			if((fnm.platform_id == TT_PLATFORM_MICROSOFT) && \
				(fnm.name_id == TT_NAME_ID_FONT_FAMILY) && \
				((fnm.language_id == TT_MS_LANGID_CHINESE_GENERAL) || (fnm.language_id == TT_MS_LANGID_CHINESE_TAIWAN) || \
				(fnm.language_id == TT_MS_LANGID_CHINESE_PRC) || (fnm.language_id == TT_MS_LANGID_CHINESE_HONG_KONG) || \
				(fnm.language_id == TT_MS_LANGID_CHINESE_SINGAPORE) || (fnm.language_id == TT_MS_LANGID_CHINESE_MACAU))) {

					if(fnm.encoding_id == TT_MS_ID_UNICODE_CS) {
						QChar fmName[128];
						for(int j = 0;j < (fnm.string_len / 2);j++) {
							int index = j * 2;
							fmName[j] = QChar((fnm.string[index] << 8) | (fnm.string[index + 1]));
						}

						fmName[fnm.string_len / 2] = QChar('\0');
						fontName = QString(fmName);

						break;
					}

					else if(fnm.encoding_id == TT_MS_ID_GB2312) {
						QTextCodec *codec = QTextCodec::codecForLocale();
						if(codec) {
							fontName = codec->toUnicode(QByteArray((const char *)fnm.string,fnm.string_len));
							break;
						}
					}
			}
		} 

		FT_Done_Face(face);

		faceNameList << fontName;
		faceIndex++;

		if(faceIndex < numOfFaces) {
			if(FT_New_Face(library,pathBytes.data(),faceIndex,&face)) {
				break;
			}
		} else {
			break;
		}
	} while(1);

	FT_Done_FreeType(library);

	return faceNameList;
}


int faceIndexFromName(const QString &filePath,const QString &name)
{
	return familyNameFromPath(filePath).indexOf(name);
}