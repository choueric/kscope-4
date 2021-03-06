#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>

#include <kapplication.h>
#include <kglobalsettings.h>

#include "kscopeconfig.h"
#include "husky.h"

// NOTE:
// This configuration file entry controls whether the welcome dialogue is
// displayed. Normally it only needs to be shown once, but the entry's name
// can be changed across versions to force the display of new information.
#define SHOW_WELCOME_ENTRY "ShowWelcomeDlg"

/**
 * Stores the display name and the configuration file entry for a configurable
 * GUI element.
 * @author	Elad Lahav
 */
struct ElementInfo
{
	/** The display name of the element. */
	const char* szName;
	/** The configuration file entry. */
	const char* szEntry;
};

/**
 * A list of GUI elements for which the colour can be configured.
 */
const ElementInfo eiColors[] = {
	{ "File List (Foreground)", "FileListFore" },
	{ "File List (Background)", "FileListBack" },
	{ "Tag List (Foreground)", "TagListFore" },
	{ "Tag List (Background)", "TagListBack" },
	{ "Query Window (Foreground)", "QueryListFore" },
	{ "Query Window (Background)", "QueryListBack" },
};

/**
 * A list of GUI elements for which the font can be configured.
 */
const ElementInfo eiFonts[] = {
	{ "File List", "FileList" },
	{ "Tag List", "TagList" },
	{ "Query Page", "QueryList" },
};

#define COLOR_NAME(_i)	eiColors[_i].szName
#define COLOR_ENTRY(_i)	eiColors[_i].szEntry
#define FONT_NAME(_i)	eiFonts[_i].szName
#define FONT_ENTRY(_i)	eiFonts[_i].szEntry

KScopeConfig::ConfParams KScopeConfig::s_cpDef = {
	"/usr/bin/cscope", // Cscope path
	"/usr/bin/ctags", // Ctags path
	true, // Show the tag list
	SPLIT_SIZES(), // Tag list width
	{
		QColor("black"), // File list foreground
		QColor("white"), // File list background
		QColor("black"), // Tag list foreground
		QColor("white"), // Tag list background
		QColor("black"), // Query page foreground
		QColor("white")  // Query page background
	},
	{
		QFont(), // Font definitions are overriden in load()
		QFont(),
		QFont()
	},
	NameAsc, // Ctags sort order
	false, // Read-only mode
	true, // Load last project
	true, // Automatic tag highlighting
	false, // Brief query captions
	true, // Warn when file is modified on the disk
	true, // Sort files when a project is loaded
    true, // show line number
	"kate --line %L %F", // External editor example
	Fast, // System profile
	Embedded, // Editor context menu
};

/**
 * Class constructor.
 */
KScopeConfig::KScopeConfig() : m_bFontsChanged(false)
{
    // TODO:
    loadDefault();
}

/**
 * Class destructor.
 */
KScopeConfig::~KScopeConfig()
{
}

/**
 * Reads KScope's parameters from the standard configuration file.
 */
