#include <QHBoxLayout>
#include <KTextEditor/ConfigInterface>

#include <qfileinfo.h>
#include <kdeversion.h>
#include <KTextEditor/Command>
#include <KTextEditor/CommandInterface>
#include <KTextEditor/ModificationInterface>
#include <KTextEditor/Editor>
#include <QStringList>
#include <QString>

#include "kscopeconfig.h"
#include "editorpage.h"

/**
 * Class constructor.
 * @param	pDoc	The document object associated with this page
 * @param	pMenu	A Cscope queries popup menu to use with the editor
 * @param	pParent	The parent widget
 * @param	szName	The widget's name
 */
EditorPage::EditorPage(KTextEditor::Document* pDoc, QMenu* pMenu, QTabWidget* pParent) : 
    m_pParentTab(pParent),
	m_pDoc(pDoc),
	m_bOpen(false),
	m_bNewFile(false),
	m_sName(""),
	m_bWritable(true), /* new documents are writable by default */
	m_bModified(false),
	m_nLine(0),
	m_bSaveNewSizes(false)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
	
	// Set read-only mode, if required
	if (Config().getReadOnlyMode())
		m_pDoc->setReadWrite(false);

	// Create the child widgets
	m_pSplit = new QSplitter(Qt::Horizontal, this);
	m_pCtagsListWidget = new CtagsListWidget(m_pSplit);
	m_pView = m_pDoc->createView(m_pSplit);

    m_pSplit->addWidget(m_pCtagsListWidget);
    m_pSplit->addWidget(m_pView);
	//m_pSplit->setResizeMode(m_pCtagsListWidget, QSplitter::KeepSize);

    layout->addWidget(m_pSplit);
	
	// Perform tasks only when the document has been loaded completely
	connect(m_pDoc, SIGNAL(completed()), this, SLOT(slotFileOpened()));
	
	// Be notified when the text in the editor changes
	connect(m_pDoc, SIGNAL(textChanged(KTextEditor::Document *)),
            this, SLOT(slotSetModified(KTextEditor::Document *)));
	connect(m_pDoc, SIGNAL(undoChanged()), this, SLOT(slotUndoChanged()));
	
	// Store the sizes of the child windows when the tag list is resized
	// (since it may imply a move of the splitter divider)
	connect(m_pCtagsListWidget, SIGNAL(resized()), this, SLOT(slotChildResized()));

	// Go to a symbol's line if it is selected in the tag list
	connect(m_pCtagsListWidget, SIGNAL(lineRequested(uint)), this,
		SLOT(slotGotoLine(uint)));

	// Add Ctag records to the tag list
	connect(&m_ctags, SIGNAL(dataReady(FrontendToken*)), m_pCtagsListWidget,
		SLOT(slotDataReady(FrontendToken*)));
		
	// Monitor Ctags' operation
	connect(&m_ctags, SIGNAL(finished(uint)), m_pCtagsListWidget, 
		SLOT(slotCtagsFinished(uint)));
		
	// Set the context menu
    m_pView->setContextMenu(pMenu);

	// Emit a signal whenever the cursor's position changes
    connect(m_pView, SIGNAL(cursorPositionChanged(KTextEditor::View *, 
                    const KTextEditor::Cursor &)), 
                this, SLOT(slotCursorPosChange(KTextEditor::View *, 
                        const KTextEditor::Cursor &)));

    setShowLinenum(Config().getShowLinenum());
}

/**
 * Class destructor.
 */
EditorPage::~EditorPage()
{
}

void EditorPage::setShowLinenum(bool bShow)
{
    KTextEditor::ConfigInterface *iface =
        qobject_cast<KTextEditor::ConfigInterface *>(m_pView);
    if (iface) {
        iface->setConfigValue("line-numbers", bShow);
    }
}

/**
 * Returns a pointer to the editor document object embedded in this page.
 * @returns	the document pointer
 */
KTextEditor::Document* EditorPage::getDocument()
{
	return m_pDoc;
}

/**
 * Returns a pointer to the editor view object embedded in this page.
 * @returns	the view pointer
 */
KTextEditor::View* EditorPage::getView()
{
	return m_pView;
}

/** 
 * Returns the full path of the file being edited.
 * @return	The path of the file associated with the Document object, empty 
 *			string if no file is currently open
 */
