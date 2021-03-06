#ifndef SYMBOLDLG_H
#define SYMBOLDLG_H

#include <QTreeWidget>
#include <qregexp.h>
#include "ui_symbollayout.h"
#include "cscopefrontend.h"

/**
 * A dialogue that prompts the user for the text of a query.
 * When a query is requested, the user needs to fill in the required
 * information (usually a symbol name). This dialogue allows the user to
 * enter this information, as well as complete a symbol name, and use
 * previously entered text.
 * @author Elad Lahav
 */
 
class SymbolDlg : public QDialog, public Ui_SymbolLayout
{
	Q_OBJECT

public:
	SymbolDlg(QWidget* pParent = 0);
	~SymbolDlg();

	enum { Reference = 0, Definition, Called, Calling, Text, Pattern,
		FileName, Including };
	
	void setType(uint);
	void setSymbol(const QString&);
	void setHistory(QStringList&);
	QString getSymbol() const;
	uint getType() const;
	bool getCase() const;
	
	static QString promptSymbol(QWidget*, uint&, const QString&, bool&);
	static uint getQueryType(uint);
	static void resetHistory() { s_slHistory.clear(); }
	
private:
	/** A cscope process used for symbol completion. */
	CscopeFrontend* m_pCscope;
	
	/** A regular expression for extracting the symbol name out of the text
		token of a Cscope record. 
		@see note in slotHintDataReady(). */
	QRegExp m_reHint;
	
	/** Displays query progress information. */
	CscopeProgress m_progress;
	
	static QStringList s_slHistory;
	
private slots:
	void slotHintClicked();
	void slotHintDataReady(FrontendToken*);
	void slotHintItemSelected(QTreeWidgetItem*, QTreeWidgetItem*);
	void slotHintOptionChanged(bool);
	void slotHintProgress(int, int);
	void slotHintFinished(uint);
	void slotTypeChanged(int);
};

#endif

