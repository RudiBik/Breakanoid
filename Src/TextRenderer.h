#pragma once

#include <OGRE/Ogre.h>
#include <OGRE/OgreSingleton.h>
 
typedef std::list<std::string> stringList;

#define FFLAG_VCENTER	1
#define FFLAG_HCENTER	10

class TextRenderer : public Ogre::Singleton<TextRenderer>
{
private:
 
    Ogre::OverlayManager*    _overlayMgr;
    Ogre::Overlay*           _overlay;
    Ogre::OverlayContainer*  _panel;
	
	stringList mList;
 
public:
 
    TextRenderer();
    ~TextRenderer();
 
    void addTextBox(
        const std::string& ID,
        const std::string& text,
        Ogre::Real x, Ogre::Real y,
        Ogre::Real width, Ogre::Real height,
		DWORD flags = 0,
		const std::string& font = "Arial-16",
		const std::string& fontHeight = "16",
        const Ogre::ColourValue& color = Ogre::ColourValue(1.0, 1.0, 1.0, 1.0));
	void TextRenderer::printf(const std::string& ID,  const char *fmt, /* args*/ ...);

    void removeTextBox(const std::string& ID);
 
    void setText(const std::string& ID, const std::string& Text);
    const std::string& getText(const std::string& ID);
};