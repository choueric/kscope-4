#include <KGlobal>
#include <kiconloader.h>
#include "kscopepixmaps.h"

static const char* XPM_FUNC[] = {
	"12 12 2 1",
	". c #000000",
	"# c #58a8ff",
	"............",
	".##########.",
	".###....###.",
	".###.######.",
	".###.######.",
	".###.######.",
	".###....###.",
	".###.######.",
	".###.######.",
	".###.######.",
	".##########.",
	"............"
};

static const char* XPM_VAR[] = {
	"12 12 3 1",
	". c #000000",
	"a c #c00000",
	"# c #ff0000",
	"............",
	".##########.",
	".##########.",
	".##.####.##.",
	".##.####.##.",
	".##.a##a.##.",
	".##a.##.a##.",
	".###.aa.###.",
	".###a..a###.",
	".####..####.",
	".##########.",
	"............"
};

static const char* XPM_STRUCT[] = {
	"12 12 2 1",
	". c #000000",
	"# c #ffff00",
	"............",
	".##########.",
	".####...###.",
	".###.###.##.",
	".###.######.",
	".####.#####.",
	".#####.####.",
	".######.###.",
	".##.###.###.",
	".###...####.",
	".##########.",
	"............"
};

static const char* XPM_MACRO[] = {
	"12 12 2 1",
	". c #000000",
	"# c #00c000",
	"............",
	".##########.",
	".##.####.##.",
	".##..##..##.",
	".##.#..#.##.",
	".##.####.##.",
	".##.####.##.",
	".##.####.##.",
	".##.####.##.",
	".##.####.##.",
	".##########.",
	"............"
};

static const char* XPM_MEMBER[] = {
	"12 12 3 1",
	". c #000000",
	"a c #0000c0",
	"# c #c0c0ff",
	"............",
	".##########.",
	".##########.",
	".##########.",
	".##a.##.a##.",
	".##.a..a.##.",
	".##.#aa#.##.",
	".##.####.##.",
	".##.####.##.",
	".##########.",
	".##########.",
	"............"
};

static const char* XPM_ENUM[] = {
	"12 12 2 1",
	". c #000000",
	"# c #ff00ff",
	"............",
	".##########.",
	".##########.",
	".##......##.",
	".##.#######.",
	".##.#######.",
	".##.....###.",
	".##.#######.",
	".##.#######.",
	".##......##.",
	".##########.",
	"............"
};

static const char* XPM_ENUMERATOR[] = {
	"12 12 2 1",
	". c #000000",
	"# c #ffc0c0",
	"............",
	".##########.",
	".##########.",
	".###...####.",
	".##.###.###.",
	".##.###.###.",
	".##.....###.",
	".##.#######.",
	".##.###.###.",
	".###...####.",
	".##########.",
	"............"
};

static const char* XPM_TYPEDEF[] = {
	"12 12 2 1",
	". c #000000",
	"# c #c0ffc0",
	"............",
	".##########.",
	".#.......##.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".##########.",
	"............"
};

static const char* XPM_LABEL[] = {
	"12 12 2 1",
	". c #000000",
	"# c #c0ff00",
	"............",
	".##########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.......##.",
	".##########.",
	"............"
};

static const char* XPM_INCLUDE[] = {
	"12 12 2 1",
	". c #000000",
	"# c #c0c0c0",
	"............",
	".##########.",
	".##.....###.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".####.#####.",
	".##.....###.",
	".##########.",
	"............"
};

static const char* XPM_CLASS[] = {
	"12 12 2 1",
	". c #000000",
	"# c #a0a0a0",
	"............",
	".##########.",
	".##.......#.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".#.########.",
	".##.......#.",
	".##########.",
	"............"
};

static const char* XPM_UNKNOWN[] = {
	"12 12 2 1",
	". c #000000",
	"# c #ffffff",
	"............",
	".##########.",
	".##.....###.",
	".#.#####.##.",
	".########.#.",
	".########.#.",
	".#######.##.",
	".######.###.",
	".####.#####.",
	".##########.",
	".####.#####.",
	"............"
};

/**
 * Class constructor.
 */
KScopePixmaps::KScopePixmaps() :
	m_pPixArray(NULL),
	m_loader()
{
}

/**
 * Class destructor.
 */
KScopePixmaps::~KScopePixmaps()
{
	int i;

    if (m_pPixArray == NULL)
        return;
	
	for (i = 0; i < PIX_ARRAY_SIZE; i++) {
        if (m_pPixArray[i] != NULL)
            delete m_pPixArray[i];
    }
		
	delete [] m_pPixArray;
}

