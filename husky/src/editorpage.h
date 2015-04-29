#ifndef EDITORPAGE_H
#define EDITORPAGE_H

#include <qwidget.h>
#include <qtabwidget.h>
#include <QSplitter>
#include <QMenu>

#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/MarkInterface>

#include "ctagsfrontend.h"
#include "ctagslistwidget.h"
#include "kscopeconfig.h"
#include "projectbase.h"

// TODO: cursor position type change  from uint to int or to Cursor

/**
 * An editor window based on the system's current editing application.
 * The page is divided into two panes. One holds an embedded editor, and the
 * other holds a list of tags (generated by Ctags) of the file currently being
 * edited.
 * The widget creates an instance of the editor application, and uses its 
 * document and view objects that allow KScope to control it. A page also
 * Each page is inserted in a separate tab in the EditorTabs widget.
 * @author Elad Lahav
 */

class EditorPage : public QWidget
{
   Q_OBJECT

public:
	EditorPage(KTextEditor::Document*, QMenu*, QTabWidget* pParent = 0);
	~EditorPage();

	void open(const QString&);
	void setNewFile();
	void save();
	bool close(bool bForce = false);
	void applyPrefs();
	void setEditorFocus();
	void setTagListFocus();
	void addBookmark(uint);
	void getBookmarks(FileLocationList&);
    void setShowLinenum(bool bShow);
	
	KTextEditor::Document* getDocument();
	KTextEditor::View* getView();
	QString getFilePath();
	QString getFileName();
	bool isWritable();
	bool isModified();
	QString getSelection();
	QString getSuggestedText();
	QString getLineContents(uint);
	void setLayout(bool bShowTagList, const SPLIT_SIZES&);	
	bool getCursorPos(uint&, uint&);
	bool setCursorPos(uint, uint nCol = 1);
	void setTabWidth(uint);

    void aboutCommand();
	
	virtual QString getWordUnderCursor(uint* pPosInWord = NULL);

	/**
	 * Implements the SymbolCompletion interface method for returning an
	 * object that supports KTextEditor::CodeCompletionInterface.
	 * @return	A pointer to the View object of the editor
	 */	
	//virtual QObject* getCCObject() { return m_pView; }
	
	/**
	 * @return	true if a previously unsaved file is currently being edited,
	 *			false otherwise
	 */
	bool isNewFile() { return m_bNewFile; }
	
	/** The identifier of the Window menu item which activates this page. */
	int m_nMenuId;

public slots:
	void slotGotoLine(uint);
	void slotMenuSelect();
	void slotCompleteSymbol();
	
signals:
	/**
	 * Emitted when a file has been fully loaded into the editor.
	 * @param	pPage	The emitting object
	 * @param	sPath	The full path of the loaded file
	 */
	void fileOpened(EditorPage* pPage, const QString& sPath);
	
	/**
	 * Emitted when an editor is opened for editing a new file.
	 * @param	pPage	The emitting object
	 */
	void newFile(EditorPage* pPage);

	/**
	 * Emitted when the 'modified' status of the editor changes.
	 * This happens when the contents of the editor change, or when the file
	 * being edited is saved.
	 * @param	pPage		The emitting object
	 * @param	bModified	true if the new state is 'modified', false if the
	 *						new state is 'unmodified'
	 */
	void modified(EditorPage* pPage, bool bModified);
	
	/**
	 * Emitted when the position of the cursor changes.
	 * @param	nLine	The new line number
	 * @param	nCol	The new column number
	 */
	void cursorPosChanged(uint nLine, uint nCol);
	
	/**
	 * Emitted when a file is saved after it was modified.
	 * Indicates the project's cross-reference database needs to be updated.
	 * @param	sPath	The full path of the saved file
	 * @param	bIsNew	true if this is a new file, false otherwise
	 */
	void fileSaved(const QString& sPath, bool bIsNew);
	
	/**
	 * Emitted when a file is closed.
	 * @param	sPath	The full path of the closed file
	 */
	void fileClosed(const QString& sPath);

private:
	/** The tab widget holding this page. */
	QTabWidget* m_pParentTab;
	
	/** A Ctags process to use on the edited source file. */
	CtagsFrontend m_ctags;
	
	/** An adjustable splitter for separating the tag list from the editor
		part. */
	QSplitter* m_pSplit;
	
	/** A list view for displaying Ctags results. */
	CtagsListWidget* m_pCtagsListWidget;
	
	/** The document part of the editor. */
	KTextEditor::Document* m_pDoc;
	
	/** The view part of the editor. */
	KTextEditor::View* m_pView;
	
	/** Whether a source file is currently loaded. */
	bool m_bOpen;
	
	/** Whether the file being edited is a new one (i.e., never saved 
	 	before.) */
	bool m_bNewFile;
	
	/** The name of the file being edited. */
	QString m_sName;
	
	/** true if the file system allows this file to be modified, false
	    otherwise. */
	bool m_bWritable;
	
	/** This variable is required in addition to m_pDoc->isModified() so
		that the modified() signal is emitted only once. */
	bool m_bModified;
	
	/** The current line position of the cursor. */
	int m_nLine;
	
	/** Provides symbol completion. */
	//SymbolCompletion* m_pCompletion;
	
	/** Determines whether size changes in the child widgets should be
		stored in the global configuration file. 
		Needs to be explicitly set to false before _each_ operation that
		does not wish to change the defaults. */
	bool m_bSaveNewSizes;
	
private slots:
	void slotChildResized();
	void slotFileOpened();
    void slotSetModified(KTextEditor::Document *pDoc);
	void slotUndoChanged();
	void slotCursorPosChange(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
};

#endif
