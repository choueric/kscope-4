#include "husky.h"
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "querywidget.h"
#include "kscopepixmaps.h"
#include "kscopeconfig.h"

/**
 * Class constructor.
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
QueryWidget::QueryWidget(QWidget* pParent) :
    QDialog(pParent),
	m_pPageMenu(NULL),
	m_pLockAction(NULL),
	m_pHistPage(NULL),
	m_bHistEnabled(true),
	m_nQueryPages(0)
{
    setupUi(this);
	// Pages can be closed by clicking their tabs
	m_pQueryTabs->setHoverCloseButton(true);
	m_pQueryTabs->setTabShape(QTabWidget::Triangular);
	m_pQueryTabs->setStyleSheet("QTabBar::tab { height: 20px }");
	
	// Change the lock action state according to the current page
	connect(m_pQueryTabs, SIGNAL(currentChanged(int)), this,
		SLOT(slotCurrentChanged(int)));
	
	// Close a query when its tab button is clicked
	connect(m_pQueryTabs, SIGNAL(closeRequest(QWidget*)), this,
		SLOT(slotClosePage(QWidget*)));

	// Show the menu when requested
	connect(m_pQueryTabs, SIGNAL(contextMenu(const QPoint&)), this,
		SLOT(slotContextMenu(const QPoint&)));
	connect(m_pQueryTabs, SIGNAL(contextMenu(QWidget*, const QPoint&)), this,
		SLOT(slotContextMenu(QWidget*, const QPoint&)));
}

/**
 * Class destructor.
 */
QueryWidget::~QueryWidget()
{
}

/**
 * Runs a query in a query page.
 * A query page is first selected, with a new one created if required. The
 * method then creates a Cscope process and runs the query.
 * @param	nType	The query's numeric type code
 * @param	sText	The query's text, as entered by the user
 * @param	bCase		true for case-sensitive queries, false otherwise
 */
void QueryWidget::initQuery(uint nType, const QString& sText, bool bCase)
{
	QueryPage* pPage;
	
	// Make sure we have a query page
	findQueryPage();
	pPage = (QueryPage*)currentPage();
			
	// Use the current page, or a new page if the current one is locked
	if (pPage->isLocked()) {
		addQueryPage();
		pPage = (QueryPage*)currentPage();
	}

	// Reset the page's results list
	pPage->clear();
	pPage->query(nType, sText, bCase);
	
	// Set the page's tab text according to the new query
	setPageCaption(pPage);
}

/**
 * Applies the user's colour and font preferences to all pages.
 */
void QueryWidget::applyPrefs()
{
	QueryPage* pPage;
	int nPages, i;

	// Iterate query pages
	nPages = m_pQueryTabs->count();
	for (i = 0; i < nPages; i++) {
		pPage = (QueryPage*)m_pQueryTabs->widget(i);
		pPage->applyPrefs();
		setPageCaption(pPage);
	}
}

/**
 * Loads all pages saved when the project was closed.
 * @param	sProjPath		The full path of the project directory
 * @param	slFiles	The list of query file names to load
 */
void QueryWidget::loadPages(const QString& sProjPath,
	const QStringList& slFiles)
{
	QStringList::ConstIterator itr;
	QueryPageBase* pPage;
	QString sName;
	
	// Iterate through query files
	for (itr = slFiles.begin(); itr != slFiles.end(); ++itr) {
		// Set the target page, based on the file type (query or history)
		if ((*itr).startsWith("History")) {
			findHistoryPage();
			pPage = m_pHistPage;
		}
		else {
			findQueryPage();
			pPage = (QueryPage*)currentPage();
		}

		// Load a query file to this page, and lock the page
		if (pPage->load(sProjPath, *itr, m_sRoot)) {
			setPageCaption(pPage);
			setPageLocked(pPage, true);
		}
	}
}

/**
 * Stores all pages marked for saving into files in the project directory.
 * @param	sProjPath		The full path of the project directory
 * @param	slFiles			Holds a list of query file names, upon return
 */
void QueryWidget::savePages(const QString& sProjPath, QStringList& slFiles)
{
	int nPageCount, i;
	QueryPage* pPage;
	QString sFileName;
	
	// Iterate pages
	nPageCount = m_pQueryTabs->count();
	for (i = 0; i < nPageCount; i++) {
		pPage = (QueryPage*)m_pQueryTabs->widget(i);
		if (pPage->shouldSave()) {
			// Store this query page
			if (pPage->save(sProjPath, sFileName) && !sFileName.isEmpty())
				slFiles.append(sFileName);
		}
	}
}