QString EditorPage::getFilePath()
{
	return m_pDoc->url().path();
}

/** 
 * Returns the name of the file being edited.
 * @return	The name of the file associated with the Document object, empty 
 *			string if no file is currently open
 */
QString EditorPage::getFileName()
{
	return m_sName;
}

/**
 * Determines whether this file can be modified, according to the file-system
 * permissions, and KScope's global settings.
 * @return	true if this document can be changed, false otherwise
 */
bool EditorPage::isWritable()
{
	// Check global settings first
	if (Config().getReadOnlyMode())
		return false;
	
	// Return FS write permissions
	return m_bWritable;
}

/**
 * Determines if the file edited in this page was modified, and the changes
 * were not yet saved.
 * @return	true if the file was modified, false otherwise
 */
bool EditorPage::isModified()
{
	return m_pDoc->isModified();
}

/**
 * Opens a file for editing.
 * @param	sFileName	The full path name of the file to edit.
 */
void EditorPage::open(const QString& sFileName)
{
	// Open the given file
	m_bOpen = false;
    KUrl file(sFileName);
	m_pDoc->openUrl(file);
}

/**
 * Marks the page as containing a new unnamed file.
 */
void EditorPage::setNewFile()
{
	m_bNewFile = true;
	emit newFile(this);
}

/**
 * Saves the edited file.
 */
void EditorPage::save()
{
	if (m_pDoc->isModified())
		m_pDoc->save();
}

/**
 * Closes an edited file.
 * @param	bForce	true to close the file regardless of any modifications, 
 *					false to prompt the user in case of unsaved chnages 
 * @return	true if the file has been closed, false if the user has aborted
 */
bool EditorPage::close(bool bForce)
{
	QString sPath;
	
	// To override the prompt-on-close behaviour, we need to mark the file
	// as unmodified
	if (bForce)
		m_pDoc->setModified(false);
	
	// Close the file, unless the user aborts the action
	sPath = m_pDoc->url().path();
	if (!m_pDoc->closeUrl())
		return false;
		
	emit fileClosed(sPath);
	return true;
}

/**
 * Applies any changes to the user preferences concerning an editor window.
 */
void EditorPage::applyPrefs()
{
	// Determine whether the editor should work in a read-only mode
	if (m_bWritable)
		m_pDoc->setReadWrite(!Config().getReadOnlyMode());
	
	// Apply preferences to the tag list of this window
	m_pCtagsListWidget->applyPrefs();
}

/**
 * Sets the keyboard focus to the editor part of the page.
 * This method is called whenever the page is activated. It is more reasonable
 * to set the focus to the editor than to the tag list.
 */
void EditorPage::setEditorFocus()
{
	m_pView->setFocus();
	const KTextEditor::Cursor c = m_pView->cursorPosition();
	slotCursorPosChange(m_pView, c);
}

/**
 * Sets the keyboard focus to the tag list.
 * This method is called when the "Go To Tag" menu command is invoked.
 */
void EditorPage::setTagListFocus()
{
	m_pCtagsListWidget->slotSetFocus();
}

/**
 * Sets a bookmark at the given line.
 * @param	nLine	The line to mark
 */
void EditorPage::addBookmark(uint nLine)
{
	KTextEditor::MarkInterface *pMarkIf = 
        qobject_cast<KTextEditor::MarkInterface *>(m_pDoc);
	if (pMarkIf)
		pMarkIf->setMark(nLine, KTextEditor::MarkInterface::markType01);
}

/**
 * Retrieves a list of all bookmarks in this page.
 */
void EditorPage::getBookmarks(FileLocationList& fll)
{
	KTextEditor::MarkInterface* pMarkIf;
	QList<KTextEditor::Mark *> plMarks;
	KTextEditor::Mark* pMark;
	
	// Get the marks interface
	pMarkIf = qobject_cast<KTextEditor::MarkInterface*>(m_pDoc);
	if (!pMarkIf)
		return;

	// Find all bookmarks
    const QHash<int, KTextEditor::Mark *> marks = pMarkIf->marks();
    QHashIterator<int, KTextEditor::Mark *> i(marks);

    while (i.hasNext()) {
        i.next();
        pMark = i.value();
		if (pMark->type == KTextEditor::MarkInterface::markType01)
			fll.append(new FileLocation(getFilePath(), pMark->line, 0));
    }
}

