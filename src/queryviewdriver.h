#ifndef QUERYVIEWDRIVER_H
#define QUERYVIEWDRIVER_H

#include <QObject>
#include <QTreeWidget>
#include "cscopefrontend.h"

class QueryView;

/**
 * Executes a Cscope query and displays the results in a QueryView widget.
 * This class is used in conjunction with QueryView to create a query
 * display object. The driver uses the view widget to display result records
 * of an executed query. It also uses the view as a parent widget for the
 * query progress bar.
 * @author Elad Lahav
 */
class QueryViewDriver : public QObject
{
	Q_OBJECT

public:
    QueryViewDriver(QueryView*, QObject* pParent = 0);
    ~QueryViewDriver();

	void query(uint, const QString&, bool bCase, QTreeWidgetItem* pItem = NULL);
    void setRoot(QString &root);
	
	/**
	 * @return	true if a query is currently running, false otherwise
	 */
	bool isRunning() { return m_bRunning; }
		
private:
	/** Cscope object for running queries. */
	CscopeFrontend* m_pCscope;
	
	/** The view to which this object adds result records. */
	QueryView* m_pView;
	
	/** QueryView item passed to addRecord(). */
	QTreeWidgetItem* m_pItem;
	
	/** Displays query progress information. */
	CscopeProgress m_progress;
	
	/** This flag is set to true when a query is executed, and back to false
		when the the CscopeFrontend object emits the finished() signal. */
	bool m_bRunning;

    QString m_sRoot;
	
private slots:
	void slotDataReady(FrontendToken*);
	void slotFinished(uint);
	void slotProgress(int, int);
	void slotViewClosed();
};

#endif
