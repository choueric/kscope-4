#include "husky.h"
#include <iostream>
#include <qfileinfo.h>
#include <qdir.h>
#include <klocale.h>
#include "frontend.h"

/**
 * Class constructor.
 * @param	nRecordSize	The number of fields in each record
 * @param	bAutoDelete	(Optional) true to delete the object when the process
 *						terminates, false (default) otherwise
 */
Frontend::Frontend(uint nRecordSize, bool bAutoDelete) : KProcess(),
	m_nRecords(0),
	m_pHeadToken(NULL),
	m_pTailToken(NULL),
	m_pCurToken(NULL),
	m_bAutoDelete(bAutoDelete),
	m_bInToken(false),
	m_nRecordSize(nRecordSize)
{
    setOutputChannelMode(SeparateChannels);
	// Parse data on the standard output
    connect(this, SIGNAL(readyReadStandardOutput()), this,
                SLOT(slotReadStdout2()));

	// Parse data on the standard error
    connect(this, SIGNAL(readyReadStandardError()), this,
            SLOT(slotReadStderr2()));
		
	// Delete the process object when the process exits
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this,
        SLOT(slotFinished(int, QProcess::ExitStatus)));
}

/**
 * Class destructor.
 */
Frontend::~Frontend()
{
	// Delete all pending tokens
	while (m_pHeadToken)
		removeToken();
}

/**
 * Executes the back-end process.
 * @param	sName		The name of the process (for error messages)
 * @param	slArgs		A list containing the command-line arguments
 * @param	sWorkDir	(Optional) working directory
 * @param	bBlock		(Optional) true to block, false otherwise
 * @return	true if the process was executed successfully, false otherwise
 */
bool Frontend::run(const QString& sName, const QStringList& slArgs, 
	const QString& sWorkDir, bool bBlock)
{
	// Cannot start if another controlled process is currently running
	if (state() == QProcess::Running) {
		m_sError = i18n("Cannot restart while another process is still "
			"running");
		return false;
	}

	// Reset variables
	m_nRecords = 0;
	m_bKilled = false;
	
    clearEnvironment(); // TODO: to check if this call is needed
	// Setup the command-line arguments
	clearProgram();
	*this << slArgs;

	// Set the working directory, if requested
	if (!sWorkDir.isEmpty())
		setWorkingDirectory(sWorkDir);

	// Execute the child process
    bool ret = true;
    if (bBlock) {
        if (execute() < 0)
            ret = false;
    } else {
        start();
    }
    if (!ret) {
		m_sError = sName + i18n(": Failed to start process");
		return false;
	}
	
	m_sError = i18n("No error");
	return true;
}

/**
 * Kills the process, and emits the aborted() signal.
 * This function should not be called unless the process needs to be
 * interrupted.
 */
void Frontend::kill()
{
	m_bKilled = true;
	KProcess::kill();
	
	emit aborted();
}

/**
 * Appends a token to the end of the token list.
 * @param	pToken	The token to add
 */
void Frontend::addToken(FrontendToken* pToken)
{
	// Check if this is the firt token
	if (m_pHeadToken == NULL) {
		m_pHeadToken = pToken;
		m_pTailToken = pToken;
	}
	else {
		// Not the first token, append and reset the tail token
		m_pTailToken->m_pNext = pToken;
		m_pTailToken = pToken;
	}
}

/**
 * Removes and deletes the token at the head of the token list.
 */
void Frontend::removeToken()
{
	FrontendToken* pToken;
	
	if (m_pHeadToken == NULL)
		return;

	pToken = m_pHeadToken;
	m_pHeadToken = m_pHeadToken->m_pNext;
	delete pToken;

	if (m_pHeadToken == NULL)
		m_pTailToken = NULL;	
}

/**
 * Removes tokens from the head of the list, according to the size of a
 * record.
 */
void Frontend::removeRecord()
{
	uint i;

	for (i = 0; (i < m_nRecordSize) && (m_pHeadToken != NULL); i++)
		removeToken();
}

/**
 * Extracts tokens of text out of a given buffer.
 * @param	ppBuf		Points to the buffer to parse, and is set to the
 *						beginning of the next token, upon return
 * @param	pBufSize	Points to the size of the buffer, and is set to the
 *						remaining size, upon return
 * @param	sResult		Holds the token's text, upon successful return
 * @param	delim		Holds the delimiter by which the token's end was
 * 						determined
 * @return	true if a token was extracted up to the given delimter(s), false
 *			if the buffer ended before a delimiter could be identified
 */