/**
 * Returns the currently selected text in an open file.
 * @return	The selected text, or a null string if no text is currently
 * 			selected
 */
QString EditorPage::getSelection()
{
    return m_pView->selectionText();
}

/**
 * Returns a the complete word defined by the current cursor position.
 * Attempts to extract a valid C symbol from the location of the cursor, by
 * starting at the current line and column, and looking forward and backward
 * for non-symbol characters.
 * @return	A C symbol under the cursor, if any, or QString::null otherwise
 */
// TODO: will be a word in two lines ?
QString EditorPage::getWordUnderCursor(uint* pPosInWord)
{
	QString sLine;
	int nLine, nCol, nFrom, nTo, nLast, nLength;
	QChar ch;

	const KTextEditor::Cursor c = m_pView->cursorPosition();

	// Get the line on which the cursor is positioned
    c.position(nLine, nCol);
    const KTextEditor::Cursor cFrom(nLine, 0);
    const KTextEditor::Cursor cTo = m_pDoc->endOfLine(nLine);
    KTextEditor::Range range(cFrom, cTo);

    sLine = m_pDoc->text(range);
	
	// Find the beginning of the current word
	for (nFrom = nCol; nFrom > 0;) {
		ch = sLine.at(nFrom - 1);
		if (!ch.isLetter() && !ch.isDigit() && ch != '_')
			break;
		
		nFrom--;
	}
	
	// Find the end of the current word
	nLast = sLine.length();
	for (nTo = nCol; nTo < nLast;) {
		ch = sLine.at(nTo);
		if (!ch.isLetter() && !ch.isDigit() && ch != '_')
			break;
		
		nTo++;
	}

	// Mark empty words
	nLength = nTo - nFrom;
	if (nLength == 0)
		return QString::null;
	
	// Return the in-word position, if required
	if (pPosInWord != NULL)
		*pPosInWord = nCol - nFrom;
	
	// Extract the word under the cursor from the entire line
	return sLine.mid(nFrom, nLength);
}

/**
 * Combines getSelection() and getWordUnderCursor() to return a suggested
 * text for queries.
 * The function first looks if any text is selected. If so, the selected text
 * is returned. Otherwise, the word under the cursor location is returned, if
 * one exists.
 * @return	Either the currently selected text, or the word under the cursor,
 *			or QString::null if both options fail
 */
QString EditorPage::getSuggestedText()
{
	QString sText;
	
	sText = getSelection();
	if (sText == QString::null)
		sText = getWordUnderCursor();

	return sText;	
}

/**
 * Returns the contents of the requested line.
 * Truncates the leading and trailing white spaces.
 * @param	nLine	The line number
 * @return	The text of the requested line, if successful, QString::null
 *			otherwise
 */
QString EditorPage::getLineContents(uint nLine)
{
	QString sLine;

	// Cannot accept line 0
	if (nLine == 0)
		return QString::null;

	// Get the line on which the cursor is positioned
    const KTextEditor::Cursor cFrom(nLine, 0);
    const KTextEditor::Cursor cTo = m_pDoc->endOfLine(nLine);
    KTextEditor::Range range(cFrom, cTo);

    sLine = m_pDoc->text(range);
	// TODO: return sLine.stripWhiteSpace();
    return sLine;
}

/**
 * Moves the editing caret to the beginning of a given line.
 * @param	nLine	The line number to move to
 */
void EditorPage::slotGotoLine(uint nLine)
{
	// Ensure there is an open document
	if (!m_bOpen)
		return;
	
	// Set the cursor to the requested line
	if (!setCursorPos(nLine))
		return;

	// Update Ctags view
	m_pCtagsListWidget->gotoLine(nLine);

	// Set the focus to the selected line
	m_pView->setFocus();
}

/**
 * Sets this editor as the current page, when the edited file's name is
 * selected in the "Window" menu.
 */
void EditorPage::slotMenuSelect()
{
	m_pParentTab->setCurrentWidget(this);
}

/**
 * Displays a list of possible completions for the symbol currently under the
 * cursor.
 */
void EditorPage::slotCompleteSymbol()
{
	//m_pCompletion->slotComplete();
}

/**
 * Stores the sizes of the child widgets whenever they are changed.
 * This slot is connected to the resized() signal of the CtagsList child
 * widget.
 */