/**
 * Creates the array of embedded pixmaps.
 * This function is separated from the constructor since QPixmap objects 
 * cannot be created at the time the static KScopePixmaps object is 
 * allocated.
 */
void KScopePixmaps::init()
{
	// Create the pixmap array
	m_pPixArray = new QPixmap * [PIX_ARRAY_SIZE];

	// Create all pixmaps
	m_pPixArray[SymFunc] = new QPixmap(XPM_FUNC);
	m_pPixArray[SymVar] = new QPixmap(XPM_VAR);
	m_pPixArray[SymStruct] = new QPixmap(XPM_STRUCT);
	m_pPixArray[SymMacro] = new QPixmap(XPM_MACRO);
	m_pPixArray[SymMember] = new QPixmap(XPM_MEMBER);
	m_pPixArray[SymEnum] = new QPixmap(XPM_ENUM);
	m_pPixArray[SymEnumerator] = new QPixmap(XPM_ENUMERATOR);
	m_pPixArray[SymTypedef] = new QPixmap(XPM_TYPEDEF);
	m_pPixArray[SymLabel] = new QPixmap(XPM_LABEL);
	m_pPixArray[SymInclude] = new QPixmap(XPM_INCLUDE);
	m_pPixArray[SymClass] = new QPixmap(XPM_CLASS);
	m_pPixArray[SymUnknown] = new QPixmap(XPM_UNKNOWN);
}

/**
 * Returns a reference to an embedded pixmap.
 * @param	name	The pixmap's identifier
 * @return	A reference to the requested pixmap
 */
const QPixmap& KScopePixmaps::getPixmap(PixName name) const
{
	return *m_pPixArray[name];
}

/**
 * Loads a pixmap with the KIconLoader mechanism.
 * @param	name	The pixmap's identifier
 * @return	The requested pixmap
 */
QPixmap KScopePixmaps::getPixmap(LoadPixName name)
{
	switch (name) {
	case TabUnlocked:
		return m_loader.loadIcon("object-unlocked", KIconLoader::Small, 0,
			false);

	case TabLocked:
		return m_loader.loadIcon("object-locked", KIconLoader::Small, 0,
			false);

	case TabBookmark:
		return m_loader.loadIcon("bookmark-toolbar", KIconLoader::Small, 0,
			false);

	case TabRW:
		return m_loader.loadIcon("document-edit", KIconLoader::Small, 0,
			false);

	case TabRO:
		return m_loader.loadIcon("document-encrypt", KIconLoader::Small, 0,
			false);

	case TabSave:
		return m_loader.loadIcon("document-save", KIconLoader::Small, 0,
			false);
	
	case TabFileList:
		return m_loader.loadIcon("view-list-details", KIconLoader::Small, 0,
			false);

	case TabFileTree:
		return m_loader.loadIcon("view-sidetree", KIconLoader::Small, 0,
			false);
			
	case TabList:
		return m_loader.loadIcon("tab-duplicate", KIconLoader::Small, 0,
			false);
		
        // TODO
	case ButtonSaveAs:
		return m_loader.loadIcon("document-save-as", KIconLoader::Toolbar, 
			0, false);
		
        // TODO
	case ButtonZoomIn:
		return m_loader.loadIcon("viewmag+", KIconLoader::Toolbar, 
			0, false);
		
        // TODO
	case ButtonZoomOut:
		return m_loader.loadIcon("viewmag-", KIconLoader::Toolbar, 
			0, false);
		
        // TODO
	case ButtonRotate:
		return m_loader.loadIcon("rotate", KIconLoader::Toolbar, 
			0, false);
		
        // TODO
	case ButtonPref:
		return m_loader.loadIcon("configure", KIconLoader::Toolbar, 
			0, false);
	
        // TODO
	case CalledTree:
		return m_loader.loadIcon("called_tree", KIconLoader::Toolbar, 
			0, false);
	
        // TODO
	case CallingTree:
		return m_loader.loadIcon("calling_tree", KIconLoader::Toolbar, 
			0, false);
			
        // TODO
	case CallGraph:
		return m_loader.loadIcon("call_graph", KIconLoader::Toolbar, 
			0, false);
	}

	return QPixmap();
}

/**
 * @return	A reference to a global KScopePixmaps object
 */
KScopePixmaps& Pixmaps()
{
	static KScopePixmaps pix;
	return pix;
}