void KScopeConfig::load()
{
	uint i;
	
    KSharedConfig::Ptr pConf = KGlobal::config();

	// Need a working instance to get the system's default font (cannot be
	// initialised statically)
	s_cpDef.fonts[FileList] = KGlobalSettings::generalFont();
	s_cpDef.fonts[TagList] = KGlobalSettings::generalFont();
	s_cpDef.fonts[QueryWindow] = KGlobalSettings::generalFont();
	
	// Read the paths to required executables
    KConfigGroup groupProgram = pConf->group("Programs");
	m_cp.sCscopePath = groupProgram.readEntry("CScope", "/usr/bin/cscope");
	m_cp.sCtagsPath = groupProgram.readEntry("CTags", "/usr/bin/ctags");

	// Read size and position parameters
    KConfigGroup groupGeometry = pConf->group("Geometry");
	m_cp.bShowTagList = groupGeometry.readEntry("ShowTagList", 
		s_cpDef.bShowTagList);
    SPLIT_SIZES defV;
	m_cp.siEditor = groupGeometry.readEntry("Editor", defV);
	if (m_cp.siEditor.empty())
		m_cp.siEditor << 200 << 800;

	// Read the recent projects list
    KConfigGroup groupProjects = pConf->group("Projects");
	m_slProjects = groupProjects.readEntry("Recent", QStringList());

	// Read colour settings
    KConfigGroup groupColors = pConf->group("Colors");
	for (i = 0; i <= LAST_COLOR; i++) {
		m_cp.clrs[i] = groupColors.readEntry(COLOR_ENTRY(i),
			s_cpDef.clrs[i]);
	}

	// Read font settings
    KConfigGroup groupFonts = pConf->group("Fonts");
	for (i = 0; i <= LAST_FONT; i++) {
		m_cp.fonts[i] = groupFonts.readEntry(FONT_ENTRY(i),
			s_cpDef.fonts[i]);
	}
	
	// Other options
    KConfigGroup gOpt = pConf->group("Options");
	m_cp.ctagSortOrder = (CtagSort)gOpt.readEntry("CtagSortOrder", (int)s_cpDef.ctagSortOrder);
	m_cp.bReadOnlyMode = gOpt.readEntry("ReadOnlyMode", s_cpDef.bReadOnlyMode);
	m_cp.bLoadLastProj = gOpt.readEntry("LoadLastProj", s_cpDef.bLoadLastProj);
	m_cp.bAutoTagHl = gOpt.readEntry("AutoTagHl", s_cpDef.bAutoTagHl);
	m_cp.bBriefQueryCaptions = gOpt.readEntry("BriefQueryCaptions", s_cpDef.bBriefQueryCaptions);
	m_cp.bWarnModifiedOnDisk = gOpt.readEntry("WarnModifiedOnDisk", s_cpDef.bWarnModifiedOnDisk);
	m_cp.bAutoSortFiles = gOpt.readEntry("AutoSortFiles", s_cpDef.bAutoSortFiles);
	m_cp.bShowLinenum = gOpt.readEntry("ShowLinenum", s_cpDef.bShowLinenum);
	m_cp.sExtEditor = gOpt.readEntry("ExternalEditor", s_cpDef.sExtEditor);
	m_cp.profile = (SysProfile)gOpt.readEntry("SystemProfile", (int)s_cpDef.profile);
	m_cp.popup = (EditorPopup)gOpt.readEntry("EditorPopup", (int)s_cpDef.popup);
}

/**
 * Sets default values to he configuration parameters (except for those where
 * a default value has no meaning, such as the recent projects list).
 */
void KScopeConfig::loadDefault()
{
	m_cp = s_cpDef;
}

/**
 * Loads the layout of the main window.
 * @param	pMainWindow	Pointer to the main docking window
 */
void KScopeConfig::loadWorkspace(KMainWindow* pMainWindow)
{
    pMainWindow->setAutoSaveSettings("Worksapcelayout");
}
 
/**
 * Writes KScope's parameters from the standard configuration file.
 */
