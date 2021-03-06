#include <QProgressBar>
#include "progressdlg.h"

/**
 * Class constructor.
 * @param	sCaption	The dialogue's title
 * @param	sText		The text to display
 * @param	pParent		The parent widget
 */
ProgressDlg::ProgressDlg(const QString& sCaption, const QString& sText, QWidget* pParent) :
	KProgressDialog(pParent, sCaption, sText),
	m_nIdleValue(-1)
{
	setAutoClose(false);
	setAllowCancel(false);
	
	// Create the idle-progress timer
	m_pIdleTimer = new QTimer(this);

	// Display a busy indicator by increasing the value of the idle counter
	connect (m_pIdleTimer, SIGNAL(timeout()), this, SLOT(slotShowBusy()));
}

/**
 * Class destructor.
 */
ProgressDlg::~ProgressDlg()
{
}

/**
 * Sets a new value to the progress bar.
 * If the new value is non-zero, the progress bar is advanced. Otherwise, the
 * idle timer is initiated to display a busy indicator.
 * @param	nValue	The new value to set.
 */
void ProgressDlg::setValue(int nValue)
{
	QProgressBar* pProgress;

	pProgress = progressBar();
	
	if (nValue != 0) {
		// Do nothing if the value hasn't changed
		if (nValue == pProgress->value())
			return;

		// Handle first non-zero value
		if (m_nIdleValue >= 0) {
			m_pIdleTimer->stop();
			m_nIdleValue = -1;
			pProgress->setTextVisible(true);
		}

		// Set the new value
		pProgress->setValue(nValue);
	}
	else if (m_nIdleValue == -1) {
		// Handle first 0 value
		pProgress->setValue(0);
		pProgress->setTextVisible(false);
		m_nIdleValue = 0;
		m_pIdleTimer->start(200);
	}
}

void ProgressDlg::setIdle()
{
	m_nIdleValue = -1;
	setValue(0);
}

/**
 * Increaes the value of the dummy counter by 1.
 * This slot is called by the timeout() event of the idle timer.
 */
void ProgressDlg::slotShowBusy()
{
	// Increase the counter
	m_nIdleValue += 5;
	if (m_nIdleValue == 100)
		m_nIdleValue = 0;
		
	// Set the value of the progress-bar
	progressBar()->setValue(m_nIdleValue);
}