void EditorPage::slotChildResized()
{
	SPLIT_SIZES si;

	// Only store sizes when allowed to
	if (!m_bSaveNewSizes) {
		m_bSaveNewSizes = true;
		return;
	}
		
	// Get the current widths of the child widgets
	si = m_pSplit->sizes();
	if (si.count() == 2)
		Config().setEditorSizes(si);
}

/**
 * Sets the visibility status and sizes of the child widgets.
 * @param	bShowTagList	true to show the tag list, false otherwise
 * @param	si				The new sizes to use
 */
void EditorPage::setLayout(bool bShowTagList, const SPLIT_SIZES& si)
{
	// Make sure sizes are not stored during this process
	m_bSaveNewSizes = false;
	
	// Adjust the layout
	m_pCtagsListWidget->setShown(bShowTagList);
	if (bShowTagList)
		m_pSplit->setSizes(si);
}

/**
 * Returns the current position of the cursor.
 * @param	nLine	Holds the line on which the cursor is currently located
 * @param	nCol	Holds the column on which the cursor is currently located
 * @return	true if successful, false otherwise (cursor interface was not
 *			obtained)
 */
bool EditorPage::getCursorPos(uint& nLine, uint& nCol)
{
    int line, col;
	// Get the cursor position (adjusted to 1-based counting)
	const KTextEditor::Cursor c = m_pView->cursorPosition();
    c.position(line, col);
    nLine = line;
    nCol = col;
	nLine++;
	nCol++;
	
	return true;
}

/**
 * Moves the cursor to a given position.
 * @param	nLine	The cursor's new line number
 * @param	nCol	The cursor's new column number
 * @return	true if successful, false otherwise (cursor interface was not
 *			obtained)
 */
bool EditorPage::setCursorPos(uint nLine, uint nCol)
{
	// Cannot accept line 0
	if (nLine == 0)
		return false;
	
	// Adjust to 0-based counting
	nLine--;
	nCol--;
		
#if 0 // TODO
	Kate::View* pKateView;
	// NOTE: The following code is a fix to a bug in Kate, which wrongly
	// calculates the column number in setCursorPosition.
	pKateView = dynamic_cast<Kate::View*>(m_pView);
	if (pKateView != NULL) {
		KTextEditor::EditInterface* pEditIf;
		const char* szLine;
		uint nRealCol;
		uint nTabAdjust;
		
		// Get a pointer to the edit interface
		pEditIf = dynamic_cast<KTextEditor::EditInterface*>(m_pDoc);
		if (!pEditIf)
			return false;
		
		nRealCol = 0;
		
		// Check for out of bound line numbers
		if (nLine < pEditIf->numLines()) {
			// Get the contents of the requested line
			szLine = pEditIf->textLine(nLine).latin1();
			
			// Check for empty line
			if (szLine != NULL) {
				// The number of columns which a tab character adds
				nTabAdjust = pKateView->tabWidth() - 1;
				
				// Calculate the real column, based on the tab width
				for (; nRealCol < nCol && szLine[nRealCol] != 0; nRealCol++) {
					if (szLine[nRealCol] == '\t')
						nCol -= nTabAdjust;
				}
			}
		}
		else {
			// Marker set beyond end of file, move to the last line
			nLine = pEditIf->numLines() - 1;
		}
		// Set the cursor position
        KTextEditor::cursor setc(nLine, nRealCol);
		m_pView->setCursorPosition(setc);
	}
	else {
		// Non-Kate editors: set the cursor position normally
        KTextEditor::cursor setc(nLine, nRealCol);
		m_pView->setCursorPosition(setc);
	}
#endif
    KTextEditor::Cursor c(nLine, nCol);
    m_pView->setCursorPosition(c);
	
	return true;
}

// set Tab charactor's width (4 / 8 or else)
void EditorPage::setTabWidth(uint nTabWidth)
{
    KTextEditor::Editor *editor = m_pDoc->editor();
    KTextEditor::CommandInterface *iface = 
        qobject_cast<KTextEditor::CommandInterface *>(editor);

	QString sCmd, sResult;
    const QString cmd("set-tab-width");
    KTextEditor::Command *pCmd = iface->queryCommand(cmd);

    if (pCmd) {
		sCmd.sprintf("set-tab-width %u", nTabWidth);
		if (pCmd->exec(m_pView, sCmd, sResult) == false) {
            qDebug("set tab width failed: %s", sResult.toAscii().data());
        }
    } else {
        qDebug("no this cmd");
    }
}

