#ifndef SHAPEDSTRINGBUFFER_H
#define SHAPEDSTRINGBUFFER_H 

#include <QObject>

class ShapedStringBuffer : public QObject
{
	Q_OBJECT
public:
	ShapedStringBuffer(QObject *parent = 0);
	~ShapedStringBuffer();

	void createShapedString(const wchar_t *pString);

	void clearShapedString();

	bool isValid();

	wchar_t *shapedString();

private:
	wchar_t						*m_pShapedString;
};

#endif // SHAPEDSTRINGBUFFER_H 