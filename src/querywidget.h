#ifndef QUERYWIDGET_H
#define QUERYWIDGET_H

#include <qlistview.h>
#include <QMenu>
#include <kaction.h>
#include <ktoggleaction.h>
#include "ui_querywidgetlayout.h"
#include "tabwidget.h"
#include "querypage.h"
#include "historypage.h"
#include "projectmanager.h"

/**
 * A tabbed-window holding Cscope query results pages.
 * @author Elad Lahav
 */

class QueryWidget : public QDialog, public Ui_QueryWidgetLayout
{
   Q_OBJECT

public:
	QueryWidget(QWidget* pParent = 0);
	~QueryWidget();

	void addQueryPage();
	void initQuery(uint, const QString&, bool);
	void applyPrefs();
	void loadPages(const QString&, const QStringList&);
	void savePages(const QString&, QStringList&);
	void addHistoryRecord(const QString&, uint, const QString&);
	void selectActiveHistory();
	void setPageMenu(QMenu*, KToggleAction*);
	void getBookmarks(FileLocationList&);
	void setRoot(const QString&);
	
	/**
	 * Enables/disables new history items.
	 * @param	bEnabled	true to enable new history items, false to 
	 *						disable
	 */
	void setHistoryEnabled(bool bEnabled) { m_bHistEnabled = bEnabled; }

public slots:
	void slotNewQueryPage();
	void slotLockCurrent(bool);
	void slotLockCurrent();
	void slotRefreshCurrent();
	void slotNextResult();
	void slotPrevResult();
	void slotCloseCurrent();
	void slotCloseAll();
	void slotHistoryPrev();
	void slotHistoryNext();
	
signals:
	/**
 	 * Emitted when the a lineRequested() signal is received from any of the
	 * currently open query pages. 
	 * @param	sPath	The full path of the requested source file
	 * @param	nLine	The requested line number
	 */
	void lineRequested(const QString& sPath, uint nLine);
	
	/**
	 * Emitted when new query page is requested by user
	 */
	void newQuery();
	
private:
	QString m_sRoot;
	/** A menu with query page commands (new query, lock/unlock, close
		query, etc.). */
	QMenu* m_pPageMenu;
	
	/** A toggle-like action for changing the locked state of a query. */
	KToggleAction* m_pLockAction;
	
	/** The active history page. */
	HistoryPage* m_pHistPage;
	
	/** Determines whether history items should be added to the active
		history page. */
	bool m_bHistEnabled;
	
	/** The number of query pages currently open. */
	int m_nQueryPages;
	
	void setPageCaption(QueryPageBase*);
	
	/**
	 * @return	The active page in the tab widget
	 */
	inline QueryPageBase* currentPage()	{ 
		return (QueryPageBase*)m_pQueryTabs->currentWidget();
	}
	
	/**
	 * @param	pWidget	A query page to set as the current one
	 */
	inline void setCurrentPage(QWidget* pWidget) {
		if (pWidget)
			m_pQueryTabs->setCurrentWidget(pWidget);
	}
		
	/**
	 * Determines if a page is a history page.
	 * @param	pPage	The page to check
	 * @return	true if the given page is a history page
	 */
	inline bool isHistoryPage(QWidget* pPage) {
		return (dynamic_cast<HistoryPage*>(pPage) != NULL);
	}
	
	void setPageLocked(QueryPageBase*, bool);
	void findQueryPage();
	void findHistoryPage();

private slots:
	void slotRequestLine(const QString&, uint);
	void slotCurrentChanged(int index);
	void slotClosePage(QWidget*);
	void slotContextMenu(const QPoint&);
	void slotContextMenu(QWidget*, const QPoint&);
};

#endif
