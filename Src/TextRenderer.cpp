#include "EngineStd.h"


#include <stdio.h>
#include <stdarg.h>

#include <OGRE\OgreFont.h>
#include <OGRE\OgreFontManager.h>
 

template<> TextRenderer* Ogre::Singleton<TextRenderer>::ms_Singleton = 0;
 
TextRenderer::TextRenderer()
{
	// create Fonts
	Ogre::FontPtr font = Ogre::FontManager::getSingleton().create("Arial-16", "Fonts");
	font->setType(Ogre::FT_TRUETYPE);
	font->setSource("arialblk.ttf");
	font->setTrueTypeSize(16);
	font->load();

	// create Fonts
	Ogre::FontPtr arial64 = Ogre::FontManager::getSingleton().create("Arial-64", "Fonts");
	arial64->setType(Ogre::FT_TRUETYPE);
	arial64->setSource("arialblk.ttf");
	arial64->setTrueTypeSize(64);
	arial64->load();

    _overlayMgr = Ogre::OverlayManager::getSingletonPtr();
 
    _overlay = _overlayMgr->create("overlay1");
    _panel = static_cast<Ogre::OverlayContainer*>(_overlayMgr->createOverlayElement("Panel", "container1"));
    _panel->setDimensions(1, 1);
    _panel->setPosition(0, 0);
 
    _overlay->add2D(_panel);
 
    _overlay->show();

	mList.clear();
}

TextRenderer::~TextRenderer() {
	_overlayMgr->destroy("overlay1");
}
 
void TextRenderer::addTextBox(const std::string& ID,
                const std::string& text,
                Ogre::Real x, Ogre::Real y,
                Ogre::Real width, Ogre::Real height,
				DWORD flags,
				const std::string &font,
				const std::string &fontHeight,
                const Ogre::ColourValue& color)
{
	for(stringList::iterator itBegin = mList.begin(), itEnd = mList.end(); itBegin != itEnd; itBegin++) {
		if((*itBegin).compare(ID) == 0) {
			return;
		}
	}

	mList.push_back(ID);

    Ogre::OverlayElement* textBox = _overlayMgr->createOverlayElement("TextArea", ID);
    textBox->setDimensions(width, height);
    textBox->setMetricsMode(Ogre::GMM_PIXELS);
    textBox->setPosition(x, y);
    textBox->setWidth(width);
    textBox->setHeight(height);
    textBox->setParameter("font_name", font);
    textBox->setParameter("char_height", fontHeight);
    textBox->setColour(color);

	if(flags && FFLAG_HCENTER) 
		textBox->setVerticalAlignment(Ogre::GVA_CENTER);
	if(flags && FFLAG_VCENTER)
		textBox->setHorizontalAlignment(Ogre::GHA_CENTER);

 
    textBox->setCaption(text);
 
    _panel->addChild(textBox);
}

void TextRenderer::printf(const std::string& ID,  const char *fmt, /* args*/ ...)
{    
    char        text[256];
    va_list        ap;
 
    if (fmt == NULL)
        *text=0;
 
    else {
    va_start(ap, fmt);
        vsprintf(text, fmt, ap);
    va_end(ap);
    }
 
    Ogre::OverlayElement* textBox = _overlayMgr->getOverlayElement(ID);
    textBox->setCaption(text);
}
 
void TextRenderer::removeTextBox(const std::string& ID)
{
	mList.remove(ID);
    _panel->removeChild(ID);
    _overlayMgr->destroyOverlayElement(ID);
}
 
void TextRenderer::setText(const std::string& ID, const std::string& Text)
{
    Ogre::OverlayElement* textBox = _overlayMgr->getOverlayElement(ID);
    textBox->setCaption(Text);
}
 
const std::string& TextRenderer::getText(const std::string& ID)
{
    Ogre::OverlayElement* textBox = _overlayMgr->getOverlayElement(ID);
    return textBox->getCaption();
}