void KScopeConfig::store()
{
	uint i;
	
    KSharedConfig::Ptr pConf = KGlobal::config();

	// Write the paths to required executables
    KConfigGroup groupProgram = pConf->group("Programs");
	groupProgram.writeEntry("CScope", m_cp.sCscopePath);
	groupProgram.writeEntry("CTags", m_cp.sCtagsPath);

	// Write size and position parameters
    KConfigGroup groupGeometry = pConf->group("Geometry");
	groupGeometry.writeEntry("ShowTagList", m_cp.bShowTagList);
	groupGeometry.writeEntry("Editor", m_cp.siEditor);

	// Write the recent projects list
    KConfigGroup groupProjects = pConf->group("Projects");
	groupProjects.writeEntry("Recent", m_slProjects);

	// Write colour settings
    KConfigGroup groupColors = pConf->group("Colors");
	for (i = 0; i <= LAST_COLOR; i++)
		groupColors.writeEntry(COLOR_ENTRY(i), m_cp.clrs[i]);

	// Write font settings
	if (m_bFontsChanged) {
        KConfigGroup groupFonts = pConf->group("Fonts");
		for (i = 0; i <= LAST_FONT; i++)
			groupFonts.writeEntry(FONT_ENTRY(i), m_cp.fonts[i]);
		
		m_bFontsChanged = false;
	}
		
	// Other options
    KConfigGroup groupOptions = pConf->group("Options");
	groupOptions.writeEntry("CtagSortOrder", (uint)m_cp.ctagSortOrder);
	groupOptions.writeEntry("ReadOnlyMode", m_cp.bReadOnlyMode);
	groupOptions.writeEntry("LoadLastProj", m_cp.bLoadLastProj);
	groupOptions.writeEntry("AutoTagHl", m_cp.bAutoTagHl);
	groupOptions.writeEntry("BriefQueryCaptions", m_cp.bBriefQueryCaptions);
	groupOptions.writeEntry("WarnModifiedOnDisk", m_cp.bWarnModifiedOnDisk);
	groupOptions.writeEntry("AutoSortFiles", m_cp.bAutoSortFiles);
	groupOptions.writeEntry("ShowLinenum", m_cp.bShowLinenum);
	groupOptions.writeEntry("ExternalEditor", m_cp.sExtEditor);
	groupOptions.writeEntry("SystemProfile", (uint)m_cp.profile);
	groupOptions.writeEntry("EditorPopup", (uint)m_cp.popup);
	
	// Do not report it's the first time on the next run
    KConfigGroup groupGeneral = pConf->group("General");
	groupGeneral.writeEntry("FirstTime", false);
	groupGeneral.writeEntry(SHOW_WELCOME_ENTRY, false);
}

/**
 * Stores the layout of the main window.
 * @param	pMainWindow	Pointer to the main docking window
 */
void KScopeConfig::storeWorkspace(KMainWindow *)
{
	// pMainWindow->writeDockConfig(kapp->config(), "Workspace");
    // QMainWindow::restoreState(), saveState()
    qDebug("TODO %s()\n", __FUNCTION__);
}

/**
 * Determines if this is the first time KScope was launched by the current
 * user.
 * @return	true if this is the first time, false otherwise
 */
bool KScopeConfig::isFirstTime()
{
    KSharedConfig::Ptr pConf = KGlobal::config();

    KConfigGroup groupGeneral = pConf->group("General");
	return groupGeneral.readEntry("FirstTime", true);
}

/**
 * Determines if the welcome dialogue should be displayed.
 * Note that while the dialogue is displayed on the first invocation of KScope,
 * it may be required on other occasions (e.g., to display important information
 * on a per-version basis) and thus it is separated from isFirstTime().
 * @return	true if the dialogue should be shown, false otherwise
 */
bool KScopeConfig::showWelcomeDlg()
{
    KSharedConfig::Ptr pConf = KGlobal::config();

    KConfigGroup groupGeneral = pConf->group("General");
	return groupGeneral.readEntry(SHOW_WELCOME_ENTRY, true);
}

/**
 * @return	The full path of the Cscope executable
 */
const QString& KScopeConfig::getCscopePath() const
{
	return m_cp.sCscopePath;
}

/**
 * @param	sPath	The full path of the Cscope executable
 */
void KScopeConfig::setCscopePath(const QString& sPath)
{
	m_cp.sCscopePath = sPath;
}

/**
 * @return	The full path of the Ctags executable
 */
const QString& KScopeConfig::getCtagsPath() const
{
	return m_cp.sCtagsPath;
}

/**
 * @param	sPath	The full path of the Ctags executable
 */
void KScopeConfig::setCtagsPath(const QString& sPath)
{
	m_cp.sCtagsPath = sPath;
}

/**
 * @return	A sorted list of recently used project paths.
 */
const QStringList& KScopeConfig::getRecentProjects() const
{
	return m_slProjects;
}

/**
 * Adds the given project path to the beginning of the recently used projects
 * list.
 * @param	sProjPath	The path of the project to add
 */
void KScopeConfig::addRecentProject(const QString& sProjPath)
{
    int index;
	
	index = m_slProjects.indexOf(sProjPath);
	if (index != -1)
		m_slProjects.removeAll(sProjPath);
			
	m_slProjects.prepend(sProjPath);
}

/**
 * Removes the given project path from recently used projects list.
 * @param	sProjPath	The path of the project to remove
 */
