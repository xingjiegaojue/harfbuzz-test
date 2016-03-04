#include "shapedstringbuffer.h"

#include <string.h>

ShapedStringBuffer::ShapedStringBuffer(QObject *parent) : 
	QObject(parent),
	m_pShapedString(NULL)
{
}


ShapedStringBuffer::~ShapedStringBuffer()
{
	clearShapedString();
}


void ShapedStringBuffer::createShapedString(const wchar_t *pString)
{
	if(!pString) {
		return;
	}

	int len = wcslen(pString);
	if(len <= 0) {
		return;
	}

	clearShapedString();

	m_pShapedString = new wchar_t[len + 1];
	Q_ASSERT(m_pShapedString);

	memset(m_pShapedString,0,sizeof(wchar_t) * (len + 1));
	memcpy(m_pShapedString,pString,sizeof(wchar_t) * len);
}


void ShapedStringBuffer::clearShapedString()
{
	if(m_pShapedString) {
		delete m_pShapedString;
		m_pShapedString = NULL;
	}
}


bool ShapedStringBuffer::isValid()
{
	return (m_pShapedString != NULL);
}


wchar_t *ShapedStringBuffer::shapedString()
{
	return m_pShapedString;
}