/*
 * show all the commands supported by KTextEditor
 * It's a debug function
 */
void EditorPage::aboutCommand()
{
    KTextEditor::Editor *editor = m_pDoc->editor();
    KTextEditor::CommandInterface *iface = qobject_cast<KTextEditor::CommandInterface *>(editor);

    if (iface == NULL) {
        qDebug("CommandInterface cast failed");
        return;
    }

    QString msg;
    const QString cmd("set-tab-width");
    KTextEditor::Command *pcmd = iface->queryCommand(cmd);
    QStringList list = pcmd->cmds();

#if 0
    for (int i = 0; i < list.size(); i++)
        qDebug("cmd: %s", list[i].toAscii().data());
#endif

    if (pcmd) {
        pcmd->help(m_pView, cmd, msg);
        qDebug("%s", msg.toAscii().data());
    } else {
        qDebug("no this cmd");
    }
}


/**
 * Called when a document has completed loading.
 * Determines the file's properties and refreshes the tag list of the editor
 * window.
 * This slot is connected to the completed() signal of the document object.
 * The signal is emitted when a new file is opened, or when a modified file is
 * saved.
 */
void EditorPage::slotFileOpened()
{
	QFileInfo fi(m_pDoc->url().path());
	
	// Get file information
	m_sName = fi.fileName();
	m_bWritable = fi.isWritable();
	
	// Set read/write or read-only mode
	m_pDoc->setReadWrite(!Config().getReadOnlyMode() && m_bWritable);
	
	// Refresh the tag list
	m_pCtagsListWidget->clear();
	m_ctags.run(m_pDoc->url().path());

	// Check if this is a modified file that has just been saved
	if (m_bModified)
		emit fileSaved(m_pDoc->url().path(), m_bNewFile);
	
	// Notify that the document has loaded
	m_bOpen = true;
	m_bModified = false;
	emit fileOpened(this, m_pDoc->url().path());

	// Set initial position of the cursor
	m_nLine = 0;
    KTextEditor::Cursor c(0, 0);
	slotCursorPosChange(m_pView, c);
	
	// This is no longer a new file
	m_bNewFile = false;
}

/**
 * Marks a file as modified when the contents of the editor change.
 * This slot is conncted to the textChanged() signal of the Document object.
 * In addition to marking the file, this method also emits the modified()
 * signal.
 */
void EditorPage::slotSetModified(KTextEditor::Document *pDoc)
{
    if (m_pDoc != pDoc) {
        qDebug() << "edieor page documant dose not match"; 
        return;
    }

	// Only perform tasks if the file is not already marked
	if (!m_bModified && pDoc->isModified()) {
		m_bModified = true;
		emit modified(this, m_bModified);
	
		// check whether it was modified on
		// disk as well, and issue a warning if so
        KTextEditor::ModificationInterface *iface =
            qobject_cast<KTextEditor::ModificationInterface *>(pDoc);
		if (iface)
			iface->slotModifiedOnDisk(m_pView);
	}
}

/**
 * Marks a file as not modified if all undo levels have been applied.
 * This slot is conncted to the undoChanged() signal of the Document object.
 * In addition to marking the file, this method also emits the modified()
 * signal.
 */
void EditorPage::slotUndoChanged()
{
	// Check if the file contents have been fully restored
	if (m_bModified && !m_pDoc->isModified()) {
		m_bModified = false;
		emit modified(this, m_bModified);
	}
}

/**
 * Handles changes in the cursor position.
 * Emits a signal with the new line and column numbers.
 */
void EditorPage::slotCursorPosChange(KTextEditor::View *view, 
        const KTextEditor::Cursor &newPosition)
{
	int nLine, nCol;

    if (view == NULL)
        return;

	// Find the new line and column number, and emit the signal
    newPosition.position(nLine, nCol);
    nLine++;
    nCol++;

	emit cursorPosChanged(nLine, nCol);
	
	// Select the relevant symbol in the tag list
	if (Config().getAutoTagHl() && (m_nLine != nLine)) {
		m_pCtagsListWidget->gotoLine(nLine);
		m_nLine = nLine;
	}
    m_pView->setFocus();
}

void EditorPage::focusOnTaglist()
{
    m_pCtagsListWidget->focusOnEdit();
}
