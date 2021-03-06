#include "husky.h"
#include <qregexp.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "newprojectdlg.h"

/**
 * Class constructor.
 * @param	bNewProj	true to create a new project dialog, false to display
 *						the properties of an existing project
 * @param	pParent		The parent widget
 * @param	szName		The widget's name
 */
NewProjectDlg::NewProjectDlg(bool bNewProj, QWidget* pParent) :
    QDialog(pParent),
	m_bNewProj(bNewProj)
{
    setupUi(this);
	ProjectBase::Options opt;

	// Create the auto-completion sub-dialogue
	m_pAutoCompDlg = new AutoCompletionDlg(this);
	
	// Restrict the path requester to existing directories.
	m_pPathRequester->setMode(KFile::Directory | KFile::ExistingOnly | 
		KFile::LocalOnly);
	m_pSrcRootRequester->setMode(KFile::Directory | KFile::ExistingOnly | 
			KFile::LocalOnly);
	
	// Set up the Create/Cancel buttons	
	connect(m_pCreateButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	// Show the auto-completion properties dialogue
	connect(m_pACButton, SIGNAL(clicked()), m_pAutoCompDlg, SLOT(exec()));	

    connect(m_pAvailTypesList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(slotAvailTypesChanged(QListWidgetItem *)));

	// Perform actions specific to the type of dialog (new project or
	// project properties)
	if (bNewProj) {
		// Set default project properties
		ProjectBase::getDefOptions(opt);
		setProperties("", "", opt);
	}
	else {
		// Give appropriate titles to the dialog and the accept button
		setWindowTitle(i18n("Project Properties"));
		m_pCreateButton->setText(i18n("OK"));
		
		// Disable the non-relevant widgets
		m_pNameEdit->setEnabled(false);
		m_pPathRequester->setEnabled(false);
	}
}

/**
 * Class destructor.
 */
NewProjectDlg::~NewProjectDlg()
{
}

/**
 * Configures the dialog's widget to display the properties of the current
 * project.
 * @param	sName	The project's name
 * @param	sPath	The project's path
 * @param	opt		Project parameters configurable in this dialogue
 */
void NewProjectDlg::setProperties(const QString& sName, const QString& sPath,
	const ProjectBase::Options& opt)
{
	QStringList::ConstIterator itr;
	
	// Set values for current project
	m_pNameEdit->setText(sName);
	m_pPathRequester->setPath(sPath);
	m_pSrcRootRequester->setPath(opt.sSrcRootPath);
	m_pKernelCheck->setChecked(opt.bKernel);
	m_pInvCheck->setChecked(opt.bInvIndex);
	m_pNoCompCheck->setChecked(opt.bNoCompress);
	m_pSlowPathCheck->setChecked(opt.bSlowPathDef);
	
	if (opt.nAutoRebuildTime >= 0) {
		m_pAutoRebuildCheck->setChecked(true);
		m_pAutoRebuildSpin->setValue(opt.nAutoRebuildTime);
	}

	if (opt.bACEnabled) {
		m_pACCheck->setChecked(true);
	}
	
	if (opt.nTabWidth > 0) {
		m_pTabWidthCheck->setChecked(true);
		m_pTabWidthSpin->setValue(opt.nTabWidth);
	}
	
	// Initialise the auto-completion sub-dialogue
	m_pAutoCompDlg->m_nMinChars = opt.nACMinChars;
	m_pAutoCompDlg->m_nDelay = opt.nACDelay;
	m_pAutoCompDlg->m_nMaxEntries = opt.nACMaxEntries;
	
	// Add type strings to the types list box
	for (itr = opt.slFileTypes.begin(); itr != opt.slFileTypes.end(); ++itr)
		m_pTypesList->addItem(*itr);
	
	m_pCtagsCmdEdit->setText(opt.sCtagsCmd);
}

/**
 * Retrieves the text entered by the user in the dialog's "Project Name" edit
 * box.
 * @return	The name of the new project
 */
QString NewProjectDlg::getName()
{
	return m_pNameEdit->text();
}

/**
 * Retrieves the text entered by the user in the dialog's "Project Path" edit
 * box.
 * Note that the chosen path will be the parent of the new project's
 * directory, created under it using the project's name.
 * @return	The full path of the parent directory for the new project
 */
QString NewProjectDlg::getPath()
{
	if (m_pHiddenDirCheck->isChecked())
		return QString(m_pSrcRootRequester->text()) + "/.cscope";
	
	return m_pPathRequester->text();
}

/**
 * Fills a structure with all user-configured project options.
 * @param	opt	The structure to fill
 */
void NewProjectDlg::getOptions(ProjectBase::Options& opt)
{
	opt.sSrcRootPath = m_pSrcRootRequester->text();
	opt.slFileTypes = m_slTypes;
	opt.bKernel = m_pKernelCheck->isChecked();
	opt.bInvIndex = m_pInvCheck->isChecked();
	opt.bNoCompress = m_pNoCompCheck->isChecked();
	opt.bSlowPathDef = m_pSlowPathCheck->isChecked();
		
	if (m_pAutoRebuildCheck->isChecked())
		opt.nAutoRebuildTime = m_pAutoRebuildSpin->value();
	else
		opt.nAutoRebuildTime = -1;
		
	if (m_pTabWidthCheck->isChecked())
		opt.nTabWidth = m_pTabWidthSpin->value();
	else
		opt.nTabWidth = 0;
		
	opt.bACEnabled = m_pACCheck->isChecked();
	opt.nACMinChars = m_pAutoCompDlg->m_nMinChars;
	opt.nACDelay = m_pAutoCompDlg->m_nDelay;
	opt.nACMaxEntries = m_pAutoCompDlg->m_nMaxEntries;
	
	opt.sCtagsCmd = m_pCtagsCmdEdit->toPlainText();
}

/**
 * Ends the dialog after the user has clicked the "OK" button.
 */
void NewProjectDlg::accept()
{
	int i, nCount;
	
	// Validate the name of a new project
	if (m_bNewProj) {
		QRegExp re("[^ \\t\\n]+");
		if (!re.exactMatch(m_pNameEdit->text())) {
			KMessageBox::error(0, i18n("Project names must not contain "
				"whitespace."));
			return;
		}
	}
	
	// Fill the string list with all file types
	nCount = (int)m_pTypesList->count();
	for (i = 0; i < nCount; i++) {
        QListWidgetItem *pItem = m_pTypesList->item(i);
		m_slTypes.append(pItem->text());
    }

	// Clean-up the source root
	QDir dir(m_pSrcRootRequester->text());
	if (dir.exists())
		m_pSrcRootRequester->setPath(dir.absolutePath());
	else
		m_pSrcRootRequester->setPath(qgetenv("HOME"));
		
	// Close the dialog
	QDialog::accept();
}

/**
 * Adds the the file type string in the edit-box to the list of project types.
 * This slot is called when the "Add.." button is clicked.
 */
void NewProjectDlg::slotAddType()
{
	QString sType;
		
	// Try the custom type edit-box first.
	sType = m_pTypesEdit->text();
	sType = sType.trimmed();
	if (sType.isEmpty()) {
		return;
    }

	// Validate the type string
	QRegExp reg("[ \\t\\n\\|\\\\\\/]");
	if (sType.contains(reg)) {
		KMessageBox::error(0, i18n("This is not a valid file type!"));
		return;
	}

	// Do not add an existing type.
    QList<QListWidgetItem *> f = m_pTypesList->findItems(sType, Qt::MatchCaseSensitive | Qt::MatchExactly);
	if (!f.isEmpty()) {
		return;
	}

	// Add the file type to the list
    dp("add item %s", sType.toAscii().data());
	m_pTypesList->addItem(sType);
	m_pTypesEdit->clear();
}

/**
 * Removes the selected item from the list of file types.
 * This slot is called when the "Remove" button is clicked.
 */
void NewProjectDlg::slotRemoveType()
{
    QListWidgetItem *p = NULL;
	QString sType;
	
	// Verify an item is selected
	p = m_pTypesList->currentItem();
	if (p == NULL) {
		return;
    }

	// Remove the selected item
	sType = p->text();
	m_pTypesList->takeItem(m_pTypesList->row(p));
    delete p;
	
	// Add to the list of available types.
    QList<QListWidgetItem *> f = m_pAvailTypesList->findItems(sType, Qt::MatchCaseSensitive | Qt::MatchExactly);
	if (f.isEmpty()) {
		m_pAvailTypesList->addItem(sType);
	}
}

/**
 * Changes the text in the types edit-box to reflect the current selection in
 * the list of available types.
 * This slot is called whenever a new item is highlighted in the list of
 * available types.
 * @param	sType	The newly selected type
 */
void NewProjectDlg::slotAvailTypesChanged(QListWidgetItem *pItem)
{
	m_pTypesEdit->setText(pItem->text());
}

/**
 * Class constructor.
 * @param	pParent		The parent widget
 * @param	szName		The widget's name
 */
AutoCompletionDlg::AutoCompletionDlg(QWidget* pParent):
    QDialog(pParent)
{
    setupUi(this);
	connect(m_pOKButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

/**
 * Class destructor.
 */
AutoCompletionDlg::~AutoCompletionDlg()
{
}

/**
 * Displays the dialogue, and waits for either the "OK" or "Cancel" button to
 * be clicked.
 * Before the dialogue is displayed, the stored values are set to the widgets.
 * @return	The dialogue's termination code
 */
int AutoCompletionDlg::exec()
{
	// Set current values
	m_pMinCharsSpin->setValue(m_nMinChars);
	m_pDelaySpin->setValue(m_nDelay);
	m_pMaxEntriesSpin->setValue(m_nMaxEntries);

	// Show the dialogue
	return QDialog::exec();
}

/**
 * Stores the values set by the user in the dialogue widgets, and terminates
 * the dialogue.
 * This slot is connected to the clicked() signal of the "OK" button.
 */
void AutoCompletionDlg::accept()
{
	// Store widget values
	m_nMinChars = m_pMinCharsSpin->value();
	m_nDelay = m_pDelaySpin->value();
	m_nMaxEntries = m_pMaxEntriesSpin->value();
	
	// Close the dialogue, indicating acceptance
	QDialog::accept();
}