void KScopeConfig::removeRecentProject(const QString& sProjPath)
{
    m_slProjects.removeAll(sProjPath);
}

/**
 * @return	true if the tag list should be visible, false otherwise
 */
bool KScopeConfig::getShowTagList() const
{
	return m_cp.bShowTagList;
}

/**
 * @param	bShowTagList	true to make the tag list visible, false otherwise
 */
void KScopeConfig::setShowTagList(bool bShowTagList)
{
	m_cp.bShowTagList = bShowTagList;
}

/**
 * @return	A list containing the widths of the Ctags list part and the
 * editor part in an editor page.
 */
const SPLIT_SIZES& KScopeConfig::getEditorSizes() const
{
	return m_cp.siEditor;
}

/**
 * @param	siEditor	A list containing the widths of the Ctags list part
 * and the editor part in an editor page.
 */
void KScopeConfig::setEditorSizes(const SPLIT_SIZES& siEditor)
{
	m_cp.siEditor = siEditor;
}

/**
 * Finds a colour to use for a GUI element.
 * @param	ce		Identifies the GUI element
 * @return	A reference to the colour object to use
 */
const QColor& KScopeConfig::getColor(ColorElement ce) const
{
	return m_cp.clrs[ce];
}

/**
 * Returns the display name of a GUI element whose colour can be configured.
 * @param	ce	The GUI element
 * @return	A name used in the colour configuration page
 */
QString KScopeConfig::getColorName(ColorElement ce) const
{
	return COLOR_NAME(ce);
}

/**
 * Sets a new colour to a GUI element.
 * @param	ce		Identifies the GUI element
 * @param	clr		The colour to use
 */ 
void KScopeConfig::setColor(ColorElement ce, const QColor& clr)
{
	m_cp.clrs[ce] = clr;
}

/**
 * Finds a font to use for a GUI element.
 * @param	fe		Identifies the GUI element
 * @return	A reference to the font object to use
 */
const QFont& KScopeConfig::getFont(FontElement fe) const
{
	return m_cp.fonts[fe];
}

/**
 * Returns the display name of a GUI element whose font can be configured.
 * @param	ce	The GUI element
 * @return	A name used in the font configuration page
 */
QString KScopeConfig::getFontName(FontElement ce) const
{
	return FONT_NAME(ce);
}

/**
 * Sets a new font to a GUI element.
 * @param	fe		Identifies the GUI element
 * @param	font	The font to use
 */ 
void KScopeConfig::setFont(FontElement fe, const QFont& font)
{
	m_bFontsChanged = true;
	m_cp.fonts[fe] = font;
}

/**
 * @return	The column and direction by which the tags should be sorted
 */
KScopeConfig::CtagSort KScopeConfig::getCtagSortOrder()
{
	return m_cp.ctagSortOrder;
}

/**
 * @param	ctagSortOrder	The column and direction by which the tags should
 *							be sorted
 */
void KScopeConfig::setCtagSortOrder(CtagSort ctagSortOrder)
{
	m_cp.ctagSortOrder = ctagSortOrder;
}

/**
 * @return	true to work in Read-Only mode, false otherwise
 */
bool KScopeConfig::getReadOnlyMode()
{
	return m_cp.bReadOnlyMode;
}

/**
 * @param	bReadOnlyMode	true to work in Read-Only mode, false otherwise
 */
void KScopeConfig::setReadOnlyMode(bool bReadOnlyMode)
{
	m_cp.bReadOnlyMode = bReadOnlyMode;
}

/**
 * @return	true to load the last project on start-up, false otherwise
 */
bool KScopeConfig::getLoadLastProj()
{
	return m_cp.bLoadLastProj;
}

/**
 * @param	bLoadLastProj	true to load the last project on start-up, false
 *							otherwise
 */
void KScopeConfig::setLoadLastProj(bool bLoadLastProj)
{
	m_cp.bLoadLastProj = bLoadLastProj;
}

/**
 * @return	true to enable tag highlighting based on cursor position, false
 *			to disable this feature
 */
bool KScopeConfig::getAutoTagHl()
{
	return m_cp.bAutoTagHl;
}