bool Frontend::tokenize(char** ppBuf, int* pBufSize, QString& sResult,
	ParserDelim& delim)
{
	int nSize;
	char* pBuf;
	bool bDelim, bWhiteSpace, bFoundToken = false;
	
	// Iterate buffer
	for (nSize = *pBufSize, pBuf = *ppBuf; (nSize > 0) && !bFoundToken;
		nSize--, pBuf++) {
		// Test if this is a delimiter character
		switch (*pBuf) {
		case '\n':
			bDelim = ((m_delim & Newline) != 0);
			bWhiteSpace = true;
			delim = Newline;
			break;

		case ' ':
			bDelim = ((m_delim & Space) != 0);
			bWhiteSpace = true;
			delim = Space;
			break;

		case '\t':
			bDelim = ((m_delim & Tab) != 0);
			bWhiteSpace = true;
			delim = Tab;
			break;

		default:
			bDelim = false;
			bWhiteSpace = false;
		}

		if (m_bInToken && bDelim) {
			m_bInToken = false;
			*pBuf = 0;
			bFoundToken = true;
		}
		else if (!m_bInToken && !bWhiteSpace) {
			m_bInToken = true;
			*ppBuf = pBuf;
		}
	}

	// Either a token was found, or the search through the buffer was
	// finished without a delimiter character
	if (bFoundToken) {
		sResult = *ppBuf;
		*ppBuf = pBuf;
		*pBufSize = nSize;
	}
	else if (m_bInToken) {
		sResult = QString::fromLatin1(*ppBuf, *pBufSize);
	}
	else {
		sResult = QString::null;
	}

	return bFoundToken;
}

/**
 * Handles text sent by the back-end process to the standard error stream.
 * By default, this method emits the error() signal with the given text.
 * @param	sText	The text sent to the standard error stream
 */
void Frontend::parseStderr(const QString& sText)
{
	emit error(sText);
}

/**
 * Deletes the process object upon the process' exit.
 */
void Frontend::slotFinished(int, ExitStatus)
{
	// Allow specialised clean-up by inheriting classes
	finalize();
	
	// Signal the process has terminated
	emit finished(m_nRecords);
	
	// Delete the object, if required
	if (m_bAutoDelete)
		delete this;
}

/**
 * Reads data written on the standard output by the controlled process.
 * This is a private slot called attached to the readyReadStdout() signal of
 * the controlled process, which means that it is called whenever data is
 * ready to be read from the process' stream.
 * The method reads whatever data is queued, and sends it to be interpreted
 * by parseStdout().
 */
void Frontend::slotReadStdout(KProcess*, char* pBuffer, int nSize)
{
	char* pLocalBuf;
	QString sToken;
	bool bTokenEnded;
	ParserDelim delim;

	// Do nothing if waiting for process to die
	if (m_bKilled)
		return;
	
	pLocalBuf = pBuffer;
	
	// Iterate over the given buffer
	while (nSize > 0) {
		// Create a new token, if the last iteration has completed one
		if (m_pCurToken == NULL)
			m_pCurToken = new FrontendToken();

		// Extract text until the requested delimiter
		bTokenEnded = tokenize(&pLocalBuf, &nSize, sToken, delim);

		// Add the extracted text to the current token
		m_pCurToken->m_sData += sToken;

		// If the buffer has ended before the requested delimiter, we need
		// to wait for more output from the process
		if (!bTokenEnded)
			return;

		// Call the process-specific parser function
		switch (parseStdout(m_pCurToken->m_sData, delim)) {
		case DiscardToken:
			// Token should not be saved
			delete m_pCurToken;
			break;

		case AcceptToken:
			// Store token in linked list
			addToken(m_pCurToken);
			break;

		case RecordReady:
			// Store token, and notify the target object that an entry can
			// be read
			m_nRecords++;
			addToken(m_pCurToken);
			emit dataReady(m_pHeadToken);

			// Delete all tokens in the entry
			removeRecord();
			break;
			
		case Abort:
			kill();
			nSize = 0;
			break;
		}

		m_pCurToken = NULL;
	}
}

void Frontend::slotReadStdout2()
{
    QByteArray output = this->readAllStandardOutput();
    slotReadStdout(this, output.data(), output.size());
}

/**
 * Reads data written on the standard error by the controlled process.
 * This is a private slot called attached to the readyReadStderr() signal of
 * the controlled process, which means that it is called whenever data is
 * ready to be read from the process' stream.
 * The method reads whatever data is queued, and sends it to be interpreted
 * by parseStderr().
 */
void Frontend::slotReadStderr(KProcess*, char* pBuffer, int nSize)
{
	
	// Do nothing if waiting for process to die
	if (m_bKilled)
		return;

	QString sBuf = QString::fromLatin1(pBuffer, nSize);
	parseStderr(sBuf);
}

void Frontend::slotReadStderr2()
{
    QByteArray output = this->readAllStandardError();
    slotReadStderr(this, output.data(), output.size());
}