/**
 * Adds a new position record to the active history page.
 * @param	sFile	The file path
 * @param	nLine	The line number
 * @param	sText	The contents of the line pointed to by the file path and
 * 					line number
 */
void QueryWidget::addHistoryRecord(const QString& sFile, uint nLine, 
	const QString& sText)
{
    QString name(sFile);

	// Validate file name and line number
	if (sFile.isEmpty() || nLine == 0)
		return;
	
	// Do nothing if history logging is disabled
	if (!m_bHistEnabled)
		return;

	// Make sure there is an active history page	
	findHistoryPage();

    if (m_sRoot != "/")
        name.replace(m_sRoot, "$");
	// Add the position entry to the active page
	m_pHistPage->addRecord(name, nLine, sText);
}

/**
 * Sets the tab caption and tool-tip for the given page.
 * @param	pPage	The page whose tab needs to be changed
 */
void QueryWidget::setPageCaption(QueryPageBase* pPage)
{
	m_pQueryTabs->setTabText(m_pQueryTabs->indexOf(pPage), 
		pPage->getCaption(Config().getUseBriefQueryCaptions()));
	m_pQueryTabs->setTabToolTip(m_pQueryTabs->indexOf(pPage), pPage->getCaption());
}

/**
 * Creates a new query page, and adds it to the tab widget.
 * The new page is set as the current one.
 */
void QueryWidget::addQueryPage()
{
	QueryPage* pPage;
	QString sTitle;

	// Create the page
	pPage = new QueryPage(this);
    pPage->setRoot(m_sRoot);

	// Add the page, and set it as the current one
    // QIcon icon("project_new");
	m_pQueryTabs->addTab(pPage, "Query");
    m_nQueryPages++;
	m_pQueryTabs->setCurrentWidget(pPage);
	
	// Emit the lineRequested() signal when a query record is selected on 
	// this page
	connect(pPage, SIGNAL(lineRequested(const QString&, uint)), this, 
		SLOT(slotRequestLine(const QString&, uint)));
}

/**
 * Creates a new query page, and emits signal about it.
 */
void QueryWidget::slotNewQueryPage()
{
	addQueryPage();
	emit newQuery();
}

/**
 * Locks or unlocks a query.
 * This slot is connected to the toggled() signal of the lock query button.
 * @param	bOn	true if the new state of the button is "on", false if it is
 *				"off"
 */
void QueryWidget::slotLockCurrent(bool bOn)
{
	QueryPageBase* pPage;
	
	pPage = currentPage();
	
	if (pPage != NULL)
		setPageLocked(currentPage(), bOn);
}

/**
 * Locks or unlocks a query, by toggling the current state.
 */
void QueryWidget::slotLockCurrent()
{
	QueryPageBase* pPage;
	
	pPage = currentPage();
	if (pPage != NULL)
		setPageLocked(pPage, !pPage->isLocked());
}

/**
 * Reruns the query whose results are displayed in the current page.
 */
void QueryWidget::slotRefreshCurrent()
{
	QueryPage* pPage;
	
	// Make sure the current page is a valid, non-empty one
	pPage = dynamic_cast<QueryPage*>(currentPage());
	if (pPage == NULL)
		return;
	
	// Clear the current page contents
	pPage->refresh();
}

/**
 * Selects the next query result record in the current query page.
 */
void QueryWidget::slotNextResult()
{
	QueryPage* pPage;

	// Select the next record in the current page
	pPage = dynamic_cast<QueryPage*>(currentPage());
	if (pPage != NULL)
		pPage->selectNext();
}

/**
 * Selects the next query result record in the current query page.
 */
void QueryWidget::slotPrevResult()
{
	QueryPage* pPage;

	// Select the next record in the current page
	pPage = dynamic_cast<QueryPage*>(currentPage());
	if (pPage != NULL)
		pPage->selectPrev();
}

/**
 * Closes the current query page.
 */
void QueryWidget::slotCloseCurrent()
{
	QWidget* pPage;

	// Close the current page
	pPage = currentPage();
	if (pPage != NULL)
		slotClosePage(pPage);
}

/**
 * Closes all query pages.
 */
void QueryWidget::slotCloseAll()
{
	int nPageCount, i;
	QueryPage* pPage;
	
	// Close all pages
	nPageCount = m_pQueryTabs->count();
	for (i = 0; i < nPageCount; i++) {
		pPage = qobject_cast<QueryPage*>(m_pQueryTabs->widget(i));
		m_pQueryTabs->removePage(pPage);
		delete pPage;
	}
	
	m_pHistPage = NULL;
}