/**
 * @param	bAutoTagHl	true to enable tag highlighting based on cursor
 *			position, false to disable this feature
 */
void KScopeConfig::setAutoTagHl(bool bAutoTagHl)
{
	m_cp.bAutoTagHl = bAutoTagHl;
}

/**
 * @return	true to use the short version of the query captions, false to use
 *			the long version
 */
bool KScopeConfig::getUseBriefQueryCaptions()
{
	return m_cp.bBriefQueryCaptions;
}

/**
 * @param	bBrief	true to use the short version of the query captions, false
 *			to use the long version
 */
void KScopeConfig::setUseBriefQueryCaptions(bool bBrief)
{
	m_cp.bBriefQueryCaptions = bBrief;
}

/**
 * @return	true to warn user when file is modified on disk, false
 *			otherwise
 */
bool KScopeConfig::getWarnModifiedOnDisk()
{
	return m_cp.bWarnModifiedOnDisk;
}

/**
 * @param	bWarn	true to warn user when file is modified on disk,
 *					false otherwise
 */
void KScopeConfig::setWarnModifiedOnDisk(bool bWarn)
{
	m_cp.bWarnModifiedOnDisk = bWarn;
}

/**
 * @return	true to sort files when a project is loaded, false otherwise
 */
bool KScopeConfig::getAutoSortFiles()
{
	return m_cp.bAutoSortFiles;
}

bool KScopeConfig::getShowLinenum()
{
	return m_cp.bShowLinenum;
}

/**
 * @param	bSort	true to sort files when a project is loaded, false 
 *					otherwise
 */
void KScopeConfig::setAutoSortFiles(bool bSort)
{
	m_cp.bAutoSortFiles = bSort;
}

void KScopeConfig::setShowLinenum(bool bShow)
{
	m_cp.bShowLinenum = bShow;
}

/**
 * @return	A command line for launching an external editor
 */
const QString& KScopeConfig::getExtEditor()
{
	return m_cp.sExtEditor;
}

/**
 * @param	sExtEditor	A command line for launching an external editor
 */
void KScopeConfig::setExtEditor(const QString& sExtEditor)
{
	m_cp.sExtEditor = sExtEditor;
}

/**
 * Determines if an external editor should be used.
 * An external editor is used if KScope is in Read-Only mode, and a command-
 * line for the editor was specified.
 * @return	true to use an external editor, false otherwise
 */
bool KScopeConfig::useExtEditor()
{
	return !m_cp.sExtEditor.isEmpty();
}

/**
 * @return	The chosen profile for this system (@see SysProfile)
 */
KScopeConfig::SysProfile KScopeConfig::getSysProfile() const
{
	return m_cp.profile;
}

/**
 * @param	profile	The system profile to use (@see SysProfile)
 */
void KScopeConfig::setSysProfile(KScopeConfig::SysProfile profile)
{
	m_cp.profile = profile;
}

/**
 * @return	The chosen popup menu type for the embedded editor (@see
 *			EditorPopup)
 */
KScopeConfig::EditorPopup KScopeConfig::getEditorPopup() const
{
	return m_cp.popup;
}

/**
 * @return	The name of the popup menu to use in the embedded editor
 */
QString KScopeConfig::getEditorPopupName() const
{
	switch (m_cp.popup) {
	case Embedded:
		return "ktexteditor_popup";
		
	case KScopeOnly:
		return "kscope_popup";
	}
	
	// Will not happen, but the compiler complains if no return statement is
	// given here
	return "";
}

/**
 * @param	popup	The popup menu to use for the embedded editor (@see
 *					EditorPopup)
 */
void KScopeConfig::setEditorPopup(KScopeConfig::EditorPopup popup)
{
	m_cp.popup = popup;
}

/**
 * Returns a reference to a global configuration object.
 * The static object defined is this function should be the only KSCopeConfig
 * object in this programme. Any code that wishes to get or set global
 * configuration parameters, should call Config(), instead of defining its
 * own object.
 * @return	Reference to a statically allocated configuration object
 */
KScopeConfig& Config()
{
	static KScopeConfig conf;
	return conf;
}