/**
 * Handles the "Go->Back" menu command.
 * Moves to the previous position in the position history.
 */
void QueryWidget::slotHistoryPrev()
{
	if (m_pHistPage != NULL) {
		m_bHistEnabled = false;
		m_pHistPage->selectPrev();
		m_bHistEnabled = true;
	}
}

/**
 * Handles the "Go->Forward" menu command.
 * Moves to the next position in the position history.
 */
void QueryWidget::slotHistoryNext()
{
	if (m_pHistPage != NULL) {
		m_bHistEnabled = false;
		m_pHistPage->selectNext();
		m_bHistEnabled = true;
	}
}

/**
 * Sets the active history page, if any, as the current page.
 */
void QueryWidget::selectActiveHistory()
{
	if (m_pHistPage)
		m_pQueryTabs->setCurrentWidget(m_pHistPage);
}

/**
 * Attaches the page operations menu to this widget.
 * The page menu is a popup menu that handles such operations as opening a
 * new page, closing a page, locking a page, etc.
 * @param	pMenu	Pointer to the popup menu
 * @param	pAction Pointer to the "Lock/Unlock" toggle action
 */
void QueryWidget::setPageMenu(QMenu* pMenu, KToggleAction* pAction)
{
	m_pPageMenu = pMenu;
	m_pLockAction = pAction;
}

/**
 * Emits a signal indicating a certain source file and line number are
 * requested.
 * This slot is connected to the recordSelected() signal emitted by any of
 * the query pages. The signal emitted by this slot is used to display an
 * editor page at the requested line.
 * @param	sFileName	The file's path
 * @param	nLine		The requested line in the file
 */
void QueryWidget::slotRequestLine(const QString& sFileName, uint nLine)
{
    QString sFile(sFileName);
	// Disable history if the request came from the active history page
	if (currentPage() == m_pHistPage)
		m_bHistEnabled = false;
		
	if (sFile.startsWith("$"))
		sFile.replace("$", m_sRoot);
	// Emit the signal
	emit lineRequested(sFile, nLine);
	
	// Re-enable history
	if (currentPage() == m_pHistPage)
		m_bHistEnabled = true;
}

/**
 * Update the lock button when the current query page changes.
 * @param	pWidget	The new current page
 */
void QueryWidget::slotCurrentChanged(int index)
{
    if (index == -1)
        return;
	QueryPage* pPage = qobject_cast<QueryPage *>(m_pQueryTabs->widget(index));
    if (pPage == NULL) {
        return;
    }
	
	if (m_pLockAction)
		m_pLockAction->setChecked(pPage->isLocked());
}

/**
 * Removes the given page from the tab widget.
 * This slot is connected to the closeRequest() signal of the KTabBar object.
 * @param	pPage	The page to close
 */
void QueryWidget::slotClosePage(QWidget* pPage)
{
	// Prompt the user before closing a locked query
	if (((QueryPage*)pPage)->isLocked()) {
		if (KMessageBox::questionYesNo(NULL, i18n("You about about to close"
			" a locked page.\nAre you sure?")) == KMessageBox::No) {
			return;
		}
	}

	// Check if the closed page is the active history page
	if (pPage == m_pHistPage)
		m_pHistPage = NULL;
	// Update the number of open history pages
	else if (dynamic_cast<HistoryPage*>(pPage) == NULL)
		m_nQueryPages--;
	
	// Remove the page and delete the object
	m_pQueryTabs->removePage(pPage);
	delete pPage;
}

/**
 * Displays a context menu for page operations.
 * This slot is connected to the contextMenu() signal, emitted by 
 * m_pQueryTabs.
 * NOTE: We assume that the first item in the menu is "New".
 * @param	pt	The point over which the mouse was clicked in request for the
 *				context menu
 */
void QueryWidget::slotContextMenu(const QPoint& pt)
{
	int i;
	
	// Disable everything but the "new" action (clicked outside any widget)
    QList<QAction *> p = m_pPageMenu->actions();
    for (i = 1; i < p.size(); i++) {
        QAction *pAction = p.at(i);
        pAction->setEnabled(false);
    }
	
	// Show the menu
	m_pPageMenu->popup(pt);
}

/**
 * Displays a context menu for page operations.
 * This slot is connected to the contextMenu() signal, emitted by 
 * m_pQueryTabs.
 * @param	pWidget	The page under the mouse
 * @param	pt		The point over which the mouse was clicked in request for 
 *					the	context menu
 */
void QueryWidget::slotContextMenu(QWidget* pWidget, const QPoint& pt)
{
	int i;
	
	// Operations are on the current page, so we must ensure the clicked
	// tab becomes the current one
	m_pQueryTabs->setCurrentWidget(pWidget);
	
	// Enable all operations
    QList<QAction *> p = m_pPageMenu->actions();
    for (i = 1; i < p.size(); i++) {
        QAction *pAction = p.at(i);
        pAction->setEnabled(true);
    }
	
	// Show the menu
	m_pPageMenu->popup(pt);
}

/**
 * Locks/unlocks the give page.
 * @param	pPage	The page to lock or unlock
 * @param	bLock	true to lock the page, false to unlock it
 */
void QueryWidget::setPageLocked(QueryPageBase* pPage, bool bLock)
{
	if (!pPage->canLock())
		return;
		
	// Set the locking state of the current page
	pPage->setLocked(bLock);
	m_pQueryTabs->setTabIcon(m_pQueryTabs->indexOf(pPage), 
            bLock ? GET_PIXMAP(TabLocked) : GET_PIXMAP(TabUnlocked));
		
	// There can only be one unlocked history page. Check if a non-active
	// query page is being unlocked
	if (isHistoryPage(pPage) && (pPage != m_pHistPage)	&& !bLock) {
		// Lock the active history page (may be NULL)
		if (m_pHistPage != NULL)
			setPageLocked(m_pHistPage, true);
		
		// Set the unlock page as the new active history page
		m_pHistPage = (HistoryPage*)pPage;
	}
}

/**
 * Ensures the current page is a query page that is ready to accept new
 * queries.
 * The function first checks the current page. If it is an unlocked query
 * page, then nothing needs to be done. Otherwise, it checks for the first
 * unlocked query page by iterating over all pages in the tab widget. If this
 * fails as well, a new query page is created.
 */
void QueryWidget::findQueryPage()
{
	QueryPage* pPage;
	int nPages, i;
	
	// First check if the current page is an unlocked query page
	pPage = dynamic_cast<QueryPage*>(currentPage());
	if (pPage != NULL) {
		if (!pPage->isLocked() && !pPage->isRunning())
			return;
	}
	
	// Look for the first unlocked query page
	nPages = m_pQueryTabs->count();
	for (i = 0; i < nPages; i++) {
		pPage = dynamic_cast<QueryPage*>(m_pQueryTabs->widget(i));
		if (pPage != NULL) {
			if (!pPage->isLocked() && !pPage->isRunning()) {
				m_pQueryTabs->setCurrentWidget(pPage);
				return;
			}
		}
	}

	// Couldn't find an unlocked query page, create a new one
	addQueryPage();
}

/**
 * Ensures an active history page exists.
 * The active history page is the only unlocked history page. If one does not
 * exist, it is created.
 */
void QueryWidget::findHistoryPage()
{
	HistoryPage* pPage;
	int nPages, i;
	QString sTitle;
	
	// First check if the active history page is unlocked
	if (m_pHistPage != NULL && !m_pHistPage->isLocked())
		return;
	
	// Look for the first unlocked history page
	nPages = m_pQueryTabs->count();
	for (i = 0; i < nPages; i++) {
		pPage = dynamic_cast<HistoryPage*>(m_pQueryTabs->widget(i));
		if (pPage != NULL && !pPage->isLocked()) {
			m_pHistPage = pPage;
			return;
		}
	}

	// Couldn't find an unlocked query page, create a new one
	m_pHistPage = new HistoryPage(this);

	// Add the page, and set it as the current one
	m_pQueryTabs->addTab(m_pHistPage, GET_PIXMAP(TabUnlocked), "");
	setPageCaption(m_pHistPage);

	// Emit the lineRequested() signal when a query record is selected on 
	// this page
	connect(m_pHistPage, SIGNAL(lineRequested(const QString&, uint)), this, 
		SLOT(slotRequestLine(const QString&, uint)));
}

/**
 * Sets a new common root path
 * @param	sRoot	The full path of the new root
 */
void QueryWidget::setRoot(const QString& sRoot)
{
	// Nothing to do if the given root is the same as the old one
	if (sRoot == m_sRoot)
		return;
	
	m_sRoot = sRoot;
	
	// TODO: Update the query pages
	//m_pFileList->setRoot(sRoot);
